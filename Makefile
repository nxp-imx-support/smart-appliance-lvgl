# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

#
# Makefile
#

CC ?= gcc
LVGL_DIR_NAME = lvgl
LVGL_DIR = ${shell pwd}
CPPFLAGS = -I$(LVGL_DIR)/ -I$(INC_DIR_SRC)/ -I$(INC_DIR_GENERATED)/ -I$(LVGL_DIR)/lvgl/src -g3 -ggdb -O3 -Wno-unused-result
CFLAGS = -O3 -g3 -ggdb -I$(LVGL_DIR)/ -I$(INC_DIR_SRC)/ -I$(INC_DIR_CUSTOM)/ -I$(INC_DIR_GENERATED)/ -I$(INC_DIR_ML)/ \
-I$(INC_DIR_MATTER)/ -I$(LVGL_DIR)/lvgl/src -I$(LVGL_DIR)/lvgl -I$(LVGL_DIR)/drm/ -Wall -Wshadow -Wundef -Wmissing-prototypes  \
-Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing \
-Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wcpp -Wtype-limits \
-Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar \
-Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion  \
-Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter \
-Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers \
-Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code \
-Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare \
-Wno-implicit-function-declaration -Wno-unused-result -Wno-undef
export LDFLAGS = -lm -lrt -ldrm
#LDFLAGS = -lm -lrt -ldrm -lwayland-client -lxkbcommon -lwayland-cursor -lrt -Llv_drivers/wayland/protocols/libdf-wayland-xdg-application-stable-latest
#BIN = $(BIN_DIR)/demo
BIN_DIR = ./bin
OBJ_DIR = ./obj
INC_DIR_SRC = ./src
INC_DIR_CUSTOM = ./src/custom
INC_DIR_GENERATED = ./src/generated
INC_DIR_ML = ./src/ml
INC_DIR_MATTER = ./src/matter
PRJ_DIR = ${shell pwd}/src
BIN = lvgl_demo

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
include $(LVGL_DIR)/src/src.mk
include $(LVGL_DIR)/src/generated/generated.mk
include $(LVGL_DIR)/src/custom/custom.mk
include $(LVGL_DIR)/ml/ml.mk
include $(LVGL_DIR)/matter/matter.mk

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CPPOBJS = $(CPPSRCS:.cpp=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC) $(CPPSRCS)
OBJS = $(AOBJS) $(COBJS) $(CPPOBJS)

.PHONY: all
all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

export LDFLAGS = -lm -lrt -ldrm -lg2d -ltensorflow-lite -lopencv_imgcodecs -lopencv_dnn -lopencv_imgproc -lopencv_core

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c -g $< -o $@

.PHONY: default
default: $(OBJS) main.o
	$(CXX) -o $(BIN) main.o $(OBJS) $(LDFLAGS)

	@mkdir -p obj_files
	@mv *.o ./obj_files/

.PHONY: clean
clean:
	rm -rf $(BIN) $(OBJS) main.o obj_files/
