# Inherit some common stuff.
$(call inherit-product, vendor/calyx/config/common_phone.mk)

# Inherit device configuration
$(call inherit-product, device/google/coral/flame/device-calyx.mk)
$(call inherit-product, device/google/coral/aosp_flame.mk)

## Device identifier. This must come after all inclusions
PRODUCT_NAME := calyx_flame
PRODUCT_MODEL := Pixel 4
PRODUCT_BRAND := google

PRODUCT_BUILD_PROP_OVERRIDES += \
    TARGET_PRODUCT=flame \
    PRIVATE_BUILD_DESC="flame-user 13 TP1A.221005.002.B2 9382335 release-keys"

BUILD_FINGERPRINT := google/flame/flame:13/TP1A.221005.002.B2/9382335:user/release-keys

PRODUCT_RESTRICT_VENDOR_FILES := false

$(call inherit-product, vendor/google/flame/flame-vendor.mk)
