/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PHY_EXYNOS_USB_H
#define _LINUX_PHY_EXYNOS_USB_H

/* PHY_MODE_USB_DEVICE submodes. Zero is the ordinary role change. */
enum exynos_usb_device_submode {
	EXYNOS_USB_PULLUP_OFF = 1,
	EXYNOS_USB_PULLUP_ON,
};

#endif
