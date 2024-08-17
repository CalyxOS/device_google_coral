DEVICE_PACKAGE_OVERLAYS += device/google/coral/overlay-calyx

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    vendor/qcom/opensource/commonsys-intf/display \
    vendor/qcom/opensource/display

# Add common definitions for Qualcomm
$(call inherit-product, hardware/qcom-caf/common/common.mk)

# DebugFS
PRODUCT_SET_DEBUGFS_RESTRICTIONS := true

# Elmyra
PRODUCT_PACKAGES += \
    ElmyraService

# EUICC
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.euicc.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/android.hardware.telephony.euicc.xml \
    device/google/coral/permissions/permissions_com.google.android.euicc.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/permissions_com.google.android.euicc.xml

# For Google Camera
PRODUCT_COPY_FILES += \
    device/google/coral/the_experiences.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/sysconfig/the_experiences.xml

# HBM
PRODUCT_COPY_FILES += \
    device/google/coral/permissions/permissions_com.android.hbmsvmanager.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/permissions_com.android.hbmsvmanager.xml

TARGET_PREBUILT_KERNEL := device/google/coral-kernel/Image.lz4

# Lineage Health
include hardware/google/pixel/lineage_health/device.mk

# LiveDisplay
PRODUCT_PACKAGES += \
    vendor.lineage.livedisplay@2.0-service-sdm

# RCS
PRODUCT_PACKAGES += \
    PresencePolling \
    RcsService

# Touch
include hardware/google/pixel/touch/device.mk

# Build necessary packages for vendor

# CHRE
PRODUCT_PACKAGES += \
    chre

# Configstore
PRODUCT_PACKAGES += \
    disable_configstore

# GPS
PRODUCT_PACKAGES += \
    flp.conf

# Identity credential
PRODUCT_PACKAGES += \
    android.hardware.identity_credential.xml

# Sound trigger
PRODUCT_PACKAGES += \
    sound_trigger.primary.msmnile

# VNDK FWK detect
PRODUCT_PACKAGES += \
    libvndfwk_detect_jni.qti.vendor

# Wi-Fi
PRODUCT_PACKAGES += \
    libwifi-hal-ctrl:64

# Properties
TARGET_VENDOR_PROP := $(LOCAL_PATH)/vendor.prop
