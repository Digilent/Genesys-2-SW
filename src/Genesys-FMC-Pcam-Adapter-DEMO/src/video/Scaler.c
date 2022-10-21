#include "xvideo_scaler.h"
#include "xstatus.h"
#include "Scaler.h"

	int scaler_init(XVideo_scaler *drv_inst_, uint16_t dev_id) // Must initialize with 0, otherwise driver craps itself
	{
		XVideo_scaler_Config * ConfigPtr = XVideo_scaler_LookupConfig(dev_id);
		if(ConfigPtr == NULL)
		{
			return XST_FAILURE;
		}
		XStatus status = XVideo_scaler_CfgInitialize(drv_inst_,
										  ConfigPtr);

		if(status != XST_SUCCESS)
		{
			return XST_FAILURE;
		}

		return XST_SUCCESS;
	}

	void scaler_setStreams(XVideo_scaler *drv_inst_, u32 WidthIn, u32 HeightIn, u32 WidthOut, u32 HeightOut)
	{
		{
			XVideo_scaler_Set_in_width(drv_inst_, WidthIn);
			XVideo_scaler_Set_in_height(drv_inst_, HeightIn);
		}
		{
			XVideo_scaler_Set_out_width(drv_inst_, WidthOut);
			XVideo_scaler_Set_out_height(drv_inst_, HeightOut);
		}
	}
	void scaler_enable(XVideo_scaler *drv_inst_)
	{
		XVideo_scaler_EnableAutoRestart(drv_inst_);
		XVideo_scaler_Start(drv_inst_);
	}
