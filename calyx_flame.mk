# Inherit some common stuff.
$(call inherit-product, vendor/calyx/config/common.mk)

# Inherit device configuration
$(call inherit-product, device/google/coral/device-calyx.mk)
$(call inherit-product, device/google/coral/aosp_flame.mk)

## Device identifier. This must come after all inclusions
PRODUCT_NAME := calyx_flame
PRODUCT_MODEL := Pixel 4
PRODUCT_BRAND := google

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRODUCT_NAME=flame \
    PRIVATE_BUILD_DESC="flame-user 11 RQ3A.211001.001 7641976 release-keys"

BUILD_FINGERPRINT := google/flame/flame:11/RQ3A.211001.001/7641976:user/release-keys

PRODUCT_RESTRICT_VENDOR_FILES := false
PRODUCT_ENFORCE_ARTIFACT_PATH_REQUIREMENTS :=

PRODUCT_SOONG_NAMESPACES += vendor/google_devices/flame
