DEVICE_PACKAGE_OVERLAYS += device/google/coral/overlay-calyx

# Elmyra
PRODUCT_PACKAGES += \
    ElmyraService

# EUICC
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.euicc.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/android.hardware.telephony.euicc.xml \
    device/google/coral/permissions/privapp-permissions-aosp-extended.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/privapp-permissions-aosp-extended.xml

# For Google Camera
PRODUCT_COPY_FILES += \
    device/google/coral/the_experiences.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/sysconfig/the_experiences.xml
