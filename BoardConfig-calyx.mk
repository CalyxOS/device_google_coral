# Inline kernel building
BOARD_KERNEL_IMAGE_NAME := Image.lz4
ifneq ($(INLINE_KERNEL_BUILDING),false)
TARGET_COMPILE_WITH_MSM_KERNEL := true
TARGET_KERNEL_ADDITIONAL_FLAGS := \
    DTC=$(shell pwd)/prebuilts/tools-lineage/$(HOST_OS)-x86/dtc/dtc \
    MKDTIMG=$(shell pwd)/prebuilts/misc/$(HOST_OS)-x86/libufdt/mkdtimg
TARGET_KERNEL_ARCH := arm64
TARGET_KERNEL_CLANG_COMPILE := true
KERNEL_LD := LD=ld.lld
TARGET_KERNEL_CONFIG := floral_defconfig
TARGET_KERNEL_SOURCE := kernel/google/coral
TARGET_NEEDS_DTBOIMAGE := true
endif
