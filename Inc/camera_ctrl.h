#ifndef CAMERA_CTRL_H_
#define CAMERA_CTRL_H_

#include "ov7670.h"


#define CAMERA_MODE_RGB565 0
#define CAMERA_MODE_YUV    1

#define CAMERA_CAP_CONTINUOUS   0
#define CAMERA_CAP_SINGLE_FRAME 1

#define CAMERA_RES_QQVGA	0
#define CAMERA_RES_QVGA		1
#define CAMERA_RES_VGA		2

typedef union buffer {
	uint8_t ch[4];
	uint32_t uint;
} buffer_t;

typedef struct _Camera {
	uint8_t mode;
	uint8_t resolution;
	uint8_t capmode;
	uint16_t res_width;
	uint16_t res_height;
	uint32_t framelength;
	buffer_t framebuffer;
} Camera;

void Camera_UserInit(Camera *cam);
void Camera_UserProcess(void);
CAMERA_STAT camera_init(void);
CAMERA_STAT camera_config(Camera *c);
CAMERA_STAT camera_start(Camera *c);
CAMERA_STAT camera_stop(void);
void camera_registerCallback(void (*cbHsync)(uint32_t h), void (*cbVsync)(uint32_t v));


#endif /* CAMERA_CTRL_H_ */
