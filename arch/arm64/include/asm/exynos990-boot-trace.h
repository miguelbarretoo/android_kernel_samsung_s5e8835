/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __ASM_EXYNOS990_BOOT_TRACE_H
#define __ASM_EXYNOS990_BOOT_TRACE_H

/* The 4.19 recovery reads this 16 KiB slot as its ramoops pmsg zone. */
#define EXYNOS990_BOOT_TRACE_ADDR		0xeb30c000
#define EXYNOS990_BOOT_TRACE_MAGIC		0x43474244
#define EXYNOS990_BOOT_TRACE_ENTRY		0x3139324d /* M291 */
#define EXYNOS990_BOOT_TRACE_PRE_MMU		0x3239324d /* M292 */
#define EXYNOS990_BOOT_TRACE_SETUP_ARCH	0x3339324d /* M293 */

/* Payload: tag followed by four little-endian u64 values. */
#define EXYNOS990_TRACE_FAULT		0x544c4658 /* XFLT */
#define EXYNOS990_TRACE_FDT		0x54444658 /* XFDT */
#define EXYNOS990_TRACE_FDT_ADDRESS	1
#define EXYNOS990_TRACE_FDT_MAGIC	2
#define EXYNOS990_TRACE_FDT_SIZE	3
#define EXYNOS990_TRACE_FDT_HEADER	4
#define EXYNOS990_TRACE_FDT_NO_RAM	5

#ifndef __ASSEMBLY__
#include <linux/types.h>

#ifdef CONFIG_MODEL_X1SLTE
void exynos990_boot_trace_error(u32 tag, u64 a, u64 b, u64 c, u64 d);
#else
static inline void exynos990_boot_trace_error(u32 tag, u64 a, u64 b, u64 c, u64 d)
{
}
#endif
#endif

#endif
