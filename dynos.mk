# ----------------------
# Dynamic Options System
# ----------------------

DYNOS_INPUT_DIR := ./dynos
DYNOS_OUTPUT_DIR := $(BUILD_DIR)/dynos
DYNOS_PACKS_DIR := $(BUILD_DIR)/dynos/packs
DYNOS_INIT := \
    mkdir -p $(DYNOS_INPUT_DIR); \
    mkdir -p $(DYNOS_OUTPUT_DIR); \
    mkdir -p $(DYNOS_PACKS_DIR); \
    cp -f -r $(DYNOS_INPUT_DIR) $(BUILD_DIR) 2>/dev/null || true ;

DYNOS_DO := $(shell $(call DYNOS_INIT))
INCLUDE_CFLAGS += -DDYNOS

$(BUILD_DIR)/data/dynos_opt_vanilla_c.o: $(BUILD_DIR)/include/text_strings.h

# ----
# Coop
# ----

SM64EX_COOP := $(or $(and $(wildcard src/pc/network/network.h),1),0)
ifeq ($(SM64EX_COOP),1)
INCLUDE_CFLAGS += -DDYNOS_COOP
endif

