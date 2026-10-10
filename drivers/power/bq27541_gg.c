/*
 * BQ27541 fuel gauge driver for the Amazon Fire Phone
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/power_supply.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#define BQ27541_REG_CNTL		0x00
#define BQ27541_REG_TEMP		0x06
#define BQ27541_REG_VOLT		0x08
#define BQ27541_REG_FLAGS		0x0a
#define BQ27541_REG_RM			0x10
#define BQ27541_REG_FCC			0x12
#define BQ27541_REG_AI			0x14
#define BQ27541_REG_CC			0x2a
#define BQ27541_REG_SOC			0x2c
#define BQ27541_REG_DCAP		0x3c

#define BQ27541_FLAG_OTC		BIT(15)
#define BQ27541_FLAG_OTD		BIT(14)
#define BQ27541_FLAG_FC			BIT(9)
#define BQ27541_FLAG_SOC1		BIT(2)
#define BQ27541_FLAG_SOCF		BIT(1)

#define BQ27541_INTER_CMD_UDELAY	66
#define BQ27541_I2C_RETRIES		3
#define BQ27541_LOW_VOLT_COUNT		3
#define ZERO_DEGC_DECIKELVIN		2732

struct bq27541_chip {
	struct device *dev;
	struct i2c_client *client;
	struct qpnp_vadc_chip *vadc_dev;
	struct power_supply bms_psy;
	struct delayed_work status_work;
	struct mutex lock;

	/* device tree */
	int shutoff_mv;
	int charge_mv;
	int capacity_mah;
	int charge_rate;
	int temp_hi_dk;
	int temp_lo_dk;
	unsigned int poll_sec;

	/* cached status */
	bool present;
	int status;
	int health;
	int temp;
	int voltage_mv;
	int flags;
	int rm;
	int fcc;
	int dcap;
	int ai;
	int cycle_count;
	int soc;
	int low_volt_count;
	const char *manufacturer;
};

static enum power_supply_property bq27541_bms_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_MANUFACTURER,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_CHARGING_ENABLED,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MIN,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_INPUT_CURRENT_MAX,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
};

static char *bq27541_supplicants[] = {
	"battery",
};

static int bq27541_read_word(struct bq27541_chip *chip, u8 reg, int *val)
{
	int ret, i;

	for (i = 0; i < BQ27541_I2C_RETRIES; i++) {
		ret = i2c_smbus_read_word_data(chip->client, reg);
		udelay(BQ27541_INTER_CMD_UDELAY);
		if (ret >= 0) {
			*val = ret;
			return 0;
		}
	}

	dev_dbg(chip->dev, "failed to read reg 0x%02x: %d\n", reg, ret);
	return ret;
}

static int bq27541_read_vadc_mv(struct bq27541_chip *chip,
				enum qpnp_vadc_channels channel, int *mv)
{
	struct qpnp_vadc_result result;
	int ret;

	if (!chip->vadc_dev)
		return -ENODEV;

	ret = qpnp_vadc_read(chip->vadc_dev, channel, &result);
	if (ret)
		return ret;

	*mv = (int)div_s64(result.physical, 1000);
	return 0;
}

static const char *bq27541_battery_id_to_name(int mv)
{
	if (mv >= 1751)
		return NULL;
	if (mv >= 1301)
		return "<BIF/Smart>";
	if (mv > 1100)
		return "Unknown";
	if (mv >= 987)
		return "Samsung/McNair";
	if (mv >= 878)
		return "ATL/Desay";
	if (mv >= 773)
		return "Coslight/Sunwoda";
	if (mv >= 669)
		return "LGC/Desay";
	if (mv < 51)
		return "<adapter>";
	return "Unknown";
}

static void bq27541_update_status(struct bq27541_chip *chip)
{
	int temp, volt, flags, rm, fcc, ai, cc, soc, vbat_mv;
	int status, health;
	bool changed = false;

	if (bq27541_read_word(chip, BQ27541_REG_TEMP, &temp) ||
	    bq27541_read_word(chip, BQ27541_REG_VOLT, &volt) ||
	    bq27541_read_word(chip, BQ27541_REG_FLAGS, &flags) ||
	    bq27541_read_word(chip, BQ27541_REG_RM, &rm) ||
	    bq27541_read_word(chip, BQ27541_REG_FCC, &fcc) ||
	    bq27541_read_word(chip, BQ27541_REG_AI, &ai) ||
	    bq27541_read_word(chip, BQ27541_REG_CC, &cc) ||
	    bq27541_read_word(chip, BQ27541_REG_SOC, &soc)) {
		dev_err(chip->dev, "failed to read gauge status\n");
		return;
	}

	temp -= ZERO_DEGC_DECIKELVIN;
	ai = -(s16)ai;

	if (soc == 100 && !(flags & BQ27541_FLAG_FC))
		soc = 99;

	if (bq27541_read_vadc_mv(chip, VBAT_SNS, &vbat_mv))
		vbat_mv = volt;
	if (vbat_mv < chip->shutoff_mv) {
		if (chip->low_volt_count >= BQ27541_LOW_VOLT_COUNT)
			soc = 0;
		else
			chip->low_volt_count++;
	} else {
		chip->low_volt_count = 0;
	}

	if (ai >= 3)
		status = POWER_SUPPLY_STATUS_DISCHARGING;
	else if (ai < -30)
		status = POWER_SUPPLY_STATUS_CHARGING;
	else if (soc == 100)
		status = POWER_SUPPLY_STATUS_FULL;
	else
		status = POWER_SUPPLY_STATUS_NOT_CHARGING;

	if (flags & (BQ27541_FLAG_OTC | BQ27541_FLAG_OTD))
		health = POWER_SUPPLY_HEALTH_OVERHEAT;
	else if (chip->temp_hi_dk &&
		 temp >= chip->temp_hi_dk - ZERO_DEGC_DECIKELVIN)
		health = POWER_SUPPLY_HEALTH_OVERHEAT;
	else if (chip->temp_lo_dk &&
		 temp <= chip->temp_lo_dk - ZERO_DEGC_DECIKELVIN)
		health = POWER_SUPPLY_HEALTH_COLD;
	else
		health = POWER_SUPPLY_HEALTH_GOOD;

	mutex_lock(&chip->lock);
	if (chip->soc != soc || chip->status != status ||
	    chip->health != health)
		changed = true;
	chip->temp = temp;
	chip->voltage_mv = volt;
	chip->flags = flags;
	chip->rm = rm;
	chip->fcc = fcc;
	chip->ai = ai;
	chip->cycle_count = cc;
	chip->soc = soc;
	chip->status = status;
	chip->health = health;
	mutex_unlock(&chip->lock);

	if (changed)
		power_supply_changed(&chip->bms_psy);
}

static void bq27541_status_work(struct work_struct *work)
{
	struct bq27541_chip *chip = container_of(work, struct bq27541_chip,
						 status_work.work);

	if (chip->present)
		bq27541_update_status(chip);

	schedule_delayed_work(&chip->status_work,
			      round_jiffies_relative(chip->poll_sec * HZ));
}

static int bq27541_capacity_level(struct bq27541_chip *chip)
{
	if (chip->flags & BQ27541_FLAG_FC)
		return POWER_SUPPLY_CAPACITY_LEVEL_FULL;
	if (chip->flags & BQ27541_FLAG_SOCF)
		return POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	if (chip->flags & BQ27541_FLAG_SOC1)
		return POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	return POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
}

static int bq27541_get_property(struct power_supply *psy,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct bq27541_chip *chip = container_of(psy, struct bq27541_chip,
						 bms_psy);
	int ret = 0;

	mutex_lock(&chip->lock);
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = chip->status;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = chip->soc;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = chip->ai * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = chip->voltage_mv * 1000;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		val->intval = chip->fcc * 1000;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = (chip->dcap ? chip->dcap : chip->capacity_mah)
				* 1000;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = chip->temp;
		break;
	case POWER_SUPPLY_PROP_MANUFACTURER:
		val->strval = chip->manufacturer;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = chip->health;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_CHARGE_NOW:
		val->intval = chip->rm * 1000;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = chip->present;
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		val->intval = chip->cycle_count;
		break;
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		val->intval = chip->health == POWER_SUPPLY_HEALTH_GOOD;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = chip->charge_mv * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN:
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = chip->shutoff_mv * 1000;
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		val->intval = chip->capacity_mah * 1800;
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_MAX:
		val->intval = chip->charge_rate * chip->capacity_mah * 100;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		val->intval = bq27541_capacity_level(chip);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&chip->lock);

	return ret;
}

static void bq27541_external_power_changed(struct power_supply *psy)
{
	struct bq27541_chip *chip = container_of(psy, struct bq27541_chip,
						 bms_psy);

	cancel_delayed_work(&chip->status_work);
	schedule_delayed_work(&chip->status_work, 0);
}

static void bq27541_parse_dt(struct bq27541_chip *chip)
{
	struct device_node *np = chip->dev->of_node;

	chip->shutoff_mv = 3400;
	chip->charge_mv = 4350;
	chip->capacity_mah = 2360;
	chip->charge_rate = 7;
	chip->poll_sec = 3;

	of_property_read_u32(np, "ursa,info-shutoff-mv", &chip->shutoff_mv);
	of_property_read_u32(np, "ursa,mib-charge-mv", &chip->charge_mv);
	of_property_read_u32(np, "ursa,mib-capacity-mah", &chip->capacity_mah);
	of_property_read_u32(np, "ursa,mib-charge-rate", &chip->charge_rate);
	of_property_read_u32(np, "ursa,limit-temp-hi-dk", &chip->temp_hi_dk);
	of_property_read_u32(np, "ursa,limit-temp-lo-dk", &chip->temp_lo_dk);
	of_property_read_u32(np, "ursa,gg-status-wake-sec", &chip->poll_sec);

	if (!chip->poll_sec)
		chip->poll_sec = 3;
}

static int __devinit bq27541_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct bq27541_chip *chip;
	int ret, val, id_mv;

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_READ_WORD_DATA)) {
		dev_err(&client->dev, "i2c functionality check failed\n");
		return -EIO;
	}

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->dev = &client->dev;
	chip->client = client;
	mutex_init(&chip->lock);
	INIT_DELAYED_WORK(&chip->status_work, bq27541_status_work);
	i2c_set_clientdata(client, chip);

	chip->vadc_dev = qpnp_get_vadc(chip->dev, "gg");
	if (IS_ERR(chip->vadc_dev)) {
		ret = PTR_ERR(chip->vadc_dev);
		if (ret == -EPROBE_DEFER)
			return ret;
		dev_warn(chip->dev, "vadc not available: %d\n", ret);
		chip->vadc_dev = NULL;
	}

	bq27541_parse_dt(chip);

	chip->status = POWER_SUPPLY_STATUS_UNKNOWN;
	chip->health = POWER_SUPPLY_HEALTH_UNKNOWN;
	chip->soc = 50;
	chip->manufacturer = "Unknown";

	chip->present = !bq27541_read_word(chip, BQ27541_REG_CNTL, &val);
	if (!chip->present)
		dev_err(chip->dev, "device ping failed, battery not present?\n");
	else if (!bq27541_read_word(chip, BQ27541_REG_DCAP, &val))
		chip->dcap = val;

	if (!bq27541_read_vadc_mv(chip, LR_MUX2_BAT_ID, &id_mv)) {
		const char *name = bq27541_battery_id_to_name(id_mv);

		if (name)
			chip->manufacturer = name;
		dev_info(chip->dev, "battery id %d mV (%s)\n", id_mv,
			 chip->manufacturer);
	}

	chip->bms_psy.name = "bms";
	chip->bms_psy.type = POWER_SUPPLY_TYPE_BMS;
	chip->bms_psy.properties = bq27541_bms_props;
	chip->bms_psy.num_properties = ARRAY_SIZE(bq27541_bms_props);
	chip->bms_psy.get_property = bq27541_get_property;
	chip->bms_psy.external_power_changed = bq27541_external_power_changed;
	chip->bms_psy.supplied_to = bq27541_supplicants;
	chip->bms_psy.num_supplicants = ARRAY_SIZE(bq27541_supplicants);

	ret = power_supply_register(chip->dev, &chip->bms_psy);
	if (ret) {
		dev_err(chip->dev, "failed to register bms psy: %d\n", ret);
		return ret;
	}

	schedule_delayed_work(&chip->status_work, 0);

	return 0;
}

static int __devexit bq27541_remove(struct i2c_client *client)
{
	struct bq27541_chip *chip = i2c_get_clientdata(client);

	cancel_delayed_work_sync(&chip->status_work);
	power_supply_unregister(&chip->bms_psy);

	return 0;
}

static int bq27541_suspend(struct device *dev)
{
	struct bq27541_chip *chip = dev_get_drvdata(dev);

	cancel_delayed_work_sync(&chip->status_work);

	return 0;
}

static int bq27541_resume(struct device *dev)
{
	struct bq27541_chip *chip = dev_get_drvdata(dev);

	schedule_delayed_work(&chip->status_work, 0);

	return 0;
}

static const struct dev_pm_ops bq27541_pm_ops = {
	.suspend = bq27541_suspend,
	.resume = bq27541_resume,
};

static const struct of_device_id bq27541_of_match[] = {
	{ .compatible = "ursa,bq27541-gg", },
	{ },
};

static const struct i2c_device_id bq27541_id[] = {
	{ "bq27541-gg", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, bq27541_id);

static struct i2c_driver bq27541_driver = {
	.driver = {
		.name = "bq27541-gg",
		.owner = THIS_MODULE,
		.of_match_table = bq27541_of_match,
		.pm = &bq27541_pm_ops,
	},
	.probe = bq27541_probe,
	.remove = __devexit_p(bq27541_remove),
	.id_table = bq27541_id,
};

module_i2c_driver(bq27541_driver);

MODULE_DESCRIPTION("BQ27541 fuel gauge for the Amazon Fire Phone");
MODULE_LICENSE("GPL v2");
