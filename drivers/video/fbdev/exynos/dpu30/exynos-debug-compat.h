/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The 4.19 <soc/samsung/exynos-debug.h> watchdog hooks the DPU calls on a
 * fatal hang. 5.15 has no such emergency-reset API, so they compile out
 * and the caller's BUG()/panic path resets instead.
 */
#ifndef __DPU30_EXYNOS_DEBUG_COMPAT_H__
#define __DPU30_EXYNOS_DEBUG_COMPAT_H__

#define s3c2410wdt_set_emergency_reset(a, b)	do { } while (0)

#endif
