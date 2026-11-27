// SPDX-License-Identifier: GPL-2.0
/*
 * The 4.19 ion_iovmm_map()/ion_iovmm_unmap() for the Exynos 990 vendor
 * drivers (MFC, G2D, ...), on DMA-BUF attachments: 5.15 has no ION buffer
 * to take the sg_table from, so map the attachment here and remember it
 * until the IOVA is unmapped.
 */
#include <linux/dma-buf.h>
#include <linux/exynos_iovmm.h>
#include <linux/ion_exynos.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>

struct ion_iovmm_mapping {
	struct list_head node;
	struct dma_buf_attachment *attachment;
	struct sg_table *sgt;
	enum dma_data_direction dir;
	dma_addr_t iova;
};

/* shortcut: linear list; fine for the tens of buffers a codec maps at once */
static LIST_HEAD(ion_iovmm_mappings);
static DEFINE_MUTEX(ion_iovmm_lock);

dma_addr_t ion_iovmm_map(struct dma_buf_attachment *attachment,
			 off_t offset, size_t size,
			 enum dma_data_direction direction, int prop)
{
	struct ion_iovmm_mapping *map;
	struct sg_table *sgt;
	dma_addr_t iova;

	map = kzalloc(sizeof(*map), GFP_KERNEL);
	if (!map)
		return -ENOMEM;

	sgt = dma_buf_map_attachment(attachment, direction);
	if (IS_ERR(sgt)) {
		kfree(map);
		return PTR_ERR(sgt);
	}

	iova = iovmm_map(attachment->dev, sgt->sgl, offset, size, direction, prop);
	if (IS_ERR_VALUE(iova)) {
		dma_buf_unmap_attachment(attachment, sgt, direction);
		kfree(map);
		return iova;
	}

	map->attachment = attachment;
	map->sgt = sgt;
	map->dir = direction;
	map->iova = iova;
	mutex_lock(&ion_iovmm_lock);
	list_add(&map->node, &ion_iovmm_mappings);
	mutex_unlock(&ion_iovmm_lock);

	return iova;
}
EXPORT_SYMBOL_GPL(ion_iovmm_map);

void ion_iovmm_unmap(struct dma_buf_attachment *attachment, dma_addr_t iova)
{
	struct ion_iovmm_mapping *map, *found = NULL;

	mutex_lock(&ion_iovmm_lock);
	list_for_each_entry(map, &ion_iovmm_mappings, node) {
		if (map->attachment == attachment && map->iova == iova) {
			list_del(&map->node);
			found = map;
			break;
		}
	}
	mutex_unlock(&ion_iovmm_lock);

	if (WARN(!found, "iova %pad not mapped for %s\n", &iova,
		 dev_name(attachment->dev)))
		return;

	iovmm_unmap(attachment->dev, found->iova);
	dma_buf_unmap_attachment(attachment, found->sgt, found->dir);
	kfree(found);
}
EXPORT_SYMBOL_GPL(ion_iovmm_unmap);
