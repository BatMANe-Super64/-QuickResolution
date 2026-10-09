# -----------------------------
# QuickResolutionPlugin Makefile
# -----------------------------

TITLE  := QuickResolution
TARGET := $(TITLE).elf
WPS    := $(TITLE).wps

# Use the pre-configured paths provided inside the container environment
DEVKITPRO ?= /opt/devkitpro
DEVKITPPC := $(DEVKITPRO)/devkitPPC

# Compiler Definition
CXX := $(DEVKITPPC)/bin/powerpc-eabi-g++

# Source / Object configurations
SRC := main.cpp
OBJ := $(SRC:.cpp=.o)

# Global Toolchain Header Bindings
CFLAGS := -std=gnu++20 -O2 -mcpu=powerpc -meabi -mhard-float -fPIC \
          -I$(DEVKITPRO)/wut/include \
          -I$(DEVKITPRO)/wut/include/sysapp \
          -I$(DEVKITPRO)/wups/include

# Library Linking Parameters
LDFLAGS := -r -L$(DEVKITPRO)/wut/lib \
           -L$(DEVKITPRO)/wups/lib \
           -lwups -lwut -lc -lgcc -lstdc++

# -----------------------------
# Compilation Rules
# -----------------------------
all: $(WPS)

# Convert the raw executable ELF into an Aroma-loadable .wps plugin
$(WPS): $(TARGET)
	wups-elf2wps $(TARGET) --output $(WPS)

# Link intermediate binaries
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

# Compile raw source modules
%.o: %.cpp
	$(CXX) $(CFLAGS) -x c++ -c $< -o $@

# Purge compilation structures
clean:
	rm -f $(OBJ) $(TARGET) $(WPS)
