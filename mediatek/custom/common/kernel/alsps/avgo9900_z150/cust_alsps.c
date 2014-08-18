#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 0,
	.polling_mode_ps =0,
	.polling_mode_als =1,

    .power_id   = MT65XX_POWER_LDO_VGP,    /*LDO is not used*/
    .power_vol  = VOL_2800,// VOL_DEFAULT,          
    .i2c_addr   = {0x72, 0x48, 0x78, 0x00},
#if defined(DOOV_T660K)
    .als_level  = { 50,  2000,10000,10000,10000,10000,10000,10000,10000,10000,10000,10000,10000,10000,65535},
    .als_value  = { 15, 200, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000},
#else
    /*Lenovo-sw chenlj2 add 2011-06-03,modify parameter below two lines*/
    .als_level  = { 5, 10,  25,   50,  100, 150,  200, 400, 1000,  1500, 2000, 3000, 5000, 8000, 65535},
    .als_value  = {10, 50,  100,  150, 200, 250,  280,  280, 1600,  1600,  1600,  6000,  6000, 9000,  10240, 10240},
#endif
    //.als_level  = { 4, 40,  80,   120,   160, 250,  400, 800, 1200,  1600, 2000, 3000, 5000, 10000, 65535},
    //.als_value  = {10, 20,20,  120, 120, 280,  280,  280, 1600,  1600,  1600,  6000,  6000, 9000,  10240, 10240},
#if defined(DOOV_T660K)
    .ps_threshold_high = 470,
    .ps_threshold_low = 370,
#else
    .ps_threshold_high = 350,
    .ps_threshold_low = 250,
#endif
    .ps_threshold = 350,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}
int AVGO9900_CMM_PPCOUNT_VALUE = 0x04;
int AVGO9900_CMM_CONTROL_VALUE = 0x60;  //50MA,1gain;  Again:0--0;1---8;2--16;3---120



#if defined(DOOV_T660K)
int ZOOM_TIME = 4315;
#else
int ZOOM_TIME = 5000;//1000;
#endif

