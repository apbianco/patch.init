# To build and flash:
# make clean; make; make program-dfu

# Project Name
TARGET = tm

# Sources
CPP_SOURCES = $(TARGET).cpp ../util/util.cpp

# Library Locations
LIBDAISY_DIR = ../../../DaisyExamples/libDaisy
DAISYSP_DIR = ../../../DaisyExamples/DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Use -DLOG_ON for debug logs on the serial line
override CPPFLAGS += -DLOG_ON -I../util
override CFLAGS += -Werror
