#include <linux/string.h>
#include <mach/mt_gpio.h>
#include "lcm_drv.h"
#define LCM_PRINT printk
#define LCM_DBG(fmt, arg...) \
	LCM_PRINT("[LCM-OTM8018B-DSI] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(800)

#define REGFLAG_DELAY             							0xFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER

#ifndef TRUE
    #define   TRUE     1
#endif
 
#ifndef FALSE
    #define   FALSE    0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

//static unsigned char esdSwitch =  1;
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_V2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

 struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x00,	1,	{0x00}},
       {0xff,	3,	{0x80, 0x09, 0x01}},
	{0x00,	1,	{0x80}},
	{0xff,	2,	{0x80, 0x09}},
	{0x00,	1,	{0x03}},
	{0xff,	1,	{0x01}},
	{0x00,	1,	{0xB4}},
	{0xC0,	1,	{0x10}},
	{0x00,	1,	{0x82}},
	{0xC5,	1,	{0xA3}},
	{0x00,	1,	{0x90}},
	{0xC5,	2,	{0x96, 0x76}},
	{0x00,	1,	{0x00}},
	{0xD8,	2,	{0x75, 0x73}},
	{0x00,	1,	{0x00}},
	{0xD9,	1,	{0x5E}},
	{0x00,	1,	{0x81}},
	{0xC1,	1,	{0x66}},
	{0x00,	1,	{0xA1}},
	{0xC1,	1,	{0x0E}},//0X08
	{0x00,	1,	{0x89}},
	{0xC4,	1,	{0x08}},
	{0x00,	1,	{0xA2}},
	{0xC0,	3,	{0x1B, 0x00, 0x02}},
	{0x00,	1,	{0x80}},
	{0xC4,	1,	{0x30}},
	{0x00,	1,	{0x81}},
	{0xC4,	1,	{0x83}},
	{0x00,	1,	{0x92}},
	{0xC5,	1,	{0x01}},
	{0x00,	1,	{0xB1}},
	{0xC5,	1,	{0xA9}},
	{0x00,	1,	{0x92}},
	{0xB3,	1,	{0x45}},
	{0x00,	1,	{0x90}},
	{0xB3,	1,	{0x02}},
	{0x00,	1,	{0x80}},
	{0xC0,	5,	{0x00, 0x58, 0x00, 0x14, 0x16}},
	{0x00,	1,	{0xC1}},
	{0xF5,	1,	{0xC0}},
	{0x00,	1,	{0x90}},
	{0xC0,	6,	{0x00, 0x44, 0x00, 0x00, 0x00, 0x03}},
	{0x00,	1,	{0xA6}},
	{0xC1,	3,	{0x00, 0x00, 0x00}},
	{0x00,	1,	{0x80}},
	{0xCE,	12,	{0x87, 0x03, 0x00, 0x85,
				0x03, 0x00, 0x86, 0x03,
				0x00, 0x84, 0x03, 0x00,}},
	{0x00,	1,	{0xA0}},
	{0xCE,	14,	{0x38, 0x03, 0x03, 0x58,
				0x00, 0x00, 0x00, 0x38,
				0x02, 0x03, 0x59, 0x00,
				0x00, 0x00}},
	{0x00,	1,	{0xB0}},
	{0xCE,	14,	{0x38, 0x01, 0x03, 0x5A,
				0x00, 0x00, 0x00, 0x38,
				0x00, 0x03, 0x5B, 0x00,
				0x00, 0x00}},
	{0x00,	1,	{0xC0}},
	{0xCE,	14,	{0x30, 0x00, 0x03, 0x5C,
				0x00, 0x00, 0x00, 0x30,
				0x01, 0x03, 0x5D, 0x00,
				0x00, 0x00}},
	{0x00,	1,	{0xD0}},
	{0xCE,	14,	{0x30, 0x02, 0x03, 0x5E,
				0x00, 0x00, 0x00, 0x30,
				0x03, 0x03, 0x5F, 0x00,
				0x00, 0x00}},
	{0x00,	1,	{0xC7}},
	{0xCF,	1,	{0x00}},
	{0x00,	1,	{0xC9}},
	{0xCF,	1,	{0x00}},
	{0x00,	1,	{0xC4}},
	{0xCB,	6,	{0x04, 0x04, 0x04, 0x04,
				0x04, 0x04}},
	{0x00,	1,	{0xD9}},
	{0xCB,	6,	{0x04, 0x04, 0x04, 0x04,
				0x04, 0x04}},
	{0x00,	1,	{0x84}},
	{0xCC,	6,	{0x0C, 0x0A, 0x10, 0x0E,
				0x03, 0x04}},
	{0x00,	1,	{0x9E}},
	{0xCC,	1,	{0x0B}},
	{0x00,	1,	{0xA0}},
	{0xCC,	5,	{0x09, 0x0F, 0x0D, 0x01,
				0x02}},
	{0x00,	1,	{0xB4}},
	{0xCC,	6,	{0x0D, 0x0F, 0x09, 0x0B,
				0x02, 0x01}},
	{0x00,	1,	{0xCE}},
	{0xCC,	1,	{0x0E}},
	{0x00,	1,	{0xD0}},
	{0xCC,	5,	{0x10, 0x0A, 0x0C, 0x04,
				0x03}},
	//{0x00,	1,	{0x00}},
	{0x3A,	1,	{0x77}},
	{0x11,	1,	{0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0x29,	1,	{0x00}},
	{REGFLAG_DELAY, 50, {}},
	{0x2C,	0,	{}},
	
	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};



static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 50, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
		params->type   							= LCM_TYPE_DSI;
		params->width  							= FRAME_WIDTH;
		params->height 							= FRAME_HEIGHT;
		params->dbi.te_mode 		    				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity	    				= LCM_POLARITY_RISING;
		params->dsi.mode   						= BURST_VDO_MODE;
		params->dsi.LANE_NUM		    				= LCM_TWO_LANE;
		params->dsi.data_format.color_order 				= LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   				= LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     				= LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      				= LCM_DSI_FORMAT_RGB888;
		params->dsi.packet_size						= 256;
		params->dsi.intermediat_buffer_num 				= 2;
		params->dsi.PS							= LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count						= 480*3;
		params->dsi.vertical_sync_active				= 84;
		params->dsi.vertical_backporch					= 28;
		params->dsi.vertical_frontporch					= 28;
		params->dsi.vertical_active_line				= FRAME_HEIGHT;
		params->dsi.horizontal_sync_active				= 6;
		params->dsi.horizontal_backporch				= 33;
		params->dsi.horizontal_frontporch				= 33;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		params->dsi.horizontal_blanking_pixel 				= 60;
		params->dsi.compatibility_for_nvk 				= 0;
		params->dsi.pll_div1						= 1;
		params->dsi.pll_div2						= 1;	
		params->dsi.fbk_div 						= 28;
}


static void lcm_init(void)
{
	LCM_DBG(" Magnum lcm_init...\n");
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(100);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}

static struct LCM_setting_table read_protect[] = {
	{0x00,	1,	{0x00}},
       {0xff,	3,	{0x80, 0x09, 0x01}},       	     	  			
	{0x00,	1,	{0x80}},
       {0xff,	2,	{0x80, 0x09}},

        {0x00,  1,      {0xC6}},
        {0xB0,  1,      {0x03}},
};

static void lcm_resume(void)
{
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

static unsigned int lcm_compare_id(void)
{
    return 1;
}


LCM_DRIVER otm8018b_dsi_vdo_lcm_drv = 
{
    .name			= "otm8018b_dsi_vdo_lcm_drv_dnj",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};
