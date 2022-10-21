/*
 * AXI_VDMA.h
 *
 *  Created on: Sep 2, 2016
 *      Author: Elod
 */

#ifndef AXI_VDMA_H_
#define AXI_VDMA_H_

int vdma_init(XAxiVdma *drv_inst_, uint16_t dev_id);
int resetRead(XAxiVdma *drv_inst_);
int resetWrite(XAxiVdma *drv_inst_);
int configureRead(XAxiVdma *drv_inst_, uint32_t frame_buf_base_addr_, uint16_t h_res, uint16_t v_res, u8 master_select);
int enableRead(XAxiVdma *drv_inst_);
int configureWrite(XAxiVdma *drv_inst_, uint32_t frame_buf_base_addr_, uint16_t h_res, uint16_t v_res, uint16_t h_full_res, uint16_t v_full_res,
		u8 master_select, u8 is_master);
int enableWrite(XAxiVdma *drv_inst_);
void readHandler(uint32_t irq_types);
void writeHandler(uint32_t irq_types);
void readErrorHandler(uint32_t mask);
void writeErrorHandler(uint32_t mask);



#endif /* AXI_VDMA_H_ */
