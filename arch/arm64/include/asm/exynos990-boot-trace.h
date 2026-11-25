/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __ASM_EXYNOS990_BOOT_TRACE_H
#define __ASM_EXYNOS990_BOOT_TRACE_H

/* The 4.19 recovery reads this 16 KiB slot as its ramoops pmsg zone. */
#define EXYNOS990_BOOT_TRACE_ADDR		0xeb30c000
#define EXYNOS990_BOOT_TRACE_MAGIC		0x43474244
#define EXYNOS990_BOOT_TRACE_ENTRY		0x3139324d /* M291 */
#define EXYNOS990_BOOT_TRACE_PRE_MMU		0x3239324d /* M292 */
#define EXYNOS990_BOOT_TRACE_SETUP_ARCH	0x3339324d /* M293 */

#endif
