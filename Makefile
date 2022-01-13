include lib/make-pal/pal.mak
DIR_BUILD:=build
DIR_LIB:=lib
DIR_SOURCE:=src
DIR_INCLUDE:=include
CC:=gcc

MAIN_NAME:=sids-cmdchat
MAIN_SRC_C:=$(wildcard $(DIR_SOURCE)/*.c)
MAIN_SRC:= $(MAIN_SRC_C)
MAIN_OBJ:=$(MAIN_SRC_C:$(DIR_SOURCE)/%.c=$(DIR_BUILD)/%.o)
MAIN_DEP:=$(MAIN_OBJ:%.o=%.d)
MAIN_CC_FLAGS:=-g -W -Wall -Wextra -Wpedantic -Wconversion -Wshadow -I$(DIR_INCLUDE)
MAIN_LD_FLAGS:=

.PHONY: all main clean

all: main

main: $(DIR_BUILD) $(DIR_BUILD)/$(MAIN_NAME).$(EXT_BIN)
$(DIR_BUILD)/$(MAIN_NAME).$(EXT_BIN): $(MAIN_OBJ)
	$(CC) $(^) -o $(@) $(MAIN_CC_FLAGS) $(MAIN_LD_FLAGS)

# Compile source files to object files.
$(DIR_BUILD)/%.o: $(DIR_SOURCE)/%.c
	$(CC) $(<) -o $(@) $(MAIN_CC_FLAGS) -c -MMD

# Make sure to recompile source files after a header they include changes.
-include $(MAIN_DEP)

$(DIR_BUILD):
	$(call pal_mkdir,$(@))
clean:
	$(call pal_rmdir,$(DIR_BUILD))
