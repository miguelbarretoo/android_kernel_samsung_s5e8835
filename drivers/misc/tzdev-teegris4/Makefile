subdir-ccflags-y += -I$(srctree)/drivers/misc/tzdev-teegris4/include
subdir-ccflags-y += -I$(srctree)/drivers/misc/tzdev-teegris4

obj-$(CONFIG_TZDEV)			+= lib/
obj-$(CONFIG_TZDEV)			+= core/
obj-$(CONFIG_TZDEV)			+= teec/
obj-$(CONFIG_TZDEV)			+= debug/
obj-$(CONFIG_TZDEV)			+= extensions/

ccflags-$(CONFIG_TZ_PMF) += -DCONFIG_TZ_PMF
