/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The 4.19 <soc/samsung/exynos-debug.h> watchdog hooks Exynos 990 drivers call on a
 * fatal hang. 5.15 has no such emergency-reset API, so they compile out
 * and the caller's BUG()/panic path resets instead.
 */
#ifndef __SOC_SAMSUNG_EXYNOS_DEBUG_H__
#define __SOC_SAMSUNG_EXYNOS_DEBUG_H__

#define s3c2410wdt_set_emergency_reset(a, b)	do { } while (0)

#endif
