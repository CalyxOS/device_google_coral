# Inherit some common stuff.
$(call inherit-product, vendor/calyx/config/common.mk)

# Inherit device configuration
$(call inherit-product, device/google/coral/device-calyx.mk)
$(call inherit-product, device/google/coral/aosp_coral.mk)

## Device identifier. This must come after all inclusions
PRODUCT_NAME := calyx_coral
PRODUCT_MODEL := Pixel 4 XL
PRODUCT_BRAND := google

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRODUCT_NAME=coral \
    PRIVATE_BUILD_DESC="coral-user 11 RQ3A.210905.001 7511028 release-keys"

BUILD_FINGERPRINT := google/coral/coral:11/RQ3A.210905.001/7511028:user/release-keys

PRODUCT_RESTRICT_VENDOR_FILES := false
PRODUCT_ENFORCE_ARTIFACT_PATH_REQUIREMENTS :=

PRODUCT_SOONG_NAMESPACES += vendor/google_devices/coral
