#include "ov7670.h"
#include <stdio.h>
#include "main.h"

static I2C_HandleTypeDef *phi2c;
static DCMI_HandleTypeDef *phdcmi;
//static UART_HandleTypeDef *phuart;
//static DMA_HandleTypeDef *phdma_dcmi;
//static uint32_t destAddressForContinuousMode;
//static void (* s_cbHsync)(uint32_t h);
//static void (* s_cbVsync)(uint32_t v);
//static uint32_t s_currentH;
//static uint32_t s_currentV;

const unsigned char OV7670_INIT[][2] = {
	{REG_CLKRC, 0x8D},
	{DBLV, 0xCA},
	{REG_MVFP, 0x07},
	{REG_TSLB,  0x04},	/* OV */
	{REG_COM7, 0},	/* VGA */
	/*
	 * Set the hardware window.  These values from OV don't entirely
	 * make sense - hstop is less than hstart.  But they work...
	 */
	{REG_HSTART, 0x13},	{REG_HSTOP, 0x01},
	{REG_HREF, 0xb6},	{REG_VSTART, 0x02},
	{REG_VSTOP, 0x7a},	{REG_VREF, 0x0a},
	{REG_SCALING_XSC, 0x3A}, {REG_SCALING_YSC, 0x35}, {REG_SCALING_PCLK_DELAY, 0x02},

	{REG_COM3, 0x04},	{REG_COM14, 0},
	/* Mystery scaling numbers */
	{0x70, 0x3a},		{0x71, 0x35},
	{0x72, 0x11},		{0x73, 0xf0},
	{0xa2,/* 0x02 changed to 1*/1},{REG_COM10, COM10_VS_NEG},
	/* Gamma curve values */
	{0x7a, 0x20},		{0x7b, 0x10},
	{0x7c, 0x1e},		{0x7d, 0x35},
	{0x7e, 0x5a},		{0x7f, 0x69},
	{0x80, 0x76},		{0x81, 0x80},
	{0x82, 0x88},		{0x83, 0x8f},
	{0x84, 0x96},		{0x85, 0xa3},
	{0x86, 0xaf},		{0x87, 0xc4},
	{0x88, 0xd7},		{0x89, 0xe8},
	/* AGC and AEC parameters.  Note we start by disabling those features,
	   then turn them only after tweaking the values. */
	{REG_COM8, COM8_FASTAEC | COM8_AECSTEP},
	{REG_GAIN, 0},	{REG_AECH, 0},
	{REG_COM4, 0x40}, /* magic reserved bit */
	{REG_COM9, 0x18}, /* 4x gain + magic rsvd bit */
	{REG_BD50MAX, 0x05},	{REG_BD60MAX, 0x07},
	{REG_AEW, 0x95},	{REG_AEB, 0x33},
	{REG_VPT, 0xe3},	{REG_HAECC1, 0x78},
	{REG_HAECC2, 0x68},	{0xa1, 0x03}, /* magic */
	{REG_HAECC3, 0xd8},	{REG_HAECC4, 0xd8},
	{REG_HAECC5, 0xf0},	{REG_HAECC6, 0x90},
	{REG_HAECC7, 0x94},
	{REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_AGC|COM8_AEC},
	{0x30,0},{0x31,0},//disable some delays
	/* Almost all of these are magic "reserved" values.  */
	{REG_COM5, 0x61},	{REG_COM6, 0x4b},
	{0x16, 0x02},		{REG_MVFP, 0x07},
	{0x21, 0x02},		{0x22, 0x91},
	{0x29, 0x07},		{0x33, 0x0b},
	{0x35, 0x0b},		{0x37, 0x1d},
	{0x38, 0x71},		{0x39, 0x2a},
	{REG_COM12, 0x78},	{0x4d, 0x40},
	{0x4e, 0x20},		{REG_GFIX, 0},
	/*{0x6b, 0x4a},*/		{0x74,0x10},
	{0x8d, 0x4f},		{0x8e, 0},
	{0x8f, 0},		{0x90, 0},
	{0x91, 0},		{0x96, 0},
	{0x9a, 0},		{0xb0, 0x84},
	{0xb1, 0x0c},		{0xb2, 0x0e},
	{0xb3, 0x82},		{0xb8, 0x0a},

	/* More reserved magic, some of which tweaks white balance */
	{0x43, 0x0a},		{0x44, 0xf0},
	{0x45, 0x34},		{0x46, 0x58},
	{0x47, 0x28},		{0x48, 0x3a},
	{0x59, 0x88},		{0x5a, 0x88},
	{0x5b, 0x44},		{0x5c, 0x67},
	{0x5d, 0x49},		{0x5e, 0x0e},
	{0x6c, 0x0a},		{0x6d, 0x55},
	{0x6e, 0x11},		{0x6f, 0x9e}, /* it was 0x9F "9e for advance AWB" */
	{0x6a, 0x40},		{REG_BLUE, 0x40},
	{REG_RED, 0x60},
	{REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_AGC|COM8_AEC|COM8_AWB},

	/* Matrix coefficients */
	{0x4f, 0x80},		{0x50, 0x80},
	{0x51, 0},		{0x52, 0x22},
	{0x53, 0x5e},		{0x54, 0x80},
	{0x58, 0x9e},

	{REG_COM16, COM16_AWBGAIN},	{REG_EDGE, 0},
	{0x75, 0x05},		{REG_REG76, 0xe1},
	{0x4c, 0},		{0x77, 0x01},
	{REG_COM13, /*0xc3*/0x48},	{0x4b, 0x09},
	{0xc9, 0x60},		/*{REG_COM16, 0x38},*/
	{0x56, 0x40},

	{0x34, 0x11},		{REG_COM11, COM11_EXP|COM11_HZAUTO},
	{0xa4, 0x82/*Was 0x88*/},		{0x96, 0},
	{0x97, 0x30},		{0x98, 0x20},
	{0x99, 0x30},		{0x9a, 0x84},
	{0x9b, 0x29},		{0x9c, 0x03},
	{0x9d, 0x4c},		{0x9e, 0x3f},
	{0x78, 0x04},

	/* Extra-weird stuff.  Some sort of multiplexor register */
	{0x79, 0x01},		{0xc8, 0xf0},
	{0x79, 0x0f},		{0xc8, 0x00},
	{0x79, 0x10},		{0xc8, 0x7e},
	{0x79, 0x0a},		{0xc8, 0x80},
	{0x79, 0x0b},		{0xc8, 0x01},
	{0x79, 0x0c},		{0xc8, 0x0f},
	{0x79, 0x0d},		{0xc8, 0x20},
	{0x79, 0x09},		{0xc8, 0x80},
	{0x79, 0x02},		{0xc8, 0xc0},
	{0x79, 0x03},		{0xc8, 0x40},
	{0x79, 0x05},		{0xc8, 0x30},
	{0x79, 0x26},
	{REG_COM10, 32},

	{0xff, 0xff},	/* END MARKER */
};

const unsigned char OV7670_YUV[][2] = {
	{REG_COM7, 0x00},	/* Selects YUV mode */
	{REG_RGB444, 0x00},	/* No RGB444 please */
	{REG_COM1, 0x00},
	{REG_COM15, COM15_R00FF},
	{REG_COM9, 0x6A},	/* 128x gain ceiling; 0x8 is reserved bit */
	{0x4F, 0x80},		/* "matrix coefficient 1" */
	{0x50, 0x80},		/* "matrix coefficient 2" */
	{0x51, 0x00},		/* vb */
	{0x52, 0x22},		/* "matrix coefficient 4" */
	{0x53, 0x5E},		/* "matrix coefficient 5" */
	{0x54, 0x80},		/* "matrix coefficient 6" */
	{REG_COM13,/*COM13_GAMMA|*/COM13_UVSAT},
	{0xFF, 0xFF},		/* END MARKER */
};

const unsigned char OV7670_RGB565[][2] = {
	{REG_COM7, COM7_RGB}, /* Selects RGB mode */
	{REG_RGB444, 0x00},	  /* No RGB444 please */
	{REG_COM1, 0x00},
	{REG_COM15, COM15_RGB565|COM15_R00FF},
	{REG_COM9, 0x6A},	 /* 128x gain ceiling; 0x8 is reserved bit */
	{0x4F, 0xB3},		 /* "matrix coefficient 1" */
	{0x50, 0xB3},		 /* "matrix coefficient 2" */
	{0x51, 0x00},		 /* vb */
	{0x52, 0x3D},		 /* "matrix coefficient 4" */
	{0x53, 0xA7},		 /* "matrix coefficient 5" */
	{0x54, 0xE4},		 /* "matrix coefficient 6" */
	{REG_COM13, /*COM13_GAMMA|*/COM13_UVSAT},
	{0xFF, 0xFF},		/* END MARKER */
};

const unsigned char OV7670_QQVGA[][2] = {
	{REG_COM14, 0x1A},	// divide by 4
	{0x72, 0x22},		// downsample by 4
	{0x73, 0xF2},		// divide by 4
	{REG_HSTART,0x16},
	{REG_HSTOP,0x04},
	{REG_HREF,0xA4},		   
	{REG_VSTART,0x02},
	{REG_VSTOP,0x7A},
	{REG_VREF,0x0A},
	{0xFF, 0xFF},		/* END MARKER */
};

const unsigned char OV7670_QVGA[][2] = {
	{REG_COM14, 0x19},
	{0x72, 0x11},
	{0x73, 0xF1},
	{REG_HSTART,0x16},
	{REG_HSTOP,0x04},
	{REG_HREF,0x24},
	{REG_VSTART,0x02},
	{REG_VSTOP,0x7A},
	{REG_VREF,0x0A},
	{0xFF, 0xFF},	/* END MARKER */
};

const unsigned char OV7670_VGA[][2] = {
	{REG_HREF,0xF6},	// was B6  
	{0x17,0x13},		// HSTART
	{0x18,0x01},		// HSTOP
	{0x19,0x02},		// VSTART
	{0x1A,0x7A},		// VSTOP
	{REG_VREF,0x0A},	// VREF
	{0xFF, 0xFF},		/* END MARKER */
};

const unsigned char OV7670_YUV_Init_List[][2] = {
	{REG_RGB444, 0x00}, {REG_COM10, 0x02}, {REG_MVFP, 0x27}, {REG_CLKRC, 0x80},
	{DBLV, 0x0A}, {REG_COM11, 0x0A}, {REG_TSLB, 0x04}, {REG_COM13, 0x88}, {REG_COM7, 0x00},
	{REG_COM17, 0x00}, {REG_COM3, 0x04}, {REG_COM15, 0xC0}, {REG_SCALING_XSC, 0x3A},
	{REG_SCALING_YSC, 0x35}, {REG_SCALING_PCLK_DELAY, 0x02},
	/* End marker */
	{0xFF, 0xFF}
};

const unsigned char OV7670_RGB_Init_List[][2] = {
	{REG_RGB444, 0x00}, {REG_COM10, 0x02}, {REG_MVFP, 0x27}, {REG_CLKRC, 0x80},
	{DBLV, 0x0A}, {REG_COM11, 0x0A}, {REG_TSLB, 0x04}, {REG_COM13, 0x88}, {REG_COM7, 0x04},
	{REG_RGB444, 0x00}, {REG_COM15, 0x10}, {REG_COM3, 0x04}, {REG_CLKRC, 0x80},
	{REG_SCALING_XSC, 0x3A}, {REG_SCALING_YSC, 0x35}, {REG_SCALING_PCLK_DELAY, 0x02},
	/* End marker */
	{0xFF, 0xFF}
};

const unsigned char OV7670_160x120[][2] = {
	{REG_COM14, 0x1A}, {REG_SCALING_DCWCTR, 0x22}, {REG_SCALING_PCLK_DIV, 0xF2}, 
	{REG_HREF, 0xa4}, {REG_HSTART, 0x16}, {REG_HSTOP, 0x04}, {REG_VREF, 0x0a},
	{REG_VSTART, 0x02}, {REG_VSTOP, 0x7a},
	
	/* Gamma curve values */
	{REG_SLOP, 0x20}, {GAM1, 0x1C}, {GAM2, 0x28}, {GAM3, 0x3C}, {GAM4, 0x5A}, 
	{GAM5, 0x68}, {GAM6, 0x76}, {GAM7, 0x80}, {GAM8, 0x88}, {GAM9, 0x8F}, {GAM10, 0x96},
	{GAM11, 0xA3}, {GAM12, 0xAF}, {GAM13, 0xC4}, {GAM14, 0xD7}, {GAM15, 0xE8},
	
	 /* AGC and AEC parameters.  Note we start by disabling those features,
        then turn them only after tweaking the values. */
	{REG_COM8, 0xE0}, {REG_GAIN, 0x00}, {REG_AECH, 0x00}, {REG_COM4, 0x40}, {REG_COM9, 0x18},
	{REG_BD50MAX, 0x05}, {REG_BD60MAX, 0x07}, {REG_AEW, 0x95}, {REG_AEB, 0x33}, {REG_VPT, 0xE3},
	{REG_HAECC1, 0x78}, {REG_HAECC2, 0x68}, {0xA1, 0x03}, {REG_HAECC3, 0xD8}, {REG_HAECC4, 0xD8},
	{REG_HAECC5, 0xF0}, {REG_HAECC6, 0x90}, {REG_HAECC7, 0x94}, {REG_COM8, 0xE5}, 
	
	/* Almost reserved values */
	{REG_COM5, 0x61}, {REG_COM6, 0x4B}, {0x16, 0x02}, {0x21, 0x02}, {0x22, 0x91}, {0x29, 0x07},
	{0x33, 0x0B}, {0x37, 0x1D}, {0x38, 0x71}, {0x39, 0x2A}, {REG_COM12, 0x78}, {0x4D, 0x40},
	{0x4E, 0x20}, {REG_GFIX, 0x00}, {REG_REG74, 0x10}, {0x8D, 0x4F}, {0x8E, 0x00}, {0x8F, 0x00},
	{0x90, 0x00}, {0x91, 0x00}, {REG_DM_LNL, 0x00}, {0x96, 0x00}, {0x9A, 0x80}, {0xB0, 0x84},
	{REG_ABLC1, 0x0C}, {0xB2, 0x0E}, {REG_THL_ST, 0x82}, {0xB8, 0x0A},
	
	/* Almost reserved values (tweaks white balance) */
	{0x43, 0x0A}, {0x44, 0xF0}, {0x45, 0x34}, {0x46, 0x58}, {0x47, 0x28}, {0x48, 0x3A}, {0x59, 0x88},
	{0x5A, 0x88}, {0x5B, 0x44}, {0x5C, 0x67}, {0x5D, 0x49}, {0x5E, 0x0E}, {REG_LCC3, 0x04},
	{REG_LCC4, 0x20}, {REG_LCC5, 0x05}, {REG_LCC6, 0x04}, {REG_LCC7, 0x08}, {AWBCTR3, 0x0A},
	{AWBCTR2, 0x55}, {AWBCTR1, 0x11}, {AWBCTR0, 0x9F}, {GGAIN, 0x40}, {REG_BLUE, 0x40},
	{REG_RED, 0x40}, {REG_COM8, 0xE7}, {REG_COM10, 0x02},
	
	/* Matrix coefficients */
	{MTX1, 0x80}, {MTX2, 0x80}, {MTX3, 0x00}, {MTX4, 0x22}, {MTX5, 0x5E}, {MTX6, 0x80}, {MTXS, 0x9E},
	{REG_COM16, 0x08}, {REG_EDGE, 0x00}, {REG_REG75, 0x05}, {REG_REG76, 0xE1}, {REG_DNSTH, 0x00},
	{REG_REG77, 0x01}, {REG_COM13, 0xC1}, {REG_REG4B, 0x09}, {REG_SATCTR, 0x60}, {REG_COM16, 0x38},
	{REG_CONTRAS, 0x40}, {REG_ARBLM, 0x11}, {REG_COM11, 0x02}, {REG_NT_CTRL, 0x88},
	{0x96, 0x00}, {0x97, 0x30}, {0x98, 0x20}, {0x99, 0x30}, {0x9A, 0x84}, {0x9B, 0x29}, {0x9C, 0x03},
	{REG_BD50ST, 0x4C}, {REG_BD60ST, 0x3F}, {0x78, 0x04},
	
	/* Sort of multiplexor register */
	{0x79, 0x01}, {0xC8, 0xF0}, {0x79, 0x0F}, {0xC8, 0x00}, {0x79, 0x10}, {0xC8, 0x7E}, {0x79, 0x0A},
	{0xC8, 0x80}, {0x79, 0x0B}, {0xC8, 0x01}, {0x79, 0x0C}, {0xC8, 0x0F}, {0x79, 0x0D}, {0xC8, 0x20},
	{0x79, 0x09}, {0xC8, 0x80}, {0x79, 0x02}, {0xC8, 0xC0}, {0x79, 0x03}, {0xC8, 0x40}, {0x79, 0x05},
	{0xC8, 0x30}, {0x79, 0x26}, {REG_COM2, 0x03}, {REG_COM11, 0x42},
	
	/* End marker */
	{0xFF, 0xFF}
};

const unsigned char OV7670_320x240[][2] = {
	{REG_COM14, 0x19}, {REG_SCALING_DCWCTR, 0x11}, {REG_SCALING_PCLK_DIV, 0xF1}, 
	{REG_HREF, 0x24}, {REG_HSTART, 0x16}, {REG_HSTOP, 0x04}, {REG_VREF, 0x0a},
	{REG_VSTART, 0x02}, {REG_VSTOP, 0x7a},
	
	/* Gamma curve values */
	{REG_SLOP, 0x20}, {GAM1, 0x1C}, {GAM2, 0x28}, {GAM3, 0x3C}, {GAM4, 0x55}, 
	{GAM5, 0x68}, {GAM6, 0x76}, {GAM7, 0x80}, {GAM8, 0x88}, {GAM9, 0x8F}, {GAM10, 0x96},
	{GAM11, 0xA3}, {GAM12, 0xAF}, {GAM13, 0xC4}, {GAM14, 0xD7}, {GAM15, 0xE8},
	
	 /* AGC and AEC parameters.  Note we start by disabling those features,
        then turn them only after tweaking the values. */
	{REG_COM8, 0xE0}, {REG_GAIN, 0x00}, {REG_AECH, 0x00}, {REG_COM4, 0x00}, {REG_COM9, 0x28},
	{REG_BD50MAX, 0x05}, {REG_BD60MAX, 0x07}, {REG_AEW, 0x75}, {REG_AEB, 0x63}, {REG_VPT, 0xA5},
	{REG_HAECC1, 0x78}, {REG_HAECC2, 0x68}, {0xA1, 0x03}, {REG_HAECC3, 0xDF}, {REG_HAECC4, 0xDF},
	{REG_HAECC5, 0xF0}, {REG_HAECC6, 0x90}, {REG_HAECC7, 0x94}, {REG_COM8, 0xE5}, 
	
	/* Almost reserved values */
	{REG_COM5, 0x61}, {REG_COM6, 0x4B}, {0x16, 0x02}, {0x21, 0x02}, {0x22, 0x91}, {0x29, 0x07},
	{0x33, 0x0B}, {0x37, 0x1D}, {0x38, 0x71}, {0x39, 0x2A}, {REG_COM12, 0x78}, {0x4D, 0x40},
	{0x4E, 0x20}, {REG_GFIX, 0x00}, {DBLV, 0x00}, {REG_REG74, 0x19}, {0x8D, 0x4F}, {0x8E, 0x00},
	{0x8F, 0x00}, {0x90, 0x00}, {0x91, 0x00}, {REG_DM_LNL, 0x00}, {0x96, 0x00}, {0x9A, 0x80},
	{0xB0, 0x84},	{REG_ABLC1, 0x0C}, {0xB2, 0x0E}, {REG_THL_ST, 0x82}, {0xB8, 0x0A},
	
	/* Almost reserved values (tweaks white balance) */
	{0x43, 0x14}, {0x44, 0xF0}, {0x45, 0x34}, {0x46, 0x58}, {0x47, 0x28}, {0x48, 0x3A}, {0x59, 0x88},
	{0x5A, 0x88}, {0x5B, 0x44}, {0x5C, 0x67}, {0x5D, 0x49}, {0x5E, 0x0E}, {REG_LCC3, 0x04},
	{REG_LCC4, 0x20}, {REG_LCC5, 0x05}, {REG_LCC6, 0x04}, {REG_LCC7, 0x08}, {AWBCTR3, 0x0A},
	{AWBCTR2, 0x55}, {AWBCTR1, 0x11}, {AWBCTR0, 0x9F}, {GGAIN, 0x40}, {REG_BLUE, 0x40},
	{REG_RED, 0x40}, {REG_COM8, 0xE7}, {REG_COM10, 0x02},
	
	/* Matrix coefficients */
	{MTX1, 0x80}, {MTX2, 0x80}, {MTX3, 0x00}, {MTX4, 0x22}, {MTX5, 0x5E}, {MTX6, 0x80}, {MTXS, 0x9E},
	{REG_COM16, 0x08}, {REG_EDGE, 0x00}, {REG_REG75, 0x05}, {REG_REG76, 0xE1}, {REG_DNSTH, 0x00},
	{REG_REG77, 0x01}, {REG_COM13, 0xC2}, {REG_REG4B, 0x09}, {REG_SATCTR, 0x60}, {REG_COM16, 0x38},
	{REG_CONTRAS, 0x40}, {REG_ARBLM, 0x11}, {REG_COM11, 0x02}, {REG_NT_CTRL, 0x89},
	{0x96, 0x00}, {0x97, 0x30}, {0x98, 0x20}, {0x99, 0x30}, {0x9A, 0x84}, {0x9B, 0x29}, {0x9C, 0x03},
	{REG_BD50ST, 0x4C}, {REG_BD60ST, 0x3F}, {0x78, 0x04},
	
	/* Sort of multiplexor register */
	{0x79, 0x01}, {0xC8, 0xF0}, {0x79, 0x0F}, {0xC8, 0x00}, {0x79, 0x10}, {0xC8, 0x7E}, {0x79, 0x0A},
	{0xC8, 0x80}, {0x79, 0x0B}, {0xC8, 0x01}, {0x79, 0x0C}, {0xC8, 0x0F}, {0x79, 0x0D}, {0xC8, 0x20},
	{0x79, 0x09}, {0xC8, 0x80}, {0x79, 0x02}, {0xC8, 0xC0}, {0x79, 0x03}, {0xC8, 0x40}, {0x79, 0x05},
	{0xC8, 0x30}, {0x79, 0x26}, {REG_COM2, 0x03}, {REG_COM11, 0x42},
	
	/* End marker */
	{0xFF, 0xFF}
};

const unsigned char OV7670_640x480[][2] = {
	{REG_CLKRC, 0x01}, {REG_TSLB, 0x04}, {REG_COM7, 0x01}, {DBLV, 0x4A}, {REG_COM3, 0x00},
	{REG_COM14, 0x00}, {REG_HSTART, 0x13}, {REG_HSTOP, 0x04}, {REG_HREF, 0x24}, {REG_VSTART, 0x02},
	{REG_VSTOP, 0x7a}, {REG_VREF, 0x0a}, {REG_SCALING_DCWCTR, 0x11}, {REG_SCALING_PCLK_DIV, 0xF1},
	
	/* Gamma curve values */
	{REG_SLOP, 0x13}, {GAM1, 0x10}, {GAM2, 0x1E}, {GAM3, 0x35}, {GAM4, 0x5A}, 
	{GAM5, 0x69}, {GAM6, 0x76}, {GAM7, 0x80}, {GAM8, 0x88}, {GAM9, 0x8F}, {GAM10, 0x96},
	{GAM11, 0xA3}, {GAM12, 0xAF}, {GAM13, 0xC4}, {GAM14, 0xD7}, {GAM15, 0xE8},
	
	 /* AGC and AEC parameters.  Note we start by disabling those features,
        then turn them only after tweaking the values. */
	{REG_COM8, 0xE0}, {REG_GAIN, 0x00}, {REG_AECH, 0x00}, {REG_COM4, 0x40}, {REG_COM9, 0x18},
	{REG_BD50MAX, 0x05}, {REG_BD60MAX, 0x07}, {REG_AEW, 0x95}, {REG_AEB, 0x33}, {REG_VPT, 0xE3},
	{REG_HAECC1, 0x78}, {REG_HAECC2, 0x68}, {0xA1, 0x03}, {REG_HAECC3, 0xD8}, {REG_HAECC4, 0xD8},
	{REG_HAECC5, 0xF0}, {REG_HAECC6, 0x90}, {REG_HAECC7, 0x94}, {REG_COM8, 0xE5}, 
	
	/* Almost reserved values */
	{REG_COM5, 0x61}, {REG_COM6, 0x4B}, {0x16, 0x02}, {REG_MVFP, 0x02}, {0x21, 0x02}, {0x22, 0x91},
	{0x29, 0x07}, {0x33, 0x0B}, {0x37, 0x1D}, {0x38, 0x71}, {0x39, 0x2A}, {REG_COM12, 0x78}, {0x4D, 0x40},
	{0x4E, 0x20}, {REG_GFIX, 0x00}, {DBLV, 0x0A}, {REG_REG74, 0x10}, {0x8D, 0x4F}, {0x8E, 0x00},
	{0x8F, 0x00}, {0x90, 0x00}, {0x91, 0x00}, {0x96, 0x00}, {0x9A, 0x00},	{0xB0, 0x84},
	{REG_ABLC1, 0x0C}, {0xB2, 0x0E}, {REG_THL_ST, 0x82}, {0xB8, 0x0A},
	
	/* Almost reserved values (tweaks white balance) */
	{0x43, 0x0A}, {0x44, 0xF0}, {0x45, 0x34}, {0x46, 0x58}, {0x47, 0x28}, {0x48, 0x3A}, {0x59, 0x88},
	{0x5A, 0x88}, {0x5B, 0x44}, {0x5C, 0x67}, {0x5D, 0x49}, {0x5E, 0x0E}, {REG_LCC3, 0x0A},
	{REG_LCC4, 0x20}, {REG_LCC5, 0x05}, {REG_LCC6, 0x04}, {REG_LCC7, 0x08}, {AWBCTR3, 0x0A},
	{AWBCTR2, 0x55}, {AWBCTR1, 0x11}, {AWBCTR0, 0x9F}, {GGAIN, 0x40}, {REG_BLUE, 0x40},
	{REG_RED, 0x60}, {REG_COM8, 0xE7},
	
	/* Matrix coefficients */
	{MTX1, 0x80}, {MTX2, 0x80}, {MTX3, 0x00}, {MTX4, 0x22}, {MTX5, 0x5E}, {MTX6, 0x80}, {MTXS, 0x9E},
	{REG_COM16, 0x08}, {REG_EDGE, 0x00}, {REG_REG75, 0x05}, {REG_REG76, 0xE1}, {REG_DNSTH, 0x00},
	{REG_REG77, 0x01}, {REG_COM13, 0xC3}, {REG_REG4B, 0x09}, {REG_SATCTR, 0x60}, {REG_COM16, 0x38},
	{REG_CONTRAS, 0x40}, {REG_ARBLM, 0x11}, {REG_COM11, 0x12}, {REG_NT_CTRL, 0x88},
	{0x96, 0x00}, {0x97, 0x30}, {0x98, 0x20}, {0x99, 0x30}, {0x9A, 0x84}, {0x9B, 0x29}, {0x9C, 0x03},
	{REG_BD50ST, 0x4C}, {REG_BD60ST, 0x3F}, {0x78, 0x04},
	
	/* Sort of multiplexor register */
	{0x79, 0x01}, {0xC8, 0xF0}, {0x79, 0x0F}, {0xC8, 0x00}, {0x79, 0x10}, {0xC8, 0x7E}, {0x79, 0x0A},
	{0xC8, 0x80}, {0x79, 0x0B}, {0xC8, 0x01}, {0x79, 0x0C}, {0xC8, 0x0F}, {0x79, 0x0D}, {0xC8, 0x20},
	{0x79, 0x09}, {0xC8, 0x80}, {0x79, 0x02}, {0xC8, 0xC0}, {0x79, 0x03}, {0xC8, 0x40}, {0x79, 0x05},
	{0xC8, 0x30}, {0x79, 0x26},
	
	/* End marker */
	{0xFF, 0xFF}
};

/**
 * Camera Reset.
 * @param p_hi2c Pointer to I2C interface.
 * @param p_hdcmi Pointer to DCMI interface.
 */
CAMERA_STAT OV7670_Init(I2C_HandleTypeDef *p_hi2c, DCMI_HandleTypeDef *p_hdcmi) {
	phi2c = p_hi2c;
	phdcmi = p_hdcmi;

	// Hardware reset
	HAL_GPIO_WritePin(CAMERA_RESET_GPIO_Port, CAMERA_RESET_Pin, GPIO_PIN_RESET);
	osDelay(100);
	HAL_GPIO_WritePin(CAMERA_RESET_GPIO_Port, CAMERA_RESET_Pin, GPIO_PIN_SET);
	osDelay(100);

	// Software reset: reset all registers to default values
	short a = SCCB_Write(REG_COM7, COM7_RESET);
	printf("%d", a);
	osDelay(100);

#ifdef DEBUG
	uint8_t pid;
	uint8_t ver;
	SCCB_Read(REG_PID, &pid);
	SCCB_Read(REG_VER, &ver);
	printf("PID: 0x%x, VER: 0x%x\r\n", pid, ver);
#endif
	
//	OV7670_Configuration(OV7670_INIT);
	
	// Stop DCMI clear buffer
	OV7670_StopDCMI();
	
	return CAMERA_STAT_OK;
}

/**
 * Camera Initialize.
 * @param mode RGB/YUV option.
 */
CAMERA_STAT OV7670_ColorspaceConfiguration(uint32_t mode) {
#ifdef DEBUG
	printf("\r\nStarting initialize \r\n");
#endif
  switch (mode){
		case OV7670_MODE_RGB565:
			OV7670_Configuration(OV7670_RGB565);
			break;
		case OV7670_MODE_YUV:
			OV7670_Configuration(OV7670_YUV);
			break;
		default:
			OV7670_Configuration(OV7670_RGB565);
			break;
  }
//	osDelay(10);
	
#ifdef DEBUG
	printf("Finalize initialize \r\n");
#endif
	return CAMERA_STAT_OK;
}

/**
 * Camera resolution selection.
 * @param opt Resolution option.
 */
CAMERA_STAT OV7670_ResolutionConfiguration(uint16_t img_res) {
#ifdef DEBUG
	printf("\r\nStarting resolution choice \r\n");
#endif
	switch (img_res) {
	case OV7670_RES_QQVGA:
		OV7670_Configuration(OV7670_QQVGA);
		break;
	case OV7670_RES_QVGA:
		OV7670_Configuration(OV7670_QVGA);
		break;
	case OV7670_RES_VGA:
		OV7670_Configuration(OV7670_VGA);
		break;
	default:
		OV7670_Configuration(OV7670_QVGA);
		break;
	}

#ifdef DEBUG
	printf("Finalize configuration \r\n");
#endif
	
	return CAMERA_STAT_OK;
}

/**
 * Start DCMI 
 */
CAMERA_STAT OV7670_StartDCMI(uint32_t capMode, uint16_t img_res, uint32_t destAddress, uint32_t framelength) {
	OV7670_StopDCMI();
	
	if(capMode == OV7670_CAP_CONTINUOUS) {
//		destAddressForContinuousMode = destAddress;
		HAL_DCMI_Start_DMA(phdcmi, DCMI_MODE_CONTINUOUS, destAddress, framelength);
	} 
	else if(capMode == OV7670_CAP_SINGLE_FRAME) {
//		destAddressForContinuousMode = 0;
		HAL_DCMI_Start_DMA(phdcmi, DCMI_MODE_SNAPSHOT, destAddress, framelength);
	}
	
#ifdef DEBUG
	printf("\r\nStart DCMI \r\n");
#endif
	
	return CAMERA_STAT_OK;
}

/**
 * Stop DCMI (Clear memory buffer)
 */
CAMERA_STAT OV7670_StopDCMI(void) {
#ifdef DEBUG
	printf("DCMI has been stopped \r\n");
#endif
	HAL_DCMI_Stop(phdcmi);
	osDelay(10); // If you get a DCMI error (data is not received), increase value to 30.
	
	return CAMERA_STAT_OK;
}

/**
 * Configure camera registers.
 * @param arr Array with addresses and values using to overwrite camera registers.
 */
CAMERA_STAT OV7670_Configuration(const unsigned char arr[][2]) {
	unsigned short i = 0;
	unsigned short j = 0;
	uint8_t reg_addr, data, data_read;
	while (1) {
		reg_addr = arr[i][0];
		data = arr[i][1];
		if (reg_addr == 0xff && data == 0xff) {
#ifdef DEBUG
		printf("Receive 0xff, 0xff \r\n");
#endif
			break;
		}
		SCCB_Read(reg_addr, &data_read);
		SCCB_Write(reg_addr, data);
#ifdef DEBUG
		printf("SCCB write: 0x%x 0x%x=>0x%x\r\n", reg_addr, data_read, data);
#endif
		osDelay(10);
		SCCB_Read(reg_addr, &data_read);
		if (data != data_read) {
#ifdef DEBUG
			printf("SCCB write failure: 0x%x 0x%x\r\n", reg_addr, data_read);
			if (j < 5) {
				j++;
				continue;
			}
			else {
				j = 0;
				printf("SCCB write failure 5 times\r\n");
			}
#endif
		}
		i++;
	}
	
	return CAMERA_STAT_OK;
}

/**
 * Write value to camera register.
 * @param reg_addr Address of register.
 * @param data New value.
 * @return  Operation status.
 */
short SCCB_Write(uint8_t reg_addr, uint8_t data) {
	short operationStatus = 0;
	uint8_t buffer[2] = { 0 };
	HAL_StatusTypeDef connectionStatus;
	buffer[0] = reg_addr;
	buffer[1] = data;
	__disable_irq();
	connectionStatus = HAL_I2C_Master_Transmit(phi2c, (uint16_t) OV7670_I2C_WRITE_ADDR, buffer, 2, 100);
	if (connectionStatus == HAL_OK) {
		operationStatus = 1;
	} else {
		operationStatus = 0;
	}
	__enable_irq();
	return operationStatus;
}

/**
 * Reading data from camera registers.
 * @param reg_addr Address of register.
 * @param pdata Value read from register.
 * @return Operation status.
 */
short SCCB_Read(uint8_t reg_addr, uint8_t *pdata) {
	short operationStatus = 0;
	HAL_StatusTypeDef connectionStatus;
	__disable_irq();
	connectionStatus = HAL_I2C_Master_Transmit(phi2c, (uint16_t) OV7670_I2C_WRITE_ADDR, &reg_addr, 1, 100);
	if (connectionStatus == HAL_OK) {
		connectionStatus = HAL_I2C_Master_Receive(phi2c, (uint16_t) OV7670_I2C_READ_ADDR, pdata, 1, 100);
		if (connectionStatus == HAL_OK) {
			operationStatus = 0;
		} else {
			operationStatus = 1;
		}
	} else {
		operationStatus = 2;
	}
	__enable_irq();
	return operationStatus;
}

///**
// * Continuous mode automatically invokes DCMI, but DMA needs to be invoked manually.
// */
//void OV7670_registerCallback(void (*cbHsync)(uint32_t h), void (*cbVsync)(uint32_t v))
//{
//  s_cbHsync = cbHsync;
//  s_cbVsync = cbVsync;
//}

//void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
//{
////  printf("FRAME %d\n", HAL_GetTick());
//  if(s_cbVsync)s_cbVsync(s_currentV);
//  if(destAddressForContinuousMode != 0) {
//    HAL_DMA_Start_IT(hdcmi->DMA_Handle, (uint32_t)&hdcmi->Instance->DR, destAddressForContinuousMode, QVGA_WIDTH * QVGA_HEIGHT/2);
//  }
//  s_currentV++;
//  s_currentH = 0;
//}
