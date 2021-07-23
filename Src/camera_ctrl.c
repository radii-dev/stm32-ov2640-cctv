#include <stdio.h>
#include "main.h"
#include "ov7670.h"
#include "camera_ctrl.h"

extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
extern DCMI_HandleTypeDef hdcmi;
extern DMA_HandleTypeDef hdma_dcmi;
extern I2C_HandleTypeDef hi2c1;

void Camera_UserInit(Camera *cam) {
	cam->mode = CAMERA_MODE_RGB565;
	cam->resolution = CAMERA_RES_QVGA;
	cam->capmode = CAMERA_CAP_CONTINUOUS;
	cam->framebuffer.uint = 0;
}

void Camera_UserProcess(void) {
	Camera cam;
	Camera_UserInit(&cam);
	
	camera_init();
//	if (camera_config(&cam) == CAMERA_STAT_OK)
//		camera_start(&cam);
//	HAL_UART_Transmit_DMA(&huart3, cam.framebuffer.ch, cam.framelength);
	while(1) { 
//		printf("%s\r\n", cam.framebuffer.ch);
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
		HAL_Delay(1000);
	}
}

CAMERA_STAT camera_init(void) {
	return OV7670_Init(&hi2c1, &hdcmi);
}


CAMERA_STAT camera_config(Camera *c) {
	if (c->mode > 1)	{
		printf("camera mode %d is not supported\n", c->mode);
		return CAMERA_STAT_ERR;
	}
	if (c->resolution > 2)	{
		printf("camera res %d is not supported\n", c->resolution);
		return CAMERA_STAT_ERR;
	}
	
  switch (c->resolution){
		case CAMERA_RES_QQVGA:
			c->res_width = QQVGA_WIDTH;
			c->res_height = QQVGA_HEIGHT;
			c->framelength = QQVGA_WIDTH * QQVGA_HEIGHT / 2;
			break;
		case CAMERA_RES_QVGA:
			c->res_width = QVGA_WIDTH;
			c->res_height = QVGA_HEIGHT;
			c->framelength = QVGA_WIDTH * QVGA_HEIGHT / 2;
			break;
		case CAMERA_RES_VGA:
			c->res_width = VGA_WIDTH;
			c->res_height = VGA_HEIGHT;
			c->framelength = VGA_WIDTH * VGA_HEIGHT / 2;
			break;
  }
	
	if (OV7670_ColorspaceConfiguration(c->mode) == CAMERA_STAT_OK && OV7670_ResolutionConfiguration(c->resolution) == CAMERA_STAT_OK)
		return CAMERA_STAT_OK;
	else 
		return CAMERA_STAT_ERR;
}

CAMERA_STAT camera_start(Camera *c) {
	if (c->capmode > 1) {
			printf("cap mode %d is not supported\n", c->capmode);
			return CAMERA_STAT_ERR;
	}
	
	return OV7670_StartDCMI(c->capmode, c->resolution, (uint32_t)&c->framebuffer.uint, c->framelength);
}

CAMERA_STAT camera_stop(void) {
	return OV7670_StopDCMI();
}

//void camera_registerCallback(void (*cbHsync)(uint32_t h), void (*cbVsync)(uint32_t v))
//{
//  OV7670_registerCallback(cbHsync, cbVsync);
//}
