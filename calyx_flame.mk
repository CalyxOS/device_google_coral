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
    TARGET_PRODUCT=flame \
    PRIVATE_BUILD_DESC="flame-user 12 SP2A.220305.012 8177914 release-keys"

BUILD_FINGERPRINT := google/flame/flame:12/SP2A.220305.012/8177914:user/release-keys

PRODUCT_RESTRICT_VENDOR_FILES := false

$(call inherit-product, vendor/google/flame/flame-vendor.mk)
