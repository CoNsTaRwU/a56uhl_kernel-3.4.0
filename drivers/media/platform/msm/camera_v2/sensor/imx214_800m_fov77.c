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
#include "msm_sensor.h"
#define imx214_800m_fov77_SENSOR_NAME "imx214_800m_fov77"
#define DUAL_CAL_OTP_SIZE 1024
static uint8_t otp[20];
static uint8_t otp_mem[DUAL_CAL_OTP_SIZE];
static uint8_t *path= "/data/misc/camera/OTPData.dat";
DEFINE_MSM_MUTEX(imx214_800m_fov77_mut);

static struct msm_sensor_ctrl_t imx214_800m_fov77_s_ctrl;

static struct msm_sensor_power_setting imx214_800m_fov77_power_setting[] = {
#ifdef CONFIG_REGULATOR_NCP6924
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,  
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
#endif
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VAF,
		.config_val = 1,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 1,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_CHECK_CAMID,
		.seq_val = 0,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 1,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 1,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 3,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

static struct v4l2_subdev_info imx214_800m_fov77_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id imx214_800m_fov77_i2c_id[] = {
	{imx214_800m_fov77_SENSOR_NAME, (kernel_ulong_t)&imx214_800m_fov77_s_ctrl},
	{ }
};

static int32_t msm_imx214_800m_fov77_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &imx214_800m_fov77_s_ctrl);
}

static struct i2c_driver imx214_800m_fov77_i2c_driver = {
	.id_table = imx214_800m_fov77_i2c_id,
	.probe  = msm_imx214_800m_fov77_i2c_probe,
	.driver = {
		.name = imx214_800m_fov77_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client imx214_800m_fov77_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id imx214_800m_fov77_dt_match[] = {
	{.compatible = "htc,imx214_800m_fov77", .data = &imx214_800m_fov77_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, imx214_800m_fov77_dt_match);

static struct platform_driver imx214_800m_fov77_platform_driver = {
	.driver = {
		.name = "htc,imx214_800m_fov77",
		.owner = THIS_MODULE,
		.of_match_table = imx214_800m_fov77_dt_match,
	},
};

static const char *imx214_800m_fov77Vendor = "Sony";
static const char *imx214_800m_fov77NAME = "imx214_800m_fov77";
static const char *imx214_800m_fov77Size = "13M";

static ssize_t sensor_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%s %s %s\n", imx214_800m_fov77Vendor, imx214_800m_fov77NAME, imx214_800m_fov77Size);
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(sensor, 0444, sensor_vendor_show, NULL);

static ssize_t otp_info_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%02X%02X%02X%02X%02X\n", otp[0],otp[1],otp[2],otp[3],otp[4]);
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(otp_info, 0444, otp_info_show, NULL);

static struct kobject *android_imx214_800m_fov77;

static int imx214_800m_fov77_sysfs_init(void)
{
	int ret ;
	pr_info("imx214_800m_fov77:kobject creat and add\n");
	android_imx214_800m_fov77 = kobject_create_and_add("android_camera", NULL);
	if (android_imx214_800m_fov77 == NULL) {
		pr_info("imx214_800m_fov77_sysfs_init: subsystem_register " \
		"failed\n");
		ret = -ENOMEM;
		return ret ;
	}
	pr_info("imx214_800m_fov77:sysfs_create_file\n");
	ret = sysfs_create_file(android_imx214_800m_fov77, &dev_attr_sensor.attr);
	if (ret) {
		pr_info("imx214_800m_fov77_sysfs_init: sysfs_create_file " \
		"failed\n");
		kobject_del(android_imx214_800m_fov77);
	}

	ret = sysfs_create_file(android_imx214_800m_fov77, &dev_attr_otp_info.attr);
	if (ret) {
		pr_info("imx214_800m_fov77_sysfs_init: sysfs_create_file " \
		"failed\n");
	}

	return 0 ;
}

int32_t imx214_800m_fov77_read_otp_memory(uint8_t *otpPtr, struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	uint16_t read_data = 0;
	int page = 0, i = 0, j = 0;
	short OTP_addr = 0x0A04;

	pr_info("%s: read OTP for dual cam calibration\n", __func__);
	
	for (page = 0; page < 16; page++)
	{
        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A02, page, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
            pr_info("%s: i2c_write w 0x0A02 fail\n", __func__);

        rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A00, 0x01, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
            pr_info("%s: i2c_write w 0x0A00 fail\n", __func__);

		for (i = 0; i < 64; i++) {
			rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
					s_ctrl->sensor_i2c_client,
					OTP_addr,
					&read_data,
					MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0){
				pr_err("%s: i2c_read 0x%x failed\n", __func__, OTP_addr);
				return rc;
			}
			otpPtr[j] = read_data;
			OTP_addr += 0x1;
			j++;
		}
		OTP_addr = 0x0A04;
	}
	pr_info("%s: read OTP memory done\n", __func__);
	return rc;
}

static void imx214_800m_fov77_parse_otp_mem(struct msm_sensor_ctrl_t *s_ctrl){
  int32_t i = 0, j = 0;

  for(j = 2; j >= 0; j--) { 
      if (otp_mem[0x10+64*j] == 0)  
          continue;

      for(i = 0; i < 5; i++)
          otp[i] = otp_mem[0x10+64*j+i];

      for(i = 0; i < 3; i++)
          otp[5+i] = otp_mem[64*j+i];

      for(i = 0; i < 10; i++)
          otp[8+i] = otp_mem[0x15+64*j+i];

      pr_info("[OTP]%s: valid_layer:%d \n", __func__,j);
          break;
  }

  for(i=0; i<5; i++)
      s_ctrl->sensordata->sensor_info->OTP_INFO[i] = otp[i];

  s_ctrl->sensordata->sensor_info->fuse_id[0] = 0;
  s_ctrl->sensordata->sensor_info->fuse_id[1] = otp[5];
  s_ctrl->sensordata->sensor_info->fuse_id[2] = otp[6];
  s_ctrl->sensordata->sensor_info->fuse_id[3] = otp[7];

	pr_info("%s: OTP Module vendor = 0x%x\n",                __func__,  otp[0]);
	pr_info("%s: OTP LENS = 0x%x\n",                         __func__,  otp[1]);
	pr_info("%s: OTP Sensor Version = 0x%x\n",               __func__,  otp[2]);
	pr_info("%s: OTP Driver IC Vendor & Version = 0x%x\n",  __func__,  otp[3]);
	pr_info("%s: OTP Actuator vender ID & Version = 0x%x\n",__func__,  otp[4]);

	pr_info("%s: OTP fuse 1 = 0x%x\n", __func__,  otp[5]);
	pr_info("%s: OTP fuse 2 = 0x%x\n", __func__,  otp[6]);
	pr_info("%s: OTP fuse 3 = 0x%x\n", __func__,  otp[7]);

	pr_info("%s: OTP BAIS Calibration data = 0x%x\n",			__func__,  otp[8]);
	pr_info("%s: OTP OFFSET Calibration data = 0x%x\n", 		__func__,  otp[9]);
	pr_info("%s: OTP VCM bottom mech. Limit (MSByte) = 0x%x\n", __func__,  otp[10]);
	pr_info("%s: OTP VCM bottom mech. Limit (LSByte) = 0x%x\n", __func__,  otp[11]);
	pr_info("%s: OTP Infinity position code (MSByte) = 0x%x\n", __func__,  otp[12]);
	pr_info("%s: OTP Infinity position code (LSByte) = 0x%x\n", __func__,  otp[13]);
	pr_info("%s: OTP Macro position code (MSByte) = 0x%x\n",	__func__,  otp[14]);
	pr_info("%s: OTP Macro position code (LSByte) = 0x%x\n",	__func__,  otp[15]);
	pr_info("%s: OTP VCM top mech. Limit (MSByte) = 0x%x\n",	__func__,  otp[16]);
	pr_info("%s: OTP VCM top mech. Limit (LSByte) = 0x%x\n",	__func__,  otp[17]);


  return;
}

static int imx214_800m_fov77_read_fuseid(struct sensorb_cfg_data *cdata,
	struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;
    static int read_otp = true;
    struct file* f;

	if (read_otp)
	{
		f = msm_fopen (path, O_CREAT|O_RDWR|O_TRUNC, 0666);
		if (f) {
			msm_fwrite (f, 0, otp_mem, DUAL_CAL_OTP_SIZE);
			msm_fclose (f);
			pr_info ("%s: dump OTP memory successfully\n", __func__);
		} else {
			pr_err ("%s: fail to open file to write OTP memory\n", __func__);
		}
		read_otp = false;
	}


    if(cdata == NULL){
		return rc;
    }

    cdata->cfg.fuse.fuse_id_word1 = 0;
    cdata->cfg.fuse.fuse_id_word2 = otp[5];
    cdata->cfg.fuse.fuse_id_word3 = otp[6];
    cdata->cfg.fuse.fuse_id_word4 = otp[7];

    cdata->af_value.VCM_VENDOR = otp[0];
    cdata->af_value.VCM_VENDOR_ID_VERSION = otp[4];
    cdata->af_value.VCM_BIAS = otp[8];
    cdata->af_value.VCM_OFFSET = otp[9];
    cdata->af_value.VCM_BOTTOM_MECH_MSB = otp[10];
    cdata->af_value.VCM_BOTTOM_MECH_LSB = otp[11];
    cdata->af_value.AF_INF_MSB = otp[12];
    cdata->af_value.AF_INF_LSB = otp[13];
    cdata->af_value.AF_MACRO_MSB = otp[14];
    cdata->af_value.AF_MACRO_LSB = otp[15];
    cdata->af_value.VCM_TOP_MECH_MSB = otp[16];
    cdata->af_value.VCM_TOP_MECH_LSB = otp[17];

    strlcpy(cdata->af_value.ACT_NAME, "lc898212_act", sizeof("lc898212_act"));
	pr_info("%s: OTP Actuator Name = %s\n",__func__, "lc898212_act");
	pr_info("OTP info %02X %02X %02X %02X %02X fuseid 00 %02X %02X %02X",otp[0],otp[1],otp[2],otp[3],otp[4],otp[5],otp[6],otp[7]);

    return rc;

}

static int32_t imx214_800m_fov77_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
	match = of_match_device(imx214_800m_fov77_dt_match, &pdev->dev);
	if(match != NULL)
		rc = msm_sensor_platform_probe(pdev, match->data);

	return rc;
}

static int __init imx214_800m_fov77_init_module(void)
{
	int32_t rc = 0;

	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&imx214_800m_fov77_platform_driver,
		imx214_800m_fov77_platform_probe);
	if (!rc) {
		imx214_800m_fov77_sysfs_init();
		return rc;
	}
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return  i2c_add_driver(&imx214_800m_fov77_i2c_driver);
}

static void __exit imx214_800m_fov77_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (imx214_800m_fov77_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&imx214_800m_fov77_s_ctrl);
		platform_driver_unregister(&imx214_800m_fov77_platform_driver);
	} else
		i2c_del_driver(&imx214_800m_fov77_i2c_driver);
	return;
}

#if defined(CONFIG_MACH_DUMMY) || defined(CONFIG_MACH_DUMMY) || defined(CONFIG_MACH_DUMMY)
static int imx214_800m_fov77_match_otp_info(void)
{
	if (otp[3] == 0x11)
		return 0;

	pr_info("%s:otp[3] = %02X, expected 0x11\n", __func__, otp[3]);
	return -1;
}
#endif

int32_t imx214_800m_fov77_sensor_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	int32_t rc1 = 0;
	static int first = 0;
	rc = msm_sensor_match_id(s_ctrl);
	if(rc == 0)
	{
	    if(first == 0)
	    {
	        rc1 = imx214_800m_fov77_read_otp_memory(otp_mem, s_ctrl);
	        imx214_800m_fov77_parse_otp_mem(s_ctrl);
			if (rc1<0) {
				pr_err("%s: imx214_800m_fov77_read_otp_memory failed %d\n", __func__, rc1);
			} else
				first = 1;
	    }
#if defined(CONFIG_MACH_DUMMY) || defined(CONFIG_MACH_DUMMY) || defined(CONFIG_MACH_DUMMY)
		rc = imx214_800m_fov77_match_otp_info();
#endif
	}
	return rc;
}

static struct msm_sensor_fn_t imx214_800m_fov77_sensor_func_tbl = {
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = imx214_800m_fov77_sensor_match_id,
	.sensor_i2c_read_fuseid = imx214_800m_fov77_read_fuseid,
};

static struct msm_sensor_ctrl_t imx214_800m_fov77_s_ctrl = {
	.sensor_i2c_client = &imx214_800m_fov77_sensor_i2c_client,
	.power_setting_array.power_setting = imx214_800m_fov77_power_setting,
	.power_setting_array.size = ARRAY_SIZE(imx214_800m_fov77_power_setting),
	.msm_sensor_mutex = &imx214_800m_fov77_mut,
	.sensor_v4l2_subdev_info = imx214_800m_fov77_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(imx214_800m_fov77_subdev_info),
	.func_tbl = &imx214_800m_fov77_sensor_func_tbl
};

module_init(imx214_800m_fov77_init_module);
module_exit(imx214_800m_fov77_exit_module);
MODULE_DESCRIPTION("imx214_800m_fov77");
MODULE_LICENSE("GPL v2");
