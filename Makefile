.SUFFIXES:

ifeq ($(strip $(DEVKITPRO)),)
$(error "DEVKITPRO is not set")
endif

include $(DEVKITPRO)/wups/share/wups_rules

WUT_ROOT  := $(DEVKITPRO)/wut
WUPS_ROOT := $(DEVKITPRO)/wups

TARGET   := QuickResolution
BUILD    := build
SOURCES  := .
DATA     := data
INCLUDES := .

CFLAGS   := -Wall -Wextra -O2 -ffunction-sections $(MACHDEP)
CXXFLAGS := $(CFLAGS) -std=gnu++20
ASFLAGS  := -g $(ARCH)

LDFLAGS  = -g $(ARCH) $(RPXSPECS) \
           -Wl,-Map,$(notdir $*.map) \
           -T$(WUPS_ROOT)/share/libwupsbackend.ld \
           $(WUPSSPECS)

LIBS    := -lwups -lwut
LIBDIRS := $(WUPS_ROOT) $(WUT_ROOT)

include $(DEVKITPRO)/wups/share/wups_rules
