/*
 * Scaler2.h
 *
 *  Created on: Dec 12, 2018
 *      Author: ROGyorgE
 */

#ifndef SRC_CAM_SCALER_H_
#define SRC_CAM_SCALER_H_

#include "xvideo_scaler.h"
#include "xstatus.h"

int scaler_init(XVideo_scaler *drv_inst_, uint16_t dev_id);
void scaler_setStreams(XVideo_scaler *drv_inst_, u32 WidthIn, u32 HeightIn, u32 WidthOut, u32 HeightOut);
void scaler_enable(XVideo_scaler *drv_inst_);




#endif /* SRC_CAM_SCALER_H_ */
