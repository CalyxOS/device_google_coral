# Inherit some common stuff.
$(call inherit-product, vendor/calyx/config/common.mk)

# Inherit device configuration
$(call inherit-product, device/google/coral/device-calyx.mk)
$(call inherit-product, device/google/coral/aosp_coral.mk)

## Device identifier. This must come after all inclusions
PRODUCT_NAME := calyx_coral
PRODUCT_MODEL := Pixel 4 XL
PRODUCT_RESTRICT_VENDOR_FILES := false
PRODUCT_ENFORCE_ARTIFACT_PATH_REQUIREMENTS :=

PRODUCT_SOONG_NAMESPACES += vendor/google_devices/coral
