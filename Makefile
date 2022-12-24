# To build and flash:
# make clean; make; make program-dfu

# Project Name
TARGET = learn

# Sources
CPP_SOURCES = $(TARGET).cpp 

# Library Locations
LIBDAISY_DIR = ../../DaisyExamples/libDaisy
DAISYSP_DIR = ../../DaisyExamples/DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# LDFLAGS = -u _printf_float
