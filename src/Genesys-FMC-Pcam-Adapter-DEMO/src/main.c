/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "xil_printf.h"
#include "sleep.h"
#include "xil_cache.h"

#include "cam/IIC.h"
#include "cam/GPIO.h"
#include "intc/XIntcInterruptController.h"
#include "cam/TCA9546.h"
#include "cam/OV5640.h"
#include "verbose/verbose.h"
#include "video/AXI_VDMA.h"
#include "video/Scaler.h"
#include "video/VideoOutput.h"
#include "MIPI_D_PHY_RX.h"
#include "MIPI_CSI_2_RX.h"
#include "platform/platform.h"
#include "pmon/pmon.h"


/* Hardware profile */
#define IRPT_CTL_DEVID 			XPAR_INTC_0_DEVICE_ID
#define VDMA_A_DEVID			XPAR_AXI_VDMA_A_DEVICE_ID
#define VDMA_A_MM2S_IRPT_ID		XPAR_INTC_0_AXIVDMA_0_MM2S_INTROUT_VEC_ID
#define VDMA_A_S2MM_IRPT_ID		XPAR_INTC_0_AXIVDMA_0_S2MM_INTROUT_VEC_ID
#define SCALER_A_DEVID			XPAR_VIDEO_SCALER_A_DEVICE_ID
#define GPIO_DEVID				XPAR_AXI_PWUP_PRSNT_GPIO_DEVICE_ID
#define GPIO_IRPT_ID			XPAR_MICROBLAZE_0_AXI_INTC_AXI_PWUP_PRSNT_GPIO_IP2INTC_IRPT_INTR
#define CAM_I2C_DEVID			XPAR_IIC_0_DEVICE_ID
#define CAM_I2C_IRPT_ID			XPAR_INTC_0_IIC_0_VEC_ID
#define UARTLITE_IRPT_ID		XPAR_INTC_0_UARTLITE_0_VEC_ID

#define VTC_DEVID				XPAR_VTC_0_DEVICE_ID
#define DYN_PIXCLK_DEVID		XPAR_VIDEO_DYNCLK_DEVICE_ID

#define VDMA_B_DEVID			XPAR_AXI_VDMA_B_DEVICE_ID
#define VDMA_B_S2MM_IRPT_ID		XPAR_MICROBLAZE_0_AXI_INTC_AXI_VDMA_B_S2MM_INTROUT_INTR
#define SCALER_B_DEVID			XPAR_VIDEO_SCALER_B_DEVICE_ID

#define VDMA_C_DEVID			XPAR_AXI_VDMA_C_DEVICE_ID
#define VDMA_C_S2MM_IRPT_ID		XPAR_MICROBLAZE_0_AXI_INTC_AXI_VDMA_C_S2MM_INTROUT_INTR
#define SCALER_C_DEVID			XPAR_VIDEO_SCALER_C_DEVICE_ID

#define VDMA_D_DEVID			XPAR_AXI_VDMA_D_DEVICE_ID
#define VDMA_D_S2MM_IRPT_ID		XPAR_MICROBLAZE_0_AXI_INTC_AXI_VDMA_D_S2MM_INTROUT_INTR
#define SCALER_D_DEVID			XPAR_VIDEO_SCALER_D_DEVICE_ID

#define CAM_CHANNEL_A 0
#define CAM_CHANNEL_B 1
#define CAM_CHANNEL_C 2
#define CAM_CHANNEL_D 3

#define BYTES_PER_PIXEL 		(XPAR_AXI_VDMA_A_S_AXIS_S2MM_TDATA_WIDTH / 8)
#define NUM_FRAMESTORES 		XPAR_AXI_VDMA_A_NUM_FSTORES

double VDMA_0_READ_VAL;
double VDMA_0_WRITE_VAL;

double VDMA_1_READ_VAL;
double VDMA_1_WRITE_VAL;

double VDMA_2_READ_VAL;
double VDMA_2_WRITE_VAL;

double VDMA_3_READ_VAL;
double VDMA_3_WRITE_VAL;

XIic IicInstance;
XGpio GpioInstance;
XIntc IntcInstance;


XAxiVdma Vdma_a;
XAxiVdma Vdma_b;
XAxiVdma Vdma_c;
XAxiVdma Vdma_d;

XVideo_scaler scaler_a;
XVideo_scaler scaler_b;
XVideo_scaler scaler_c;
XVideo_scaler scaler_d;

XVtc sVtc_;
XClk_Wiz sClkWiz_;

/* *************** */
uintptr_t frame_baseaddr 	= XPAR_MIG7SERIES_0_BASEADDR + 0x0A000000U; //Must make sure linker reserves the region above this
uintptr_t const portb_offset 		= 1920/2*BYTES_PER_PIXEL;
uintptr_t const portc_offset 		= 1920*1080/2*BYTES_PER_PIXEL;
uintptr_t const portd_offset 		= (1920*1080/2+1920/2)*BYTES_PER_PIXEL;
/* *************** */

static int input_pipeline_mode_change(uint8_t channel_mask, XAxiVdma *vdma_driver, XVideo_scaler *scaler, uint32_t frame_buf_base_addr_, Resolution HW_ScaledCaptureRes, Resolution VideoOutputRes, mode_t mode, uintptr_t dphy_baseaddr, uintptr_t csi2_baseaddr, uintptr_t gamma_baseaddr, u8 master_select, u8 is_master);
static void output_pipeline_mode_change(XAxiVdma *vdma_driver, uint32_t frame_buf_base_addr_, Resolution VideoOutputRes, u8 master_select);
static void perf_cnt();

int main() {
	//Init CPU, UART, caches etc.
    init_platform();

    double total_bandwidth;
    double total_mem_bandwidth;
    double percent_of_bandwidth;

    total_mem_bandwidth = (((800*32)/8)*2)*2;

    u8 master_select;
    XStatus status_a, status_b, status_c, status_d;
    //Blank VDMA frame buffers
    memset((u8*)frame_baseaddr, 0x55, (1920*1080*BYTES_PER_PIXEL)*NUM_FRAMESTORES);
    //Flush D-Cache because this is DMA-accessible memory
    Xil_DCacheFlushRange(frame_baseaddr, (1920*1080*BYTES_PER_PIXEL)*NUM_FRAMESTORES);

#ifdef _DEBUG
	SET_VERBOSE_FLAG();
#endif

	VERBOSE("Initializing...");

    iic_init(&IicInstance, CAM_I2C_DEVID);
    gpio_init(&GpioInstance, GPIO_DEVID);
    intc_init(&IntcInstance, IRPT_CTL_DEVID,
    		&IicInstance, CAM_I2C_IRPT_ID,
			&GpioInstance, GPIO_IRPT_ID,
			&Vdma_a, VDMA_A_MM2S_IRPT_ID, VDMA_A_S2MM_IRPT_ID,
			&Vdma_b, VDMA_B_S2MM_IRPT_ID,
			&Vdma_c, VDMA_C_S2MM_IRPT_ID,
			&Vdma_d, VDMA_D_S2MM_IRPT_ID);


    //Dual-channel VDMA for the display and the first camera
    vdma_init(&Vdma_a, VDMA_A_DEVID);

    //Output pipeline drivers
    vtc_init(&sVtc_, VTC_DEVID, &sClkWiz_, DYN_PIXCLK_DEVID);

    //Input pipeline drivers
    scaler_init(&scaler_a, SCALER_A_DEVID);

    //Initialize input pipelines, depending on how many D-PHY IPs are in hardware
#if XPAR_MIPI_D_PHY_RX_NUM_INSTANCES >= 2
    vdma_init(&Vdma_b, VDMA_B_DEVID);
    scaler_init(&scaler_b, SCALER_B_DEVID);
#endif

#if XPAR_MIPI_D_PHY_RX_NUM_INSTANCES >= 3
    vdma_init(&Vdma_c, VDMA_C_DEVID);
    scaler_init(&scaler_c, SCALER_C_DEVID);
#endif

#if XPAR_MIPI_D_PHY_RX_NUM_INSTANCES >= 4
    vdma_init(&Vdma_d, VDMA_D_DEVID);
    scaler_init(&scaler_d, SCALER_D_DEVID);
#endif

    //Init of system board finished
    VERBOSE("FMC Pcam Adapter Rev.demo \r\n");
    gpio_clearBit(&GpioInstance, CAM_GPIO0);
    usleep(1000000);
    gpio_setBit(&GpioInstance, CAM_GPIO0);
    usleep(1000000);

    mux_reset(&IicInstance, 0);
    ov5640_reset(&GpioInstance);

    master_select = 0;//there must always be a Pcam in the FMC Pcam connector A

    status_a = input_pipeline_mode_change(CAM_CHANNEL_A, &Vdma_a, &scaler_a, frame_baseaddr, R960_540_60_PP, R1920_1080_60_PP, MODE_1080P_1920_1080_30fps, XPAR_MIPI_D_PHY_RX_A_S_AXI_LITE_BASEADDR, XPAR_MIPI_CSI_2_RX_A_S_AXI_LITE_BASEADDR, XPAR_AXI_GAMMACORRECTION_A_BASEADDR, master_select, master_select == 0);
    status_b = input_pipeline_mode_change(CAM_CHANNEL_B, &Vdma_b, &scaler_b, frame_baseaddr + portb_offset, R960_540_60_PP, R1920_1080_60_PP, MODE_1080P_1920_1080_30fps, XPAR_MIPI_D_PHY_RX_B_S_AXI_LITE_BASEADDR, XPAR_MIPI_CSI_2_RX_B_S_AXI_LITE_BASEADDR, XPAR_AXI_GAMMACORRECTION_B_BASEADDR, master_select, master_select == 1);
    status_c = input_pipeline_mode_change(CAM_CHANNEL_C, &Vdma_c, &scaler_c, frame_baseaddr + portc_offset, R960_540_60_PP, R1920_1080_60_PP, MODE_1080P_1920_1080_30fps, XPAR_MIPI_D_PHY_RX_C_S_AXI_LITE_BASEADDR, XPAR_MIPI_CSI_2_RX_C_S_AXI_LITE_BASEADDR, XPAR_AXI_GAMMACORRECTION_C_BASEADDR, master_select, master_select == 2);
    status_d = input_pipeline_mode_change(CAM_CHANNEL_D, &Vdma_d, &scaler_d, frame_baseaddr + portd_offset, R960_540_60_PP, R1920_1080_60_PP, MODE_1080P_1920_1080_30fps, XPAR_MIPI_D_PHY_RX_D_S_AXI_LITE_BASEADDR, XPAR_MIPI_CSI_2_RX_D_S_AXI_LITE_BASEADDR, XPAR_AXI_GAMMACORRECTION_D_BASEADDR, master_select, master_select == 3);

    output_pipeline_mode_change(&Vdma_a, frame_baseaddr, R1920_1080_60_PP, master_select);

    axi_perf_mon_config();

    //Init global vars to 0
    VDMA_0_READ_VAL = 0;
    VDMA_0_WRITE_VAL = 0;

    VDMA_1_READ_VAL = 0;
    VDMA_1_WRITE_VAL = 0;

    VDMA_2_READ_VAL = 0;
    VDMA_2_WRITE_VAL = 0;

    VDMA_3_READ_VAL = 0;
    VDMA_3_WRITE_VAL = 0;

    perf_cnt();
    xil_printf("\r\n");

    total_bandwidth = VDMA_0_READ_VAL + VDMA_0_WRITE_VAL + VDMA_1_READ_VAL + VDMA_1_WRITE_VAL + VDMA_2_READ_VAL + VDMA_2_WRITE_VAL + VDMA_3_READ_VAL + VDMA_3_WRITE_VAL;
    total_bandwidth = total_bandwidth;

    percent_of_bandwidth = (total_bandwidth/total_mem_bandwidth) * 100;

    printf("System Total Bandwidth = %f MB/s\r\n",total_bandwidth);
    printf("DDR3 Theoretical Bandwidth = %f MB/s\r\n",total_mem_bandwidth);
    printf("Percent of DDR3 Theoretical Bandwidth = %f%%\r\n",percent_of_bandwidth);



    cleanup_platform();
    return 0;
}

// Global function that sets the camera sensor to live image


int input_pipeline_mode_change(uint8_t channel_mask, XAxiVdma *vdma_driver, XVideo_scaler *scaler, uint32_t frame_buf_base_addr_, Resolution HW_ScaledCaptureRes, Resolution VideoOutputRes, mode_t mode, uintptr_t dphy_baseaddr, uintptr_t csi2_baseaddr, uintptr_t gamma_baseaddr, u8 master_select, u8 is_master)
{
	//Bring up input pipeline back-to-front
	{
		resetWrite(vdma_driver);
		MIPI_CSI_2_RX_mWriteReg(csi2_baseaddr, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
		MIPI_D_PHY_RX_mWriteReg(dphy_baseaddr, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
		//cam.reset(); //TODO cams sharing same power enable
	}

	{
		configureWrite(vdma_driver, frame_buf_base_addr_, timing[HW_ScaledCaptureRes].h_active, timing[HW_ScaledCaptureRes].v_active, timing[VideoOutputRes].h_active, timing[VideoOutputRes].v_active, master_select, is_master);
		Xil_Out32(gamma_baseaddr, 3); // Set Gamma correction factor to 1/1.8
		scaler_setStreams(scaler, 1920, 1080, 960, 540);
		//TODO CSI-2, D-PHY config here
		XStatus Status = ov5640_init(&IicInstance, (1<<channel_mask));
		if (Status != XST_SUCCESS)
		{
			return XST_FAILURE;
		}
	}

	{
		enableWrite(vdma_driver);
		scaler_enable(scaler);
		MIPI_CSI_2_RX_mWriteReg(csi2_baseaddr, CR_OFFSET, CR_ENABLE_MASK);
		MIPI_D_PHY_RX_mWriteReg(dphy_baseaddr, CR_OFFSET, CR_ENABLE_MASK);
		set_mode(&IicInstance, mode, (1<<channel_mask));
		set_awb(&IicInstance, AWB_ADVANCED, (1<<channel_mask));
	}

	return XST_SUCCESS;
}

void output_pipeline_mode_change(XAxiVdma *vdma_driver, uint32_t frame_buf_base_addr_, Resolution VideoOutputRes, u8 master_select)
{

	//Bring up output pipeline back-to-front
	{
		vtc_reset(&sVtc_);
		resetRead(vdma_driver);
	}

	{
		vtc_configure(&sVtc_, &sClkWiz_, VideoOutputRes);
		configureRead(vdma_driver, frame_buf_base_addr_, timing[VideoOutputRes].h_active, timing[VideoOutputRes].v_active, master_select);
	}

	{
		vtc_enable(&sVtc_);
		enableRead(vdma_driver);
	}
}

void perf_cnt()
{
	double HP0_read;
	double HP0_write;
	double HP0_read_dec;
	double HP0_write_dec;

	double HP1_read;
	double HP1_write;
	double HP1_read_dec;
	double HP1_write_dec;

	double HP2_read;
	double HP2_write;
	double HP2_read_dec;
	double HP2_write_dec;

	double HP3_read;
	double HP3_write;
	double HP3_read_dec;
	double HP3_write_dec;


	int intr = 0;
	// reset
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x300, 0x00000202);

	//Enable counters
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x300, 0x00000101);

	// Interval counter load
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x28, 0x2); //Load
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x28, 0x0); // Deactivate load
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x28, 0x101); // Enable the counter

	intr = Xil_In32(XPAR_AXIPMON_0_BASEADDR+0x0038); // Read Intr Status Reg
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x0038, intr);//Clear Intr status


	intr = 0;
	while ((intr & 0x00000002) == 0)
	{
		intr = Xil_In32(XPAR_AXIPMON_0_BASEADDR+0x0038); // Read Intr Status Reg

	}
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x0038, intr);//Clear Intr status

	//Obtain counters for each AXI monitor interface
	//And Compute bandwidth in MB/s.
	HP0_read = Xil_In32(XPAR_AXIPMON_0_BASEADDR+0x200);  // Read Byte Count
	HP0_write = Xil_In32(XPAR_AXIPMON_0_BASEADDR+0x210);  // Write Byte Count

	HP1_read = Xil_In32(XPAR_AXIPMON_0_BASEADDR+0x220);
	HP1_write = Xil_In32(XPAR_AXIPMON_0_BASEADDR+0x230);

	HP2_read = Xil_In32(XPAR_AXIPMON_0_BASEADDR+0x240);
	HP2_write = Xil_In32(XPAR_AXIPMON_0_BASEADDR+0x250);

	HP3_read = Xil_In32(XPAR_AXIPMON_0_BASEADDR+0x260);
	HP3_write = Xil_In32(XPAR_AXIPMON_0_BASEADDR+0x270);


	HP0_read_dec = HP0_read*.000001;
	HP0_write_dec = HP0_write*.000001;

	HP1_read_dec = HP1_read*.000001;
	HP1_write_dec = HP1_write*.000001;

	HP2_read_dec = HP2_read*.000001;
	HP2_write_dec = HP2_write*.000001;

	HP3_read_dec = HP3_read*.000001;
	HP3_write_dec = HP3_write*.000001;

	VDMA_0_READ_VAL = HP0_read_dec;
	VDMA_0_WRITE_VAL = HP0_write_dec;

	VDMA_1_READ_VAL = HP1_read_dec;
	VDMA_1_WRITE_VAL = HP1_write_dec;

	VDMA_2_READ_VAL = HP2_read_dec;
	VDMA_2_WRITE_VAL = HP2_write_dec;

	VDMA_3_READ_VAL = HP3_read_dec;
	VDMA_3_WRITE_VAL = HP3_write_dec;

	printf("\r\n");

	printf("AXI_VDMA1 Tx = %f \r\n", HP0_read_dec);
	printf("AXI_VDMA1 Rx = %f \r\n", HP0_write_dec);

	printf("AXI_VDMA2 Tx = %f \r\n", HP1_read_dec);
	printf("AXI_VDMA2 Rx = %f \r\n", HP1_write_dec);

	printf("AXI_VDMA3 Tx = %f \r\n", HP2_read_dec);
	printf("AXI_VDMA3 Rx = %f \r\n", HP2_write_dec);

	printf("AXI_VDMA4 Tx = %f \r\n", HP3_read_dec);
	printf("AXI_VDMA4 Rx = %f \r\n", HP3_write_dec);

}
