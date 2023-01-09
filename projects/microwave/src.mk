ifneq (,$(PLATFORM))
SRC_DIRS += $(PROJECT)/src/platform/$(PLATFORM)
endif

ifneq (,$(EXAMPLE))
SRC_DIRS += $(PROJECT)/src/examples/$(EXAMPLE)
include $(PROJECT)/src/examples/$(EXAMPLE)/src.mk
endif

TINYIIOD=y
LIBRARIES += iio
SRC_DIRS += $(NO-OS)/iio/iio_app
