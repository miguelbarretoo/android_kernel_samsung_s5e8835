/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The part of the 4.19 Exynos ION kernel API that the Exynos 990 vendor
 * drivers still use, on top of DMA-BUF heaps (5.15 has no ION).
 */
#ifndef __LINUX_ION_EXYNOS_H__
#define __LINUX_ION_EXYNOS_H__

#include <linux/dma-direction.h>
#include <linux/dma-heap.h>
#include <linux/err.h>
#include <linux/fcntl.h>
#include <linux/string.h>

struct dma_buf_attachment;

#define ION_FLAG_CACHED 1
#define ION_FLAG_NOZEROED 8
#define ION_FLAG_PROTECTED 16

/* shortcut: only ion_system_heap is backed, as in ion_compat; add heaps with their DT nodes */
static inline struct dma_buf *ion_alloc_dmabuf(const char *heap_name,
					       size_t len, unsigned int flags)
{
	struct dma_heap *heap;
	struct dma_buf *buf;

	if (strcmp(heap_name, "ion_system_heap"))
		return ERR_PTR(-ENODEV);

	heap = dma_heap_find(flags & ION_FLAG_CACHED ?
			     "system" : "system-uncached");
	if (!heap)
		return ERR_PTR(-ENODEV);

	buf = dma_heap_buffer_alloc(heap, len, O_RDWR, 0);
	dma_heap_put(heap);
	return buf;
}

#ifdef CONFIG_EXYNOS_IOVMM
/* map a DMA-BUF attachment in its device's Exynos 990 IOVMM domain */
dma_addr_t ion_iovmm_map(struct dma_buf_attachment *attachment,
			 off_t offset, size_t size,
			 enum dma_data_direction direction, int prop);
void ion_iovmm_unmap(struct dma_buf_attachment *attachment, dma_addr_t iova);
#endif

#endif /* __LINUX_ION_EXYNOS_H__ */
