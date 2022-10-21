#include "xaxivdma.h"
#include "xstatus.h"
#include "AXI_VDMA.h"

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)


/*!
 * \brief Driver class for Xilinx AXI VDMA IP. Needs to have stable clocks before
 * instantiation to be able to complete hardware reset.
 */
	typedef struct
	{
		/* The state variable to keep track if the initialization is done*/
		unsigned int init_done;

		/* The XAxiVdma_DmaSetup structure contains all the necessary information to
		 * start a frame write or read. */
		XAxiVdma_DmaSetup ReadCfg;
		XAxiVdma_DmaSetup WriteCfg;
		/* Horizontal size of frame */
		unsigned int hsize;
		/* Vertical size of frame */
		unsigned int vsize;
		/* Buffer address from where read and write will be done by VDMA */
		unsigned int buffer_address;
		/* Flag to tell VDMA to interrupt on frame completion*/
		unsigned int enable_frm_cnt_intr;
		/* The counter to tell VDMA on how many frames the interrupt should happen*/
		unsigned int number_of_frame_count;
	} vdma_context_t;

vdma_context_t context_;
int const RESET_POLL = 1000;

	int vdma_init(XAxiVdma *drv_inst_, uint16_t dev_id){

		XAxiVdma_Config* psConf;
		XStatus Status;

		psConf = XAxiVdma_LookupConfig(dev_id);
		if (!psConf) {
			return XST_FAILURE;
		}

		//Initialize driver instance and reset VDMA
		Status = XAxiVdma_CfgInitialize(drv_inst_, psConf, psConf->BaseAddress);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		//Set error interrupt error handlers, which for some reason need completion handler defined too
		if (psConf->HasMm2S)
		{

			XAxiVdma_SetCallBack(drv_inst_, XAXIVDMA_HANDLER_GENERAL,
					readHandler, (void *)drv_inst_, XAXIVDMA_READ);

			XAxiVdma_SetCallBack(drv_inst_, XAXIVDMA_HANDLER_ERROR,
					readErrorHandler, (void *)drv_inst_, XAXIVDMA_READ);

		}
		if (psConf->HasS2Mm)
		{

			XAxiVdma_SetCallBack(drv_inst_, XAXIVDMA_HANDLER_GENERAL,
					writeHandler, (void *)drv_inst_, XAXIVDMA_WRITE);

			XAxiVdma_SetCallBack(drv_inst_, XAXIVDMA_HANDLER_ERROR,
					writeErrorHandler, (void *)drv_inst_, XAXIVDMA_WRITE);
		}

		return XST_SUCCESS;

	}

	int resetRead(XAxiVdma *drv_inst_)
	{
		XAxiVdma_ChannelReset(&drv_inst_->ReadChannel);

		int Polls = RESET_POLL;

		while (Polls && XAxiVdma_ChannelResetNotDone(&drv_inst_->ReadChannel)) {
			--Polls;
		}

		if (!Polls) {
			return XST_FAILURE;
		}

		return XST_SUCCESS;
	}

	int resetWrite(XAxiVdma *drv_inst_)
	{
		XAxiVdma_ChannelReset(&drv_inst_->WriteChannel);

		int Polls = RESET_POLL;

		while (Polls && XAxiVdma_ChannelResetNotDone(&drv_inst_->WriteChannel)) {
			--Polls;
		}

		if (!Polls) {
			return XST_FAILURE;
		}

		return XST_SUCCESS;
	}

	int configureRead(XAxiVdma *drv_inst_, uint32_t frame_buf_base_addr_, uint16_t h_res, uint16_t v_res, u8 master_select)
	{
		XStatus status;
		context_.ReadCfg.HoriSizeInput = h_res * drv_inst_->ReadChannel.StreamWidth;
		context_.ReadCfg.VertSizeInput = v_res;
		context_.ReadCfg.Stride = context_.ReadCfg.HoriSizeInput;
		context_.ReadCfg.FrameDelay = 2; // Frame delay 2 because some pcams may be one frame behind the master and we must not overlap them
		context_.ReadCfg.EnableCircularBuf = 1;
		context_.ReadCfg.EnableSync = 1;
		context_.ReadCfg.PointNum = master_select;
		context_.ReadCfg.EnableFrameCounter = 0;
		context_.ReadCfg.FixedFrameStoreAddr = 0; //ignored, no park
		context_.ReadCfg.GenLockRepeat = 1; //Repeat previous frame on frame error (genlock?)
		status = XAxiVdma_DmaConfig(drv_inst_, XAXIVDMA_READ, &context_.ReadCfg);

		if (XST_SUCCESS != status)
		{
			return XST_FAILURE;
		}
		uint32_t addr = frame_buf_base_addr_;
		for (int iFrm=0; iFrm<drv_inst_->MaxNumFrames; ++iFrm) {
			context_.ReadCfg.FrameStoreStartAddr[iFrm] = addr;
			//memset((void*)addr,0,context_.ReadCfg.HoriSizeInput * context_.ReadCfg.VertSizeInput);
			addr += context_.ReadCfg.HoriSizeInput * context_.ReadCfg.VertSizeInput;
		}
		status = XAxiVdma_DmaSetBufferAddr(drv_inst_, XAXIVDMA_READ, context_.ReadCfg.FrameStoreStartAddr);
		if (XST_SUCCESS != status)
		{
			return XST_FAILURE;
		}
		//Use quad frame buffering, if FRMSTR_REG is enabled in hardware
		//If not, use all configured in hardware
		XAxiVdma_SetFrmStore(drv_inst_, drv_inst_->MaxNumFrames, XAXIVDMA_READ);
		//Clear errors in SR
		XAxiVdma_ClearChannelErrors(&drv_inst_->ReadChannel, XAXIVDMA_SR_ERR_ALL_MASK);
		//Enable read channel error and frame count interrupts
		XAxiVdma_IntrEnable(drv_inst_, XAXIVDMA_IXR_ERROR_MASK, XAXIVDMA_READ);

		return XST_SUCCESS;
	}

	int enableRead(XAxiVdma *drv_inst_)
	{
		XStatus status;
		//Start read channel
		status = XAxiVdma_DmaStart(drv_inst_, XAXIVDMA_READ);
		if (XST_SUCCESS != status)
		{
			return XST_FAILURE;
		}

		return XST_SUCCESS;
	}
	int configureWrite(XAxiVdma *drv_inst_, uint32_t frame_buf_base_addr_, uint16_t h_res, uint16_t v_res, uint16_t h_full_res, uint16_t v_full_res,
			u8 master_select, u8 is_master)
	{
		XAxiVdma_ClearDmaChannelErrors(drv_inst_, XAXIVDMA_WRITE, XAXIVDMA_SR_ERR_ALL_MASK);

		XStatus status;
		context_.WriteCfg.HoriSizeInput = h_res * drv_inst_->WriteChannel.StreamWidth;
		context_.WriteCfg.VertSizeInput = v_res;
		context_.WriteCfg.Stride = h_full_res * drv_inst_->WriteChannel.StreamWidth;
		// All write channels are Genlock Slave which may be set to follow any of themselves.
		// At runtime, one is selected to be "promoted to master" by disabling its genlock, making it cycle through all framebuffers blindly. All other channels are slaves to it.
		// Four framebuffers are needed, because both the input channels and the output channel may be behind one frame to their supposed frame pointer:
		// master - 0: master and input slaves
		// master - 1: input slaves that are lagging
		// master - 2: output slave
		// master - 3: output slave if it's lagging
		context_.WriteCfg.FrameDelay = 0; // Slaves try to position themselves on the exact same frame as the appointed master
		context_.WriteCfg.EnableCircularBuf = 1; // Appointed master must cycle through all framebuffers, not park
		context_.WriteCfg.EnableSync = !is_master; // Appointed master has sync disabled, all others have it enabled
		context_.WriteCfg.PointNum = master_select; // Everyone is a slave to the appointed master
		context_.WriteCfg.EnableFrameCounter = 0;
		context_.WriteCfg.FixedFrameStoreAddr = 0;
		context_.WriteCfg.GenLockRepeat = 1;
		status = XAxiVdma_DmaConfig(drv_inst_, XAXIVDMA_WRITE, &context_.WriteCfg);
		if (XST_SUCCESS != status)
		{
			return XST_FAILURE;
		}
//		status=XAxiVdma_GenLockSourceSelect(&drv_inst_,XAXIVDMA_EXTERNAL_GENLOCK,XAXIVDMA_WRITE);
//		if (XST_SUCCESS != status)
//		{
//					throw std::runtime_error(__FILE__ ":" LINE_STRING);
//		}

		uint32_t addr = frame_buf_base_addr_;
		for (int iFrm=0; iFrm<drv_inst_->MaxNumFrames; ++iFrm) {
			context_.WriteCfg.FrameStoreStartAddr[iFrm] = addr;
			//addr += context_.WriteCfg.HoriSizeInput * context_.WriteCfg.VertSizeInput;
			addr += context_.WriteCfg.Stride * v_full_res;
		}
		status = XAxiVdma_DmaSetBufferAddr(drv_inst_, XAXIVDMA_WRITE, context_.WriteCfg.FrameStoreStartAddr);
		if (XST_SUCCESS != status)
		{
			return XST_FAILURE;
		}
		//Use quad frame buffering, if FRMSTR_REG is enabled in hardware
		//If not, use all configured in hardware
		XAxiVdma_SetFrmStore(drv_inst_, drv_inst_->MaxNumFrames, XAXIVDMA_WRITE);
		//Clear errors in SR
		XAxiVdma_ClearChannelErrors(&drv_inst_->WriteChannel, XAXIVDMA_SR_ERR_ALL_MASK);
		//Unmask error interrupts
		XAxiVdma_MaskS2MMErrIntr(drv_inst_, ~XAXIVDMA_S2MM_IRQ_ERR_ALL_MASK, XAXIVDMA_WRITE);
		//Enable write channel error and frame count interrupts
		XAxiVdma_IntrEnable(drv_inst_, XAXIVDMA_IXR_ERROR_MASK, XAXIVDMA_WRITE);

		return XST_SUCCESS;
	}
	int enableWrite(XAxiVdma *drv_inst_) {
		XStatus status;
		//Start read channel
		status = XAxiVdma_DmaStart(drv_inst_, XAXIVDMA_WRITE);
		if (XST_SUCCESS != status)
		{
			return XST_FAILURE;
		}

		return XST_SUCCESS;
	}
	void readHandler(uint32_t irq_types)
	{
		//std::cout << "VDMA:read complete" << std::endl;
		xil_printf("\nVDMA:read complete\n");
	}
	void writeHandler(uint32_t irq_types)
	{
		//std::cout << "VDMA:write complete" << std::endl;
		xil_printf("\nVDMA:write complete\n");
	}
	void readErrorHandler(uint32_t mask)
	{
		//std::cout << "VDMA:read error" << std::endl;
		xil_printf("\nVDMA:read error\n");
	}
	void writeErrorHandler(uint32_t mask)
	{
		//std::cout << "VDMA:write error" << std::endl;
		xil_printf("\nVDMA:write error\n");
	}
