/* SPDX-License-Identifier: GPL-2.0 */
/* videobuf2 dma-sg memops with an Exynos 990 IOVMM address per plane */
#ifndef __MFC_VB2_DMA_SG_H__
#define __MFC_VB2_DMA_SG_H__

#include <media/videobuf2-v4l2.h>

extern const struct vb2_mem_ops mfc_vb2_dma_sg_memops;
dma_addr_t mfc_vb2_dma_sg_plane_dma_addr(struct vb2_buffer *vb,
					 unsigned int plane_no);

#endif
