#ifndef OV7670_H_
#define OV7670_H_

#include "main.h"
#include "stm32f4xx.h"
#include "ov7670_reg.h"
#include "cmsis_os.h"

#define DEBUG

/*
 * Basic window sizes
 */
#define OV7670_MODE_RGB565 0
#define OV7670_MODE_YUV    1

#define OV7670_CAP_CONTINUOUS   0
#define OV7670_CAP_SINGLE_FRAME 1

#define OV7670_RES_QQVGA	0
#define OV7670_RES_QVGA		1
#define OV7670_RES_VGA		2

#define VGA_WIDTH			640
#define VGA_HEIGHT		480
#define QVGA_WIDTH		320
#define QVGA_HEIGHT		240
#define QQVGA_WIDTH		160
#define QQVGA_HEIGHT	120

typedef enum {
	CAMERA_STAT_OK = 0x00,
	CAMERA_STAT_NO_DATA = 0x01,
	CAMERA_STAT_DO_NOTHING = 0x02,
	CAMERA_STAT_ERR = 0x80,
	CAMERA_STAT_ERR_TIMEOUT = 0x81
} CAMERA_STAT;

CAMERA_STAT OV7670_Init(I2C_HandleTypeDef *p_hi2c, DCMI_HandleTypeDef *p_hdcmi);
CAMERA_STAT OV7670_ColorspaceConfiguration(uint32_t mode);
CAMERA_STAT OV7670_ResolutionConfiguration(uint16_t img_res);
CAMERA_STAT OV7670_StartDCMI(uint32_t capMode, uint16_t img_res, uint32_t destAddress, uint32_t framebuffer);
CAMERA_STAT OV7670_StopDCMI(void);
CAMERA_STAT OV7670_Configuration(const unsigned char arr[][2]);
short SCCB_Write(uint8_t reg_addr, uint8_t data);
short SCCB_Read(uint8_t reg_addr, uint8_t *pdata);
void OV7670_registerCallback(void (*cbHsync)(uint32_t h), void (*cbVsync)(uint32_t v));


#endif /* OV7670_H_ */
