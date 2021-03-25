# Inherit some common stuff.
$(call inherit-product, vendor/calyx/config/common.mk)

# Inherit device configuration
$(call inherit-product, device/google/coral/device-calyx.mk)
$(call inherit-product, device/google/coral/aosp_coral.mk)

DEVICE_PACKAGE_OVERLAYS += vendor/google_devices/coral/vendor_overlay/carrier-naked/

## Device identifier. This must come after all inclusions
PRODUCT_NAME := calyx_coral
PRODUCT_MODEL := Pixel 4 XL
PRODUCT_RESTRICT_VENDOR_FILES := false
PRODUCT_ENFORCE_ARTIFACT_PATH_REQUIREMENTS :=
