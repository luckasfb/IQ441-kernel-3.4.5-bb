/******************************************************************************
 * mtk_tpd.c - MTK Android Linux Touch Panel Device Driver               *
 *                                                                            *
 * Copyright 2008-2009 MediaTek Co.,Ltd.                                      *
 *                                                                            *
 * DESCRIPTION:                                                               *
 *     this file provide basic touch panel event to input sub system          *
 *                                                                            *
 * AUTHOR:                                                                    *
 *     Kirby.Wu (mtk02247)                                                    *
 *                                                                            *
 * NOTE:                                                                      *
 * 1. Sensitivity for touch screen should be set to edge-sensitive.           *
 *    But in this driver it is assumed to be done by interrupt core,          *
 *    though not done yet. Interrupt core may provide interface to            *
 *    let drivers set the sensitivity in the future. In this case,            *
 *    this driver should set the sensitivity of the corresponding IRQ         *
 *    line itself.                                                            *
 ******************************************************************************/

#include "tpd.h"

/* function definitions */
static int __init  tpd_device_init(void);
static void __exit tpd_device_exit(void);
static int         tpd_probe(struct platform_device *pdev);
static int tpd_remove(struct platform_device *pdev);

extern void        tpd_suspend(struct early_suspend *h);
extern void        tpd_resume(struct early_suspend *h);
extern void tpd_button_init(void);

//int tpd_load_status = 0; //0: failed, 1: sucess
int tpd_register_flag=0;
/* global variable definitions */
struct tpd_device  *tpd = 0;
static struct tpd_driver_t tpd_driver_list[TP_DRV_MAX_COUNT] ;//= {0};

static struct platform_driver tpd_driver = {
    .remove     = tpd_remove,
    .shutdown   = NULL,
    .probe      = tpd_probe,
    #ifndef CONFIG_HAS_EARLYSUSPEND
    .suspend    = NULL,
    .resume     = NULL,
    #endif
    .driver     = {
        .name = TPD_DEVICE,
    },
};

/*20091105, Kelvin, re-locate touch screen driver to earlysuspend*/
#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend MTK_TS_early_suspend_handler = 
{
	//Gionee wanght modify level on 2012-06-18 form + to -
    .level = EARLY_SUSPEND_LEVEL_DISABLE_FB-1,    
    .suspend = NULL,
    .resume  = NULL,
};
#endif
//gionee wld porting from mt6573 20120328 begin
#if defined (GN_MTK_BSP)
static ssize_t show_update_pro(struct device_driver *ddri, char *buf)
{
    TPD_DMESG(" show_update_fw_pro\n");
    char ver[20];
    int time_reg = 0,i;
    for(i = 1; i < TP_DRV_MAX_COUNT; i++)
    {
        /* add tpd driver into list */
        if(tpd_driver_list[i].tpd_device_name != NULL && tpd_driver_list[i].update_show != NULL)
        {
            tpd_driver_list[i].update_show(ver);
            TPD_DMESG(" store_update_fw_pro***-%d\n",i);
            break;
        }
    }       

    return snprintf(buf, VAR_CHAR_NUM_MAX, "%s\n", ver);        
}
static ssize_t store_update_pro(struct device_driver *ddri, char *buf)
{
    TPD_DMESG(" store_update_fw_pro\n");
    char strbuf[10]="OK\n";

    int i;
    TPD_DMESG(" store_update_fw_pro--%s----\n",buf);
    if(!strcmp(buf , "update"))
    {
        for(i = 1; i < TP_DRV_MAX_COUNT; i++)
        {
            /* add tpd driver into list */
            if(tpd_driver_list[i].tpd_device_name != NULL)
            {
                tpd_driver_list[i].update();
                TPD_DMESG(" store_update_fw_pro***-%d\n",i);
            }
        }    
    }

    return snprintf(buf, VAR_CHAR_NUM_MAX, "%s\n", strbuf);        
}

static DRIVER_ATTR(update, S_IRUGO |S_IWUSR, show_update_pro, store_update_pro);

static struct driver_attribute *tp_update_attr_list[] = {
    &driver_attr_update,    
};
static int tpd_create_attr(struct device_driver *driver) 
{
    int idx, err = 0;
    int num = (int)(sizeof(tp_update_attr_list)/sizeof(tp_update_attr_list[0]));
    if (driver == NULL)
    {
        return -EINVAL;
    }
    TPD_DMESG("tpd_create_attr ----0 \n");
    for(idx = 0; idx < num; idx++)
    {
        if(err = driver_create_file(driver, tp_update_attr_list[idx]))
        {            
            TPD_DMESG("TPD  driver_create_file ");
            break;
        }
    }    
    TPD_DMESG("TPD  driver_create_file success---2\n");
    return err;
}
static int tpd_delete_attr(struct device_driver *driver)
{
    int idx ,err = 0;
    int num = (int)(sizeof(tp_update_attr_list)/sizeof(tp_update_attr_list[0]));
    if (driver == NULL)
    {
        return -EINVAL;
    }

    for(idx = 0; idx < num; idx++)
    {
        driver_remove_file(driver,tp_update_attr_list[idx]);
    }


    return err;
}

#endif
//gionee wld porting from mt6573 20120328 end
static struct tpd_driver_t *g_tpd_drv = NULL;
/* Add driver: if find TPD_TYPE_CAPACITIVE driver sucessfully, loading it */
int tpd_driver_add(struct tpd_driver_t *tpd_drv)
{
	int i;
	
	if(g_tpd_drv != NULL)
	{
		TPD_DMESG("touch driver exist \n");
		return -1;
	}
	/* check parameter */
	if(tpd_drv == NULL)
	{
		return -1;
	}
	/* R-touch */
	if(strcmp(tpd_drv->tpd_device_name, "generic") == 0)
	{
			tpd_driver_list[0].tpd_device_name = tpd_drv->tpd_device_name;
			tpd_driver_list[0].tpd_local_init = tpd_drv->tpd_local_init;
			tpd_driver_list[0].suspend = tpd_drv->suspend;
			tpd_driver_list[0].resume = tpd_drv->resume;
//gionee wld porting from mt6573 20120328 begin
#if defined (GN_MTK_BSP)
		tpd_driver_list[0].update = tpd_drv->update;
		tpd_driver_list[0].update_show = tpd_drv->update_show;
#endif
//gionee wld porting from mt6573 20120328 end
			tpd_driver_list[0].tpd_have_button = tpd_drv->tpd_have_button;
			return 0;
	}
	for(i = 1; i < TP_DRV_MAX_COUNT; i++)
	{
		/* add tpd driver into list */
		if(tpd_driver_list[i].tpd_device_name == NULL)
		{
			tpd_driver_list[i].tpd_device_name = tpd_drv->tpd_device_name;
			tpd_driver_list[i].tpd_local_init = tpd_drv->tpd_local_init;
			tpd_driver_list[i].suspend = tpd_drv->suspend;
			tpd_driver_list[i].resume = tpd_drv->resume;
//gionee wld porting from mt6573 20120328 begin
#if defined (GN_MTK_BSP)
            tpd_driver_list[i].update = tpd_drv->update;
            tpd_driver_list[i].update_show = tpd_drv->update_show;
#endif
//gionee wld porting from mt6573 20120328 end
			tpd_driver_list[i].tpd_have_button = tpd_drv->tpd_have_button;
			#if 0
			if(tpd_drv->tpd_local_init()==0)
			{
				TPD_DMESG("load %s sucessfully\n", tpd_driver_list[i].tpd_device_name);
				g_tpd_drv = &tpd_driver_list[i];
			}
			#endif
			break;
		}
		if(strcmp(tpd_driver_list[i].tpd_device_name, tpd_drv->tpd_device_name) == 0)
		{
			return 1; // driver exist
		}
	}
	
	return 0;
}

int tpd_driver_remove(struct tpd_driver_t *tpd_drv)
{
	int i = 0;
	/* check parameter */
	if(tpd_drv == NULL)
	{
		return -1;
	}
//gionee wld porting from mt6573 20120328 begin
#if defined (GN_MTK_BSP)
    if(i = tpd_delete_attr(&tpd_driver.driver))
    {
        TPD_DMESG("TPD_delete_attr fail: %d\n", i);
    }
#endif
//gionee wld porting from mt6573 20120328 end
	for(i = 0; i < TP_DRV_MAX_COUNT; i++)
	{
		/* find it */
		if (strcmp(tpd_driver_list[i].tpd_device_name, tpd_drv->tpd_device_name) == 0)
		{
			memset(&tpd_driver_list[i], 0, sizeof(struct tpd_driver_t));
			break;
		}
	}
	return 0;
}

/* touch panel probe */
static int tpd_probe(struct platform_device *pdev) {
		int  touch_type = 1; // 0:R-touch, 1: Cap-touch
		int i=0;
	  TPD_DMESG("enter %s, %d\n", __FUNCTION__, __LINE__); 
	  /* Select R-Touch */
	 // if(g_tpd_drv == NULL||tpd_load_status == 0) 
#if 0
	 if(g_tpd_drv == NULL) 
	  {
	  	g_tpd_drv = &tpd_driver_list[0];
	  	/* touch_type:0: r-touch, 1: C-touch */
	  	touch_type = 0;
	  	TPD_DMESG("Generic touch panel driver\n");
	  }
	  	
    #ifdef CONFIG_HAS_EARLYSUSPEND
    MTK_TS_early_suspend_handler.suspend = g_tpd_drv->suspend;
    MTK_TS_early_suspend_handler.resume = g_tpd_drv->resume;
    register_early_suspend(&MTK_TS_early_suspend_handler);
    #endif
    #endif
    if((tpd=(struct tpd_device*)kmalloc(sizeof(struct tpd_device), GFP_KERNEL))==NULL) return -ENOMEM;
    memset(tpd, 0, sizeof(struct tpd_device));

    /* allocate input device */
    if((tpd->dev=input_allocate_device())==NULL) { kfree(tpd); return -ENOMEM; }
  
    TPD_RES_X = simple_strtoul(LCM_WIDTH, NULL, 0);
    TPD_RES_Y = simple_strtoul(LCM_HEIGHT, NULL, 0);
  
    printk("mtk_tpd: TPD_RES_X = %d, TPD_RES_Y = %d\n", TPD_RES_X, TPD_RES_Y);
  
    tpd_mode = TPD_MODE_NORMAL;
    tpd_mode_axis = 0;
    tpd_mode_min = TPD_RES_Y/2;
    tpd_mode_max = TPD_RES_Y;
    tpd_mode_keypad_tolerance = TPD_RES_X*TPD_RES_X/1600;
    /* struct input_dev dev initialization and registration */
    tpd->dev->name = TPD_DEVICE;
    set_bit(EV_ABS, tpd->dev->evbit);
//gionee wld porting from mt6573 20120328 begin
#if defined (GN_MTK_BSP)
    set_bit(EV_SYN, tpd->dev->evbit);
#endif
//gionee wld porting from mt6573 20120328 end
    set_bit(EV_KEY, tpd->dev->evbit);
    set_bit(ABS_X, tpd->dev->absbit);
    set_bit(ABS_Y, tpd->dev->absbit);
    set_bit(ABS_PRESSURE, tpd->dev->absbit);
    set_bit(BTN_TOUCH, tpd->dev->keybit);
    set_bit(INPUT_PROP_DIRECT, tpd->dev->propbit);
#if 1    
  for(i = 1; i < TP_DRV_MAX_COUNT; i++)
	{
    		/* add tpd driver into list */
		if(tpd_driver_list[i].tpd_device_name != NULL)
		{
			tpd_driver_list[i].tpd_local_init();
			//msleep(1);
			if(tpd_load_status ==1) {
				TPD_DMESG("[mtk-tpd]tpd_probe, tpd_driver_name=%s\n", tpd_driver_list[i].tpd_device_name);
				g_tpd_drv = &tpd_driver_list[i];
				break;
			}
		}    
  }
	 if(g_tpd_drv == NULL) {
	 	if(tpd_driver_list[0].tpd_device_name != NULL) {
	  	g_tpd_drv = &tpd_driver_list[0];
	  	/* touch_type:0: r-touch, 1: C-touch */
	  	touch_type = 0;
	  	g_tpd_drv->tpd_local_init();
	  	TPD_DMESG("[mtk-tpd]Generic touch panel driver\n");
	  } else {
	  	TPD_DMESG("[mtk-tpd]cap touch and Generic touch both are not loaded!!\n");
	  	return 0;
	  	} 
	  }	
    #ifdef CONFIG_HAS_EARLYSUSPEND
    MTK_TS_early_suspend_handler.suspend = g_tpd_drv->suspend;
    MTK_TS_early_suspend_handler.resume = g_tpd_drv->resume;
    register_early_suspend(&MTK_TS_early_suspend_handler);
    #endif		  
#endif	  
//#ifdef TPD_TYPE_CAPACITIVE
		/* TPD_TYPE_CAPACITIVE handle */
		if(touch_type == 1){
			
		set_bit(ABS_MT_TRACKING_ID, tpd->dev->absbit);
    	set_bit(ABS_MT_TOUCH_MAJOR, tpd->dev->absbit);
    	set_bit(ABS_MT_TOUCH_MINOR, tpd->dev->absbit);
    	set_bit(ABS_MT_POSITION_X, tpd->dev->absbit);
    	set_bit(ABS_MT_POSITION_Y, tpd->dev->absbit);
        #if 0 // linux kernel update from 2.6.35 --> 3.0
    	tpd->dev->absmax[ABS_MT_POSITION_X] = TPD_RES_X;
    	tpd->dev->absmin[ABS_MT_POSITION_X] = 0;
    	tpd->dev->absmax[ABS_MT_POSITION_Y] = TPD_RES_Y;
    	tpd->dev->absmin[ABS_MT_POSITION_Y] = 0;
    	tpd->dev->absmax[ABS_MT_TOUCH_MAJOR] = 100;
    	tpd->dev->absmin[ABS_MT_TOUCH_MINOR] = 0;
#else
		input_set_abs_params(tpd->dev, ABS_MT_POSITION_X, 0, TPD_RES_X, 0, 0);
		input_set_abs_params(tpd->dev, ABS_MT_POSITION_Y, 0, TPD_RES_Y, 0, 0);
		input_set_abs_params(tpd->dev, ABS_MT_TOUCH_MAJOR, 0, 100, 0, 0);
		input_set_abs_params(tpd->dev, ABS_MT_TOUCH_MINOR, 0, 100, 0, 0); 
//gionee wld porting from mt6573 20120328 begin
#if defined (GN_MTK_BSP)
		input_set_abs_params(tpd->dev, ABS_MT_TRACKING_ID, 0, 3, 0, 0); 
#endif	
//gionee wld porting from mt6573 20120328 end
#endif
    	TPD_DMESG("Cap touch panel driver\n");
  	}
//#endif
    #if 0 //linux kernel update from 2.6.35 --> 3.0
    tpd->dev->absmax[ABS_X] = TPD_RES_X;
    tpd->dev->absmin[ABS_X] = 0;
    tpd->dev->absmax[ABS_Y] = TPD_RES_Y;
    tpd->dev->absmin[ABS_Y] = 0;
	
    tpd->dev->absmax[ABS_PRESSURE] = 255;
    tpd->dev->absmin[ABS_PRESSURE] = 0;
    #else
		input_set_abs_params(tpd->dev, ABS_X, 0, TPD_RES_X, 0, 0);
		input_set_abs_params(tpd->dev, ABS_Y, 0, TPD_RES_Y, 0, 0);
		input_abs_set_res(tpd->dev, ABS_X, TPD_RES_X);
		input_abs_set_res(tpd->dev, ABS_Y, TPD_RES_Y);
		input_set_abs_params(tpd->dev, ABS_PRESSURE, 0, 255, 0, 0);

    #endif
    if(input_register_device(tpd->dev))
        TPD_DMESG("input_register_device failed.(tpd)\n");
    else
			tpd_register_flag = 1;
    /* init R-Touch */
    #if 0
	  if(touch_type == 0) 
	  {
	  	g_tpd_drv->tpd_local_init();
	  }    
		#endif
    if(g_tpd_drv->tpd_have_button)
    {
    	tpd_button_init();
        }
//gionee wld porting from mt6573 20120328 begin
#if defined (GN_MTK_BSP)
    if(i = tpd_create_attr(&tpd_driver.driver))
    {
        TPD_DMESG(" tpd create attribute err = %d\n", i);
    }
#endif
//gionee wld porting from mt6573 20120328 end
    return 0;
}

static int tpd_remove(struct platform_device *pdev)
{
	   input_unregister_device(tpd->dev);
    #ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&MTK_TS_early_suspend_handler);
    #endif
    return 0;
}

/* called when loaded into kernel */
static int __init tpd_device_init(void) {
    printk("MediaTek touch panel driver init\n");
    if(platform_driver_register(&tpd_driver)!=0) {
        TPD_DMESG("unable to register touch panel driver.\n");
        return -1;
    }   
    return 0;
}

/* should never be called */
static void __exit tpd_device_exit(void) {
    TPD_DMESG("MediaTek touch panel driver exit\n");
    //input_unregister_device(tpd->dev);
    platform_driver_unregister(&tpd_driver);
    #ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&MTK_TS_early_suspend_handler);
    #endif
}

module_init(tpd_device_init);
module_exit(tpd_device_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek touch panel driver");
MODULE_AUTHOR("Kirby Wu<kirby.wu@mediatek.com>");

