#ifndef OV5640_H_
#define OV5640_H_

#include "xiic.h"

typedef enum {OK=0, ERR_LOGICAL, ERR_GENERAL} Errc;

typedef struct { uint16_t addr; uint8_t data; }config_word_t;

typedef enum { MODE_720P_1280_720_60fps = 0, MODE_1080P_1920_1080_15fps,
	MODE_1080P_1920_1080_30fps, MODE_1080P_1920_1080_30fps_336M_MIPI,
	MODE_1080P_1920_1080_30fps_336M_1LANE_MIPI, MODE_END
}cam_mode_t;

typedef struct { cam_mode_t mode; config_word_t const* cfg; size_t cfg_size; }config_modes_t;
typedef enum { TEST_DISABLED = 0, TEST_EIGHT_COLOR_BAR, TEST_END }test_t;
typedef enum { AWB_DISABLED = 0, AWB_SIMPLE, AWB_ADVANCED, AWB_END }awb_t;
typedef struct { awb_t awb; config_word_t const* cfg; size_t cfg_size; }config_awb_t;
typedef enum { ISP_RAW = 0, ISP_RGB, ISP_END }isp_format_t;

/************************** Function Prototypes ******************************/
int ov5640_init(XIic *IicInstance, uint8_t channel_mask_);
Errc ov5640_reset(XGpio *GpioInstance);
Errc set_mode(XIic *IicInstance, cam_mode_t mode, uint8_t channel_mask_);
Errc set_awb(XIic *IicInstance, awb_t awb, uint8_t channel_mask_);
Errc set_isp_format(XIic *IicInstance, isp_format_t isp, uint8_t channel_mask_);
void set_test(XIic *IicInstance, test_t test, uint8_t channel_mask_);
void readReg(XIic *IicInstance, uint16_t reg_addr, uint8_t *buf, uint8_t channel_mask_);
void writeReg(XIic *IicInstance, uint16_t reg_addr, uint8_t const reg_data, uint8_t channel_mask_);
void writeRegLiquid(XIic *IicInstance, uint8_t const reg_data, uint8_t channel_mask_);
void writeConfig(XIic *IicInstance, config_word_t const* cfg, size_t cfg_size, uint8_t channel_mask_);

#endif /* OV5640_H_ */
