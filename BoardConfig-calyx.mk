#
# Copyright (C) 2020-2021 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Lineage Health
TARGET_HEALTH_CHARGING_CONTROL_CHARGING_PATH := /sys/class/power_supply/charger/charge_disable
TARGET_HEALTH_CHARGING_CONTROL_CHARGING_ENABLED := 0
TARGET_HEALTH_CHARGING_CONTROL_CHARGING_DISABLED := 1

# Manifests
DEVICE_MANIFEST_FILE += device/google/coral/lineage_manifest.xml
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE += vendor/calyx/config/device_framework_matrix.xml

# Partitions
AB_OTA_PARTITIONS += \
    vendor
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4

# SELinux
BOARD_SEPOLICY_DIRS += device/google/coral/sepolicy-lineage/dynamic
BOARD_SEPOLICY_DIRS += device/google/coral/sepolicy-lineage/vendor

# Needed for camera
TARGET_GRALLOC_HANDLE_HAS_RESERVED_SIZE := true

PLATFORM_SECURITY_PATCH := 2022-10-05
