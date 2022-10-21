/*
 * VideoSource.h
 *
 *  Created on: Aug 30, 2016
 *      Author: Elod
 */

#ifndef VIDEOOUTPUT_H_
#define VIDEOOUTPUT_H_

#include "xvtc.h"
#include "xclk_wiz.h"

typedef enum
{
	R1920_1080_60_PP = 0,
	R1280_720_60_PP,
	R960_540_60_PP,
	R640_480_60_NN
} Resolution;

typedef enum {NEG=0, POS=1}Polarity;

typedef struct
{
	Resolution res;
	uint16_t h_active, h_fp, h_sync, h_bp;
	Polarity h_pol;
	uint16_t v_active, v_fp, v_sync, v_bp;
	Polarity v_pol;
	uint32_t pclk_freq_Hz;

} timing_t;

extern timing_t const timing[];

int vtc_init(XVtc *vtc_inst, u32 VTC_dev_id, XClk_Wiz *clk_wiz_inst, u32 clkwiz_dev_id);
void vtc_reset(XVtc *vtc_inst);
void vtc_configure(XVtc *vtc_inst, XClk_Wiz *clk_wiz_inst, Resolution res);
void vtc_enable(XVtc *vtc_inst);

#endif /* VIDEOOUTPUT_H_ */
