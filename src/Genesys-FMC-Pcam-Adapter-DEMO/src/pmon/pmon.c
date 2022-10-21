#include "pmon.h"

#include "xaxipmon.h"
#include "xil_exception.h"
#include "xstatus.h"
#include "stdio.h"

double read_bandwidth=0,write_bandwidth=0;
#define PERFMON XPAR_AXI_PERF_MON_0_BASEADDR
int error=0;

void axi_perf_mon_config()
{

#define XAPM_METRIC_SET_0 0

	// Configure Perf Mon
	// Interval Load Register
//	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x24, 0x05F5E100);
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x24, 200000000);

	// Interval counter load
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x28, 0x2); //Load
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x28, 0x0); // Deactivate load
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x28, 0x101); // Enable the counter
	//Metric Selector Reg
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x44, 0x22230203);
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x48, 0x62634243);


	// reset
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x300, 0x00000202);

	//Enable counters
	Xil_Out32(XPAR_AXIPMON_0_BASEADDR+0x300, 0x00000101);
}

void perfmon_main(){

	char string_res[16][20] = { 	"640x480 @ 60 Hz" 	,"720x480 @ 60 Hz" 		,"800x600 @ 60 Hz" ,"1024x768 @ 60 Hz" ,
			   	   	   	   	    	"1280x720 @ 60 Hz"  ,"1280x1024 @ 60 Hz" 	,"1920x1080 @ 60 Hz" ,"1600x1200 @ 60 Hz" ,
			   	   	   	   	    	"640x480 @ 75 Hz" 	,"720x480 @ 75 Hz" 		,"800x600 @ 75 Hz" ,"1024x768 @ 75 Hz" ,
			   	   	   	   	    	"1280x720 @ 75 Hz"	,"1280x1024 @ 75 Hz" 	,"1920x1080 @ 75 Hz" ,"1600x1200 @ 75 Hz"    };

	int resolution = 6;
	int frame_rate = 0;
	double Arith_value_frac, TPut;
	unsigned long long Arith_value, h8_llu, h7_llu;
	unsigned int metric_count_7h;
	unsigned int metric_count_7l;
	unsigned int metric_count_8h;
	unsigned int metric_count_8l;
	unsigned int global_clk_count_h;
	unsigned int global_clk_count_l;

	global_clk_count_l = 0;
	global_clk_count_h = 0;



	*((volatile int*)(PERFMON+0x38))=0xFFF;
	*((volatile int*)(PERFMON+0x300))=0x0020002;
	*((volatile int*)(PERFMON+0x44))=0x03020100;
	*((volatile int*)(PERFMON+0x30))=0x00000001;
	*((volatile int*)(PERFMON+0x34))=0x00000002;
//	*((volatile int*)(PERFMON+0x24))=0x05F5E100;   // 200M, 500ms
	*((volatile int*)(PERFMON+0x24))=200000000;
	*((volatile int*)(PERFMON+0x28))=0x00000002;
	*((volatile int*)(PERFMON+0x28))=0x00000001;

	xil_printf("START\n\r");
	*((volatile int*)(PERFMON+0x300))=0x0020002;
	*((volatile int*)(PERFMON+0x300))=0x00010001;
	while((*((volatile int*)(PERFMON+0x38))& 0x00000002)!=0x00000002){
	}
	*((volatile int*)(PERFMON+0x28))=0x00000000;
	*((volatile int*)(PERFMON+0x300))=0x00000000;
	for(int i=0;i<1000;i++);

	metric_count_7h = 0;
	metric_count_8h = 0;
	metric_count_7l = *(volatile int *)(PERFMON+0x220) ;
	metric_count_8l = *(volatile int *)(PERFMON+0x230) ;

	global_clk_count_l = *(volatile int *) (PERFMON+0x0004);
	global_clk_count_h = *(volatile int *) (PERFMON+0x0000);
	xil_printf("--GlblCnt : %x%x , Slot1 Wr Byte %x , Rd Byte:%x--\n\r",
			global_clk_count_h, global_clk_count_l,metric_count_7l,metric_count_8l
	);

	xil_printf("\r\n ---------DDR3, AXI4 Slave Profile Summary........\n");
	xil_printf("\r\nTheoretical DDR Bandwidth                                    				= 12800 Mbytes/sec\n\r");

	h7_llu = metric_count_7h ;
	h8_llu = metric_count_8h ;
	Arith_value = ((metric_count_7l) + (h7_llu << 32) + (metric_count_8l) + (h8_llu << 32)) ;
	xil_printf("\r\nPractical DDR bandwidth                                          			=  %d Mbytes/sec\n\r", (unsigned int) (((metric_count_7l/1000000)+(metric_count_8l/1000000))<<1));

	TPut=(double)(Arith_value*200000000)/(double)global_clk_count_l ;
	if(TPut < 8957952000  )
	{
		xil_printf("\n\rTest Failed \n\r");
		xil_printf("\n\rExpected Tput :8958 Mbytes/s  Achieved Tput : %d Mbytes/s \n\r", (unsigned int)( TPut/1000000));
		error++;
	}

	Arith_value_frac = (double)(Arith_value*2) /(double)(128000000) ;
	unsigned int whole,fract;
	whole= Arith_value_frac;
	fract= (Arith_value_frac-whole)*100;


	xil_printf("\r\nPercentage of DDDR Bandwidth consumed by frame of resolution (%s)(Approx.)= %d.%02d  \n\r", string_res[frame_rate*8 + resolution], whole,fract);

	// reset
	global_clk_count_l = 0;
	global_clk_count_h = 0;
	*((volatile int*)(PERFMON+0x300))=0x0020002;

}
