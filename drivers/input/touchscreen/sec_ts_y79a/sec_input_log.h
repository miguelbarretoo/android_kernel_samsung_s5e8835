/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The Samsung additions the 4.19 driver found in <uapi/linux/input.h>:
 * input feature bits and the sec_input log helpers (the variant without
 * the TSP log buffer, which 5.15 provides only inside sec_input).
 */
#ifndef __SEC_TS_Y79A_INPUT_LOG_H__
#define __SEC_TS_Y79A_INPUT_LOG_H__

#include <linux/device.h>

#define INPUT_FEATURE_ENABLE_SETTINGS_AOT	(1 << 0) /* Double tap wakeup settings */
#define INPUT_FEATURE_ENABLE_PRESSURE		(1 << 1) /* homekey pressure */
#define INPUT_FEATURE_ENABLE_SYNC_RR120		(1 << 2) /* sync reportrate 120hz */
#define INPUT_FEATURE_ENABLE_VRR		(1 << 3) /* variable refresh rate (support 240hz) */
#define INPUT_FEATURE_SUPPORT_OPEN_SHORT_TEST		(1 << 8) /* open/short test support */
#define INPUT_FEATURE_SUPPORT_MIS_CALIBRATION_TEST	(1 << 9) /* mis-calibration test support */
#define INPUT_FEATURE_ENABLE_MULTI_CALIBRATION		(1 << 10) /* multi calibration support */

/* Samsung input event codes (4.19 input-event-codes.h) */
#ifndef ABS_MT_CUSTOM
#define ABS_MT_CUSTOM		0x3e	/* custom event */
#endif
#ifndef KEY_BLACK_UI_GESTURE
#define KEY_BLACK_UI_GESTURE	0x1c7
#endif

#ifndef KEY_INT_CANCEL
#define KEY_INT_CANCEL		0x2be	/* for touch event skip */
#endif

#define SECLOG			"[sec_input]"
#define INPUT_LOG_BUF_SIZE	512

#define input_dbg(mode, dev, fmt, ...)	dev_dbg(dev, SECLOG fmt, ## __VA_ARGS__)
#define input_info(mode, dev, fmt, ...)	dev_info(dev, SECLOG fmt, ## __VA_ARGS__)
#define input_err(mode, dev, fmt, ...)	dev_err(dev, SECLOG fmt, ## __VA_ARGS__)
#define input_raw_info(mode, dev, fmt, ...) input_info(mode, dev, fmt, ## __VA_ARGS__)
#define input_log_fix()			do { } while (0)
#define input_raw_data_clear()		do { } while (0)
/* the TSP command history lives in the sec_input TSP log, not built here */
#define sec_debug_tsp_command_history(buf)	do { } while (0)

#endif
