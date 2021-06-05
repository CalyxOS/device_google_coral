# Inherit some common stuff.
$(call inherit-product, vendor/calyx/config/common.mk)

# Inherit device configuration
$(call inherit-product, device/google/coral/coral/device-calyx.mk)
$(call inherit-product, device/google/coral/aosp_coral.mk)

## Device identifier. This must come after all inclusions
PRODUCT_NAME := calyx_coral
PRODUCT_MODEL := Pixel 4 XL
PRODUCT_BRAND := google

PRODUCT_BUILD_PROP_OVERRIDES += \
    TARGET_PRODUCT=coral \
    PRIVATE_BUILD_DESC="coral-user 12 SQ1A.220105.002 7961164 release-keys"

BUILD_FINGERPRINT := google/coral/coral:12/SQ1A.220105.002/7961164:user/release-keys

PRODUCT_RESTRICT_VENDOR_FILES := false

$(call inherit-product, vendor/google/coral/coral-vendor.mk)
