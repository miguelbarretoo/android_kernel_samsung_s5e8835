/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_DEVICE_H
#define __ASM_DEVICE_H

struct dev_archdata {
#ifdef CONFIG_EXYNOS990_IOMMU
	void *iommu; /* struct exynos_iommu_owner of the 4.19 Exynos 990 System MMU */
#endif
};

struct pdev_archdata {
};

#endif
