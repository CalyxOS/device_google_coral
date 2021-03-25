# Inherit some common stuff.
$(call inherit-product, vendor/calyx/config/common.mk)

# Inherit device configuration
$(call inherit-product, device/google/coral/device-calyx.mk)
$(call inherit-product, device/google/coral/aosp_flame.mk)

DEVICE_PACKAGE_OVERLAYS += vendor/google_devices/flame/vendor_overlay/carrier-naked/

## Device identifier. This must come after all inclusions
PRODUCT_NAME := calyx_flame
PRODUCT_MODEL := Pixel 4
PRODUCT_RESTRICT_VENDOR_FILES := false
PRODUCT_ENFORCE_ARTIFACT_PATH_REQUIREMENTS :=
