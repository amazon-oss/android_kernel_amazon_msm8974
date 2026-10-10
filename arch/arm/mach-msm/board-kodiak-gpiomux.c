/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>

static struct gpiomux_setting func1_pd_2ma_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting func1_pd_8ma_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting func1_pk_8ma_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting func1_pn_2ma_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting func1_pn_6ma_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting func1_pn_8ma_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting func2_pn_8ma_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting func3_pn_8ma_cfg = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_oh_2ma_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting gpio_ol_2ma_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting gpio_pd_2ma_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_pn_2ma_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_pu_2ma_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting lcd_en_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting kodiak_touch_int_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting kodiak_touch_int_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting kodiak_touch_resout_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting kodiak_touch_resout_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct msm_gpiomux_config kodiak_audio_configs_common[] __initdata = {
	{
		.gpio = 63,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_ol_2ma_cfg,
		},
	},
	{
		.gpio = 72,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pd_2ma_cfg,
		},
	},
	{
		.gpio = 70,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func1_pk_8ma_cfg,
		},
	},
	{
		.gpio = 71,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func1_pk_8ma_cfg,
		},
	},
	{
		.gpio = 48,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func3_pn_8ma_cfg,
		},
	},
	{
		.gpio = 47,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func3_pn_8ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_camera_configs_common[] __initdata = {
	{
		.gpio = 15,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_2ma_cfg,
		},
	},
	{
		.gpio = 19,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_2ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pn_2ma_cfg,
		},
	},
	{
		.gpio = 20,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_2ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pn_2ma_cfg,
		},
	},
	{
		.gpio = 16,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_6ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_2ma_cfg,
		},
	},
	{
		.gpio = 24,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pn_2ma_cfg,
		},
	},
	{
		.gpio = 23,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_oh_2ma_cfg,
		},
	},
	{
		.gpio = 25,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_oh_2ma_cfg,
		},
	},
	{
		.gpio = 26,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_oh_2ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_sensor_configs_common[] __initdata = {
	{
		.gpio = 10,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func3_pn_8ma_cfg,
		},
	},
	{
		.gpio = 11,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func3_pn_8ma_cfg,
		},
	},
	{
		.gpio = 64,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_ol_2ma_cfg,
		},
	},
	{
		.gpio = 66,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pd_2ma_cfg,
		},
	},
	{
		.gpio = 67,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pd_2ma_cfg,
		},
	},
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pn_2ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_display_configs[] __initdata = {
	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pn_2ma_cfg,
		},
	},
	{
		.gpio = 58,
		.settings = {
			[GPIOMUX_ACTIVE] = &lcd_en_act_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_ol_2ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_nfc_configs[] __initdata = {
	{
		.gpio = 29,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func3_pn_8ma_cfg,
		},
	},
	{
		.gpio = 30,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func3_pn_8ma_cfg,
		},
	},
	{
		.gpio = 59,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pd_2ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_system_configs_common[] __initdata = {
	{
		.gpio = 94,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pd_2ma_cfg,
		},
	},
	{
		.gpio = 95,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pd_2ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_touch_configs_common[] __initdata = {
	{
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE] = &func3_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func3_pn_8ma_cfg,
		},
	},
	{
		.gpio = 7,
		.settings = {
			[GPIOMUX_ACTIVE] = &func3_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func3_pn_8ma_cfg,
		},
	},
	{
		.gpio = 60,
		.settings = {
			[GPIOMUX_ACTIVE] = &kodiak_touch_resout_act_cfg,
			[GPIOMUX_SUSPENDED] = &kodiak_touch_resout_sus_cfg,
		},
	},
	{
		.gpio = 61,
		.settings = {
			[GPIOMUX_ACTIVE] = &kodiak_touch_int_act_cfg,
			[GPIOMUX_SUSPENDED] = &kodiak_touch_int_sus_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_audio_configs_p1[] __initdata = {
	{
		.gpio = 79,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_8ma_cfg,
		},
	},
	{
		.gpio = 80,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func1_pd_8ma_cfg,
		},
	},
	{
		.gpio = 81,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func1_pd_8ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_bt_wifi_configs_p1[] __initdata = {
	{
		.gpio = 36,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_2ma_cfg,
		},
	},
	{
		.gpio = 37,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_2ma_cfg,
		},
	},
	{
		.gpio = 38,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_2ma_cfg,
		},
	},
	{
		.gpio = 39,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_2ma_cfg,
		},
	},
	{
		.gpio = 40,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_2ma_cfg,
		},
	},
	{
		.gpio = 35,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_8ma_cfg,
		},
	},
	{
		.gpio = 43,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_8ma_cfg,
		},
	},
	{
		.gpio = 44,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_8ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_camera_configs_p1p2common[] __initdata = {
	{
		.gpio = 31,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_ol_2ma_cfg,
		},
	},
	{
		.gpio = 32,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pd_2ma_cfg,
		},
	},
	{
		.gpio = 89,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_ol_2ma_cfg,
		},
	},
	{
		.gpio = 18,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pu_2ma_cfg,
		},
	},
	{
		.gpio = 56,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_ol_2ma_cfg,
		},
	},
	{
		.gpio = 57,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_ol_2ma_cfg,
		},
	},
	{
		.gpio = 91,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_ol_2ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_system_configs_p1[] __initdata = {
	{
		.gpio = 4,
		.settings = {
			[GPIOMUX_ACTIVE] = &func2_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_pd_2ma_cfg,
		},
	},
	{
		.gpio = 5,
		.settings = {
			[GPIOMUX_ACTIVE] = &func2_pn_8ma_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_pd_2ma_cfg,
		},
	},
	{
		.gpio = 83,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func3_pn_8ma_cfg,
		},
	},
	{
		.gpio = 84,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func3_pn_8ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_touch_configs_p1[] __initdata = {
	{
		.gpio = 61,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pn_2ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_nfc_configs_p2[] __initdata = {
	{
		.gpio = 86,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pd_2ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_system_configs_p2[] __initdata = {
	{
		.gpio = 92,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_pd_2ma_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_pu_2ma_cfg,
		},
	},
};

static struct msm_gpiomux_config kodiak_camera_configs_dvt[] __initdata = {
	{
		.gpio = 17,
		.settings = {
			[GPIOMUX_ACTIVE] = &func1_pn_6ma_cfg,
			[GPIOMUX_SUSPENDED] = &func1_pd_2ma_cfg,
		},
	},
	{
		.gpio = 21,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func1_pn_2ma_cfg,
		},
	},
	{
		.gpio = 22,
		.settings = {
			[GPIOMUX_SUSPENDED] = &func1_pn_2ma_cfg,
		},
	},
};

void __init kodiak_init_gpiomux(void)
{
	int rc;

	rc = msm_gpiomux_init_dt();
	if (rc) {
		pr_err("%s failed %d\n", __func__, rc);
		return;
	}

	msm_gpiomux_install(kodiak_audio_configs_common,
			ARRAY_SIZE(kodiak_audio_configs_common));
	msm_gpiomux_install(kodiak_camera_configs_common,
			ARRAY_SIZE(kodiak_camera_configs_common));
	msm_gpiomux_install(kodiak_sensor_configs_common,
			ARRAY_SIZE(kodiak_sensor_configs_common));
	msm_gpiomux_install_nowrite(kodiak_display_configs,
			ARRAY_SIZE(kodiak_display_configs));
	msm_gpiomux_install(kodiak_nfc_configs,
			ARRAY_SIZE(kodiak_nfc_configs));
	msm_gpiomux_install(kodiak_system_configs_common,
			ARRAY_SIZE(kodiak_system_configs_common));
	msm_gpiomux_install(kodiak_touch_configs_common,
			ARRAY_SIZE(kodiak_touch_configs_common));
	msm_gpiomux_install(kodiak_audio_configs_p1,
			ARRAY_SIZE(kodiak_audio_configs_p1));
	msm_gpiomux_install(kodiak_bt_wifi_configs_p1,
			ARRAY_SIZE(kodiak_bt_wifi_configs_p1));
	msm_gpiomux_install(kodiak_camera_configs_p1p2common,
			ARRAY_SIZE(kodiak_camera_configs_p1p2common));
	msm_gpiomux_install(kodiak_system_configs_p1,
			ARRAY_SIZE(kodiak_system_configs_p1));
	msm_gpiomux_install(kodiak_touch_configs_p1,
			ARRAY_SIZE(kodiak_touch_configs_p1));
	msm_gpiomux_install(kodiak_nfc_configs_p2,
			ARRAY_SIZE(kodiak_nfc_configs_p2));
	msm_gpiomux_install(kodiak_system_configs_p2,
			ARRAY_SIZE(kodiak_system_configs_p2));
	msm_gpiomux_install(kodiak_camera_configs_dvt,
			ARRAY_SIZE(kodiak_camera_configs_dvt));

	if (socinfo_get_version() >= 0x20000)
		msm_tlmm_misc_reg_write(TLMM_SPARE_REG, 0x5);
}
