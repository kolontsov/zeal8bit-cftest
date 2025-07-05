ifndef ZOS_PATH
    $(error "Failure: ZOS_PATH variable not found. It must point to Zeal 8-bit OS path.")
endif

include $(ZOS_PATH)/kernel_headers/sdcc/base_sdcc.mk
