#include <cust_leds.h>
#include <mach/mt_pwm.h>

#include <linux/kernel.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>

// LuckAs add PWM1
//#define GN_MTK_BL_GPAD
#define GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM1
// LuckAs end
//gionee weiwie  add for clear up code CR00680404 begain
#if defined  GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM1
    #define PWM_NU PWM1
    #define GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM
#elif defined GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM2
    #define PWM_NU PWM2
	#define GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM
#elif defined GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM3
    #define PWM_NU PWM3
	#define GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM
#elif defined  GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM4
    #define PWM_NU PWM4
	#define GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM
#elif defined  GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM5
    #define PWM_NU PWM5
	#define GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM
#elif defined  GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM6
    #define PWM_NU PWM6
    #define GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM
#elif defined  GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM7
    #define PWM_NU PWM7
	#define GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM
#endif
//gionee weiwei add for clear up code CR00680404 end

extern int mtkfb_set_backlight_level(unsigned int level);
extern int mtkfb_set_backlight_pwm(int div);
//add by chendong 2012-05-13
#ifdef GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM
#define config_data {1,CLK_DIV2,65,65}  //{1,CLK_DIV2,65,65}
// orig
//#define config_data {1,CLK_DIV2,32,32}  //{1,CLK_DIV2,65,65}
#endif
//add by chendong end
unsigned int brightness_mapping(unsigned int level)
{
//add by chendong 2012-05-13
#ifdef GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM
	//add by yangqingbing for GFW135 backlight specially
	#ifdef GN_MTK_BSP_GFW135_BACKLIGHT
		return	(level+1)/4;
	#else		
		if (level < 30)
			return (level >> 1) + 7;
		else if (level <= 120)
			return (level >> 2) + 14;
		else if (level <= 160)
			return level / 5 + 20;
		else
			return (level >> 3) + 33; 	
	#endif
#else
    unsigned int mapped_level;
    
    mapped_level = level;
       
	return mapped_level;

#endif
//add by chendong end
}

unsigned int Cust_SetBacklight(int level, int div)
{
    mtkfb_set_backlight_pwm(div);
    mtkfb_set_backlight_level(brightness_mapping(level));
    return 0;
}

//gionee pengchao 20120925 add for  CR00693470 start
#if defined(GN_SSD2825_SMD_S6E8AA)
extern void S6e88a_Lcm_ACL_Enable(unsigned int on_off);
#endif


#if defined(GN_AMOLED_LCM_ACL_SUPPORT)
unsigned int Cust_EnableACL(int on_off, int div)
{
#if defined(GN_SSD2825_SMD_S6E8AA)
	 S6e88a_Lcm_ACL_Enable(on_off);
#endif
}
#endif
//gionee pengchao 20120925 add for  CR00693470 end

static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_NONE, -1,{0}},
	{"green",             MT65XX_LED_MODE_NONE, -1,{0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_BUTTON,{0}},
/* Gionee Qux. GPAD */
#if defined(GN_MTK_BL_GPAD)
    {"lcd-backlight",     MT65XX_LED_MODE_PWM, PWM1,{0}},
//add by chendong 2012-05-13
#elif defined(GN_MTK_BSP_LCD_BACKLIGHT_USE_PWM)
	{"lcd-backlight",     MT65XX_LED_MODE_PWM, PWM_NU, config_data},
#else
	{"lcd-backlight",     MT65XX_LED_MODE_CUST, (int)Cust_SetBacklight,{0}},
#endif
//add by chendong end

//gionee pengchao 20120925 add for  CR00693470 start
#if defined(GN_AMOLED_LCM_ACL_SUPPORT)
	{"amoled_lcm_acl",        MT65XX_LED_MODE_CUST, (int)Cust_EnableACL,{0}},
#endif
//gionee pengchao 20120925 add for  CR00693470 end
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

