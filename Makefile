
# Makefile to rebuild SM64 split image

### Default target ###

default: all

### Build Options ###

# These options can either be changed by modifying the makefile, or
# by building with 'make SETTING=value'. 'make clean' may be required.

# Version of the game to build
VERSION ?= us
# Graphics microcode used
GRUCODE ?= f3d_old
# If COMPARE is 1, check the output sha1sum when building 'all'
COMPARE ?= 1
# If NON_MATCHING is 1, define the NON_MATCHING and AVOID_UB macros when building (recommended)
NON_MATCHING ?= 1

# Build and optimize for Raspberry Pi(s)
TARGET_RPI ?= 0

# Build for Emscripten/WebGL
TARGET_WEB ?= 0

# Makeflag to enable OSX fixes
OSX_BUILD ?= 0

# Specify the target you are building for, TARGET_BITS=0 means native
TARGET_ARCH ?= native
TARGET_BITS ?= 0

# Disable better camera by default
BETTERCAMERA ?= 0
# Disable no drawing distance by default
NODRAWINGDISTANCE ?= 0
# Disable texture fixes by default (helps with them purists)
TEXTURE_FIX ?= 0
# Enable extended options menu by default
EXT_OPTIONS_MENU ?= 1
# Disable text-based save-files by default
TEXTSAVES ?= 0
# Load resources from external files
EXTERNAL_DATA ?= 0

# Various workarounds for weird toolchains

NO_BZERO_BCOPY ?= 0
NO_LDIV ?= 0

# Use OpenGL 1.3 renderer

LEGACY_GL ?= 0

# Automatic settings for PC port(s)

NON_MATCHING := 1
GRUCODE := f3dex2e
WINDOWS_BUILD ?= 0

# Attempt to detect OS

ifeq ($(OS),Windows_NT)
  HOST_OS ?= Windows
else
  HOST_OS ?= $(shell uname -s 2>/dev/null || echo Unknown)
  # some weird MINGW/Cygwin env that doesn't define $OS
  ifneq (,$(findstring MINGW,HOST_OS))
    HOST_OS := Windows
  endif
endif

ifeq ($(TARGET_WEB),0)
ifeq ($(HOST_OS),Windows)
WINDOWS_BUILD := 1
endif
endif

# MXE overrides

ifeq ($(WINDOWS_BUILD),1)
  ifeq ($(CROSS),i686-w64-mingw32.static-)
    TARGET_ARCH = i386pe
    TARGET_BITS = 32
    NO_BZERO_BCOPY := 1
  else ifeq ($(CROSS),x86_64-w64-mingw32.static-)
    TARGET_ARCH = i386pe
    TARGET_BITS = 64
    NO_BZERO_BCOPY := 1
  endif
endif

ifneq ($(TARGET_BITS),0)
  BITS := -m$(TARGET_BITS)
else
  BITS :=
endif

# Release (version) flag defs

ifeq ($(VERSION),jp)
  VERSION_CFLAGS := -DVERSION_JP
  VERSION_ASFLAGS := --defsym VERSION_JP=1
  GRUCODE_CFLAGS := -DF3D_OLD
  GRUCODE_ASFLAGS := --defsym F3D_OLD=1
  TARGET := sm64.jp
else
ifeq ($(VERSION),us)
  VERSION_CFLAGS := -DVERSION_US
  VERSION_ASFLAGS := --defsym VERSION_US=1
  GRUCODE_CFLAGS := -DF3D_OLD
  GRUCODE_ASFLAGS := --defsym F3D_OLD=1
  TARGET := sm64.us
else
ifeq ($(VERSION),eu)
  VERSION_CFLAGS := -DVERSION_EU
  VERSION_ASFLAGS := --defsym VERSION_EU=1
  GRUCODE_CFLAGS := -DF3D_NEW
  GRUCODE_ASFLAGS := --defsym F3D_NEW=1
  TARGET := sm64.eu
else
ifeq ($(VERSION),sh)
  $(warning Building SH is experimental and is prone to breaking. Try at your own risk.)
  VERSION_CFLAGS := -DVERSION_SH
  VERSION_ASFLAGS := --defsym VERSION_SH=1
  GRUCODE_CFLAGS := -DF3D_NEW
  GRUCODE_ASFLAGS := --defsym F3D_NEW=1
  TARGET := sm64.sh
# TODO: GET RID OF THIS!!! We should mandate assets for Shindou like EU but we dont have the addresses extracted yet so we'll just pretend you have everything extracted for now.
  NOEXTRACT := 1
else
  $(error unknown version "$(VERSION)")
endif
endif
endif
endif

# Stuff for showing the git hash in the intro on nightly builds
# From https://stackoverflow.com/questions/44038428/include-git-commit-hash-and-or-branch-name-in-c-c-source
ifeq ($(shell git rev-parse --abbrev-ref HEAD),nightly)
  GIT_HASH=`git rev-parse --short HEAD`
  COMPILE_TIME=`date -u +'%Y-%m-%d %H:%M:%S UTC'`
  VERSION_CFLAGS += -DNIGHTLY -DGIT_HASH="\"$(GIT_HASH)\"" -DCOMPILE_TIME="\"$(COMPILE_TIME)\""
endif

# Microcode

ifeq ($(GRUCODE),f3dex) # Fast3DEX
  GRUCODE_CFLAGS := -DF3DEX_GBI
  GRUCODE_ASFLAGS := --defsym F3DEX_GBI_SHARED=1 --defsym F3DEX_GBI=1
  TARGET := $(TARGET).f3dex
  COMPARE := 0
else
ifeq ($(GRUCODE), f3dex2) # Fast3DEX2
  GRUCODE_CFLAGS := -DF3DEX_GBI_2
  GRUCODE_ASFLAGS := --defsym F3DEX_GBI_SHARED=1 --defsym F3DEX_GBI_2=1
  TARGET := $(TARGET).f3dex2
  COMPARE := 0
else
ifeq ($(GRUCODE), f3dex2e) # Fast3DEX2 Extended (PC default)
  GRUCODE_CFLAGS := -DF3DEX_GBI_2E
  TARGET := $(TARGET).f3dex2e
  COMPARE := 0
else
ifeq ($(GRUCODE),f3d_new) # Fast3D 2.0H (Shindou)
  GRUCODE_CFLAGS := -DF3D_NEW
  GRUCODE_ASFLAGS := --defsym F3D_NEW=1
  TARGET := $(TARGET).f3d_new
  COMPARE := 0
else
ifeq ($(GRUCODE),f3dzex) # Fast3DZEX (2.0J / Animal Forest - Dōbutsu no Mori)
  $(warning Fast3DZEX is experimental. Try at your own risk.)
  GRUCODE_CFLAGS := -DF3DEX_GBI_2
  GRUCODE_ASFLAGS := --defsym F3DEX_GBI_SHARED=1 --defsym F3DZEX_GBI=1
  TARGET := $(TARGET).f3dzex
  COMPARE := 0
endif
endif
endif
endif
endif

# Default build is for PC now
VERSION_CFLAGS := $(VERSION_CFLAGS) -DNON_MATCHING -DAVOID_UB

ifeq ($(TARGET_RPI),1) # Define RPi to change SDL2 title & GLES2 hints
      VERSION_CFLAGS += -DUSE_GLES
endif

ifeq ($(OSX_BUILD),1) # Modify GFX & SDL2 for OSX GL
     VERSION_CFLAGS += -DOSX_BUILD
endif

VERSION_ASFLAGS := --defsym AVOID_UB=1
COMPARE := 0

ifeq ($(TARGET_WEB),1)
  VERSION_CFLAGS := $(VERSION_CFLAGS) -DTARGET_WEB
endif

################### Universal Dependencies ###################

# (This is a bit hacky, but a lot of rules implicitly depend
# on tools and assets, and we use directory globs further down
# in the makefile that we want should cover assets.)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)

# Make sure assets exist
NOEXTRACT ?= 0
ifeq ($(NOEXTRACT),0)
DUMMY != ./extract_assets.py $(VERSION) >&2 || echo FAIL
ifeq ($(DUMMY),FAIL)
  $(error Failed to extract assets)
endif
endif

# Make tools if out of date
DUMMY != make -C tools >&2 || echo FAIL
ifeq ($(DUMMY),FAIL)
  $(error Failed to build tools)
endif

endif
endif

################ Target Executable and Sources ###############

# BUILD_DIR is location where all build artifacts are placed
BUILD_DIR_BASE := build

ifeq ($(TARGET_WEB),1)
  BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)_web
else
  BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)_pc
endif

LIBULTRA := $(BUILD_DIR)/libultra.a

ifeq ($(TARGET_WEB),1)
EXE := $(BUILD_DIR)/$(TARGET).html
	else
	ifeq ($(WINDOWS_BUILD),1)
		EXE := $(BUILD_DIR)/$(TARGET).exe

		else # Linux builds/binary namer
		ifeq ($(TARGET_RPI),1)
			EXE := $(BUILD_DIR)/$(TARGET).arm
		else
			EXE := $(BUILD_DIR)/$(TARGET)
		endif
	endif
endif

ELF := $(BUILD_DIR)/$(TARGET).elf
LD_SCRIPT := sm64.ld
MIO0_DIR := $(BUILD_DIR)/bin
SOUND_BIN_DIR := $(BUILD_DIR)/sound
TEXTURE_DIR := textures
ACTOR_DIR := actors
LEVEL_DIRS := $(patsubst levels/%,%,$(dir $(wildcard levels/*/header.h)))

# Directories containing source files

# Hi, I'm a PC
SRC_DIRS := src src/engine src/game src/audio src/menu src/buffers actors levels bin data assets src/pc src/pc/gfx src/pc/audio src/pc/controller
ASM_DIRS :=

BIN_DIRS := bin bin/$(VERSION)

ULTRA_SRC_DIRS := lib/src lib/src/math
ULTRA_ASM_DIRS := lib/asm lib/data
ULTRA_BIN_DIRS := lib/bin

GODDARD_SRC_DIRS := src/goddard src/goddard/dynlists

MIPSISET := -mips2
MIPSBIT := -32

ifeq ($(VERSION),eu)
  OPT_FLAGS := -O2
else
ifeq ($(VERSION),sh)
  OPT_FLAGS := -O2
else
  OPT_FLAGS := -O2
endif
endif

# Set BITS (32/64) to compile for
OPT_FLAGS += $(BITS)

ifeq ($(TARGET_WEB),1)
  OPT_FLAGS := -O2 -g4 --source-map-base http://localhost:8080/
endif

# Use a default opt flag for gcc, then override if RPi

# OPT_FLAGS := -O2 # "Whole-compile optimization flag" Breaks sound on x86.

ifeq ($(TARGET_RPI),1)
	machine = $(shell sh -c 'uname -m 2>/dev/null || echo unknown')
# Raspberry Pi B+, Zero, etc
	ifneq (,$(findstring armv6l,$(machine)))
                OPT_FLAGS := -march=armv6zk+fp -mfpu=vfp -Ofast
        endif

# Raspberry Pi 2 and 3 in ARM 32bit mode
        ifneq (,$(findstring armv7l,$(machine)))
                model = $(shell sh -c 'cat /sys/firmware/devicetree/base/model 2>/dev/null || echo unknown')

                ifneq (,$(findstring 3,$(model)))
                         OPT_FLAGS := -march=armv8-a+crc -mtune=cortex-a53 -mfpu=neon-fp-armv8 -O3
                         else
                         OPT_FLAGS := -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -O3
                endif
        endif

# RPi3 or RPi4, in ARM64 (aarch64) mode. NEEDS TESTING 32BIT.
# DO NOT pass -mfpu stuff here, thats for 32bit ARM only and will fail for 64bit ARM.
        ifneq (,$(findstring aarch64,$(machine)))
                model = $(shell sh -c 'cat /sys/firmware/devicetree/base/model 2>/dev/null || echo unknown')
                ifneq (,$(findstring 3,$(model)))
                         OPT_FLAGS := -march=armv8-a+crc -mtune=cortex-a53 -O3
                else ifneq (,$(findstring 4,$(model)))
                         OPT_FLAGS := -march=armv8-a+crc+simd -mtune=cortex-a72 -O3
                endif

        endif
endif

# File dependencies and variables for specific files
include Makefile.split

# Source code files
LEVEL_C_FILES := $(wildcard levels/*/leveldata.c) $(wildcard levels/*/script.c) $(wildcard levels/*/geo.c)
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) $(LEVEL_C_FILES)
CXX_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
S_FILES := $(foreach dir,$(ASM_DIRS),$(wildcard $(dir)/*.s))
ULTRA_C_FILES := $(foreach dir,$(ULTRA_SRC_DIRS),$(wildcard $(dir)/*.c))
GODDARD_C_FILES := $(foreach dir,$(GODDARD_SRC_DIRS),$(wildcard $(dir)/*.c))

GENERATED_C_FILES := $(BUILD_DIR)/assets/mario_anim_data.c $(BUILD_DIR)/assets/demo_data.c \
  $(addprefix $(BUILD_DIR)/bin/,$(addsuffix _skybox.c,$(notdir $(basename $(wildcard textures/skyboxes/*.png)))))

ifeq ($(WINDOWS_BUILD),0)
  CXX_FILES :=
endif

# We need to keep this for now
# If we're not N64 use below

  ULTRA_C_FILES_SKIP := \
    sqrtf.c \
    string.c \
    sprintf.c \
    _Printf.c \
    kdebugserver.c \
    osInitialize.c \
    func_802F7140.c \
    func_802F71F0.c \
    func_802F4A20.c \
    EU_D_802f4330.c \
    D_802F4380.c \
    osLeoDiskInit.c \
    osCreateThread.c \
    osDestroyThread.c \
    osStartThread.c \
    osSetThreadPri.c \
    osPiStartDma.c \
    osPiRawStartDma.c \
    osPiRawReadIo.c \
    osPiGetCmdQueue.c \
    osJamMesg.c \
    osSendMesg.c \
    osRecvMesg.c \
    osSetEventMesg.c \
    osTimer.c \
    osSetTimer.c \
    osSetTime.c \
    osCreateViManager.c \
    osViSetSpecialFeatures.c \
    osVirtualToPhysical.c \
    osViBlack.c \
    osViSetEvent.c \
    osViSetMode.c \
    osViSwapBuffer.c \
    osSpTaskLoadGo.c \
    osCreatePiManager.c \
    osGetTime.c \
    osEepromProbe.c \
    osEepromWrite.c \
    osEepromLongWrite.c \
    osEepromRead.c \
    osEepromLongRead.c \
    osContInit.c \
    osContStartReadData.c \
    osAiGetLength.c \
    osAiSetFrequency.c \
    osAiSetNextBuffer.c \
    __osViInit.c \
    __osSyncPutChars.c \
    __osAtomicDec.c \
    __osSiRawStartDma.c \
    __osViSwapContext.c \
    __osViGetCurrentContext.c \
    __osDevMgrMain.c

  C_FILES := $(filter-out src/game/main.c,$(C_FILES))
  ULTRA_C_FILES := $(filter-out $(addprefix lib/src/,$(ULTRA_C_FILES_SKIP)),$(ULTRA_C_FILES))

# "If we're not N64, use the above"

ifeq ($(VERSION),sh)
SOUND_BANK_FILES := $(wildcard sound/sound_banks/*.json)
SOUND_SEQUENCE_FILES := $(wildcard sound/sequences/jp/*.m64) \
    $(wildcard sound/sequences/*.m64) \
    $(foreach file,$(wildcard sound/sequences/jp/*.s),$(BUILD_DIR)/$(file:.s=.m64)) \
    $(foreach file,$(wildcard sound/sequences/*.s),$(BUILD_DIR)/$(file:.s=.m64))
else
SOUND_BANK_FILES := $(wildcard sound/sound_banks/*.json)
SOUND_SEQUENCE_FILES := $(wildcard sound/sequences/$(VERSION)/*.m64) \
    $(wildcard sound/sequences/*.m64) \
    $(foreach file,$(wildcard sound/sequences/$(VERSION)/*.s),$(BUILD_DIR)/$(file:.s=.m64)) \
    $(foreach file,$(wildcard sound/sequences/*.s),$(BUILD_DIR)/$(file:.s=.m64))
endif

SOUND_SAMPLE_DIRS := $(wildcard sound/samples/*)
SOUND_SAMPLE_AIFFS := $(foreach dir,$(SOUND_SAMPLE_DIRS),$(wildcard $(dir)/*.aiff))
SOUND_SAMPLE_TABLES := $(foreach file,$(SOUND_SAMPLE_AIFFS),$(BUILD_DIR)/$(file:.aiff=.table))
SOUND_SAMPLE_AIFCS := $(foreach file,$(SOUND_SAMPLE_AIFFS),$(BUILD_DIR)/$(file:.aiff=.aifc))
SOUND_OBJ_FILES := $(SOUND_BIN_DIR)/sound_data.ctl.o \
                   $(SOUND_BIN_DIR)/sound_data.tbl.o \
                   $(SOUND_BIN_DIR)/sequences.bin.o \
                   $(SOUND_BIN_DIR)/bank_sets.o

# Object files
O_FILES := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
           $(foreach file,$(CXX_FILES),$(BUILD_DIR)/$(file:.cpp=.o)) \
           $(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
           $(foreach file,$(GENERATED_C_FILES),$(file:.c=.o))

ULTRA_O_FILES := $(foreach file,$(ULTRA_S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
                 $(foreach file,$(ULTRA_C_FILES),$(BUILD_DIR)/$(file:.c=.o))

GODDARD_O_FILES := $(foreach file,$(GODDARD_C_FILES),$(BUILD_DIR)/$(file:.c=.o))

# Automatic dependency files
DEP_FILES := $(O_FILES:.o=.d) $(ULTRA_O_FILES:.o=.d) $(GODDARD_O_FILES:.o=.d) $(BUILD_DIR)/$(LD_SCRIPT).d

# Segment elf files
SEG_FILES := $(SEGMENT_ELF_FILES) $(ACTOR_ELF_FILES) $(LEVEL_ELF_FILES)

##################### Compiler Options #######################
INCLUDE_CFLAGS := -I include -I $(BUILD_DIR) -I $(BUILD_DIR)/include -I src -I .
ENDIAN_BITWIDTH := $(BUILD_DIR)/endian-and-bitwidth

# Huge deleted N64 section was here

AS := $(CROSS)as

ifeq ($(OSX_BUILD),1)
AS := i686-w64-mingw32-as
endif

ifneq ($(TARGET_WEB),1) # As in, not-web PC port
  CC := $(CROSS)gcc
  CXX := $(CROSS)g++
else
  CC := emcc
endif

LD := $(CC)

ifeq ($(WINDOWS_BUILD),1)
  ifeq ($(CROSS),i686-w64-mingw32.static-) # fixes compilation in MXE on Linux and WSL
    LD := $(CC)
  else ifeq ($(CROSS),x86_64-w64-mingw32.static-)
    LD := $(CC)
  else
    LD := $(CXX)
  endif
endif

ifeq ($(WINDOWS_BUILD),1) # fixes compilation in MXE on Linux and WSL
  CPP := cpp -P
  OBJCOPY := objcopy
  OBJDUMP := $(CROSS)objdump
else ifeq ($(OSX_BUILD),1)
  CPP := cpp-9 -P
  OBJDUMP := i686-w64-mingw32-objdump
  OBJCOPY := i686-w64-mingw32-objcopy
else # Linux & other builds
  CPP := $(CROSS)cpp -P
  OBJCOPY := $(CROSS)objcopy
  OBJDUMP := $(CROSS)objdump
endif

PYTHON := python3
SDLCONFIG := $(CROSS)sdl2-config

ifeq ($(WINDOWS_BUILD),1)
CC_CHECK := $(CC) -fsyntax-only -fsigned-char $(INCLUDE_CFLAGS) -Wall -Wextra -Wno-format-security $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) `$(SDLCONFIG) --cflags` -DUSE_SDL=2
CFLAGS := $(OPT_FLAGS) $(INCLUDE_CFLAGS) $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -fno-strict-aliasing -fwrapv `$(SDLCONFIG) --cflags` -DUSE_SDL=2

else ifeq ($(TARGET_WEB),1)
CC_CHECK := $(CC) -fsyntax-only -fsigned-char $(INCLUDE_CFLAGS) -Wall -Wextra -Wno-format-security $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -s USE_SDL=2
CFLAGS := $(OPT_FLAGS) $(INCLUDE_CFLAGS) $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -fno-strict-aliasing -fwrapv -s USE_SDL=2

# Linux / Other builds below
else
CC_CHECK := $(CC) -fsyntax-only -fsigned-char $(INCLUDE_CFLAGS) -Wall -Wextra -Wno-format-security $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) `$(SDLCONFIG) --cflags` -DUSE_SDL=2
CFLAGS := $(OPT_FLAGS) $(INCLUDE_CFLAGS) $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -fno-strict-aliasing -fwrapv `$(SDLCONFIG) --cflags` -DUSE_SDL=2
endif

# Check for enhancement options

# Check for Puppycam option
ifeq ($(BETTERCAMERA),1)
  CC_CHECK += -DBETTERCAMERA
  CFLAGS += -DBETTERCAMERA
  EXT_OPTIONS_MENU := 1
endif

ifeq ($(TEXTSAVES),1)
  CC_CHECK += -DTEXTSAVES
  CFLAGS += -DTEXTSAVES
endif

# Check for no drawing distance option
ifeq ($(NODRAWINGDISTANCE),1)
  CC_CHECK += -DNODRAWINGDISTANCE
  CFLAGS += -DNODRAWINGDISTANCE
endif

# Check for texture fix option
ifeq ($(TEXTURE_FIX),1)
  CC_CHECK += -DTEXTURE_FIX
  CFLAGS += -DTEXTURE_FIX
endif

# Check for extended options menu option
ifeq ($(EXT_OPTIONS_MENU),1)
  CC_CHECK += -DEXT_OPTIONS_MENU
  CFLAGS += -DEXT_OPTIONS_MENU
endif

# Check for no bzero/bcopy workaround option
ifeq ($(NO_BZERO_BCOPY),1)
  CC_CHECK += -DNO_BZERO_BCOPY
  CFLAGS += -DNO_BZERO_BCOPY
endif

# Use internal ldiv()/lldiv()
ifeq ($(NO_LDIV),1)
  CC_CHECK += -DNO_LDIV
  CFLAGS += -DNO_LDIV
endif

# Use OpenGL 1.3
ifeq ($(LEGACY_GL),1)
  CC_CHECK += -DLEGACY_GL
  CFLAGS += -DLEGACY_GL
endif

# Load external textures
ifeq ($(EXTERNAL_DATA),1)
  CC_CHECK += -DEXTERNAL_DATA
  CFLAGS += -DEXTERNAL_DATA
  # tell skyconv to write names instead of actual texture data and save the split tiles so we can use them later
  SKYCONV_ARGS := --store-names --write-tiles "$(BUILD_DIR)/textures/skybox_tiles"
endif

ASFLAGS := -I include -I $(BUILD_DIR) $(VERSION_ASFLAGS)

ifeq ($(TARGET_WEB),1)
LDFLAGS := -lm -lGL -lSDL2 -no-pie -s TOTAL_MEMORY=20MB -g4 --source-map-base http://localhost:8080/ -s "EXTRA_EXPORTED_RUNTIME_METHODS=['callMain']"
else ifeq ($(WINDOWS_BUILD),1)
  LDFLAGS := $(BITS) -march=$(TARGET_ARCH) -Llib -lpthread -lglew32 `$(SDLCONFIG) --static-libs` -lm -lglu32 -lsetupapi -ldinput8 -luser32 -lgdi32 -limm32 -lole32 -loleaut32 -lshell32 -lwinmm -lversion -luuid -lopengl32 -static
  ifeq ($(CROSS),)
    LDFLAGS += -no-pie
  endif
  ifeq ($(WINDOWS_CONSOLE),1)
    LDFLAGS += -mconsole
  endif
else ifeq ($(TARGET_RPI),1)
# Linux / Other builds below
LDFLAGS := $(OPT_FLAGS) -lm -lGLESv2 `$(SDLCONFIG) --libs` -no-pie
else
ifeq ($(OSX_BUILD),1)
LDFLAGS := -lm -framework OpenGL `$(SDLCONFIG) --libs` -no-pie -lpthread `pkg-config --libs libusb-1.0 glfw3 glew`
else
LDFLAGS := $(BITS) -march=$(TARGET_ARCH) -lm -lGL `$(SDLCONFIG) --libs` -no-pie -lpthread
endif
endif # End of LDFLAGS

# Prevent a crash with -sopt
export LANG := C

####################### Other Tools #########################

# N64 conversion tools
TOOLS_DIR = tools
MIO0TOOL = $(TOOLS_DIR)/mio0
N64CKSUM = $(TOOLS_DIR)/n64cksum
N64GRAPHICS = $(TOOLS_DIR)/n64graphics
N64GRAPHICS_CI = $(TOOLS_DIR)/n64graphics_ci
TEXTCONV = $(TOOLS_DIR)/textconv
IPLFONTUTIL = $(TOOLS_DIR)/iplfontutil
AIFF_EXTRACT_CODEBOOK = $(TOOLS_DIR)/aiff_extract_codebook
VADPCM_ENC = $(TOOLS_DIR)/vadpcm_enc
EXTRACT_DATA_FOR_MIO = $(TOOLS_DIR)/extract_data_for_mio
SKYCONV = $(TOOLS_DIR)/skyconv
EMULATOR = mupen64plus
EMU_FLAGS = --noosd
LOADER = loader64
LOADER_FLAGS = -vwf
SHA1SUM = sha1sum
ZEROTERM = $(PYTHON) $(TOOLS_DIR)/zeroterm.py

###################### Dependency Check #####################

# Stubbed

######################## Targets #############################

all: $(EXE)

ifeq ($(EXTERNAL_DATA),1)

# thank you apple very cool
ifeq ($(HOST_OS),Darwin)
  CP := gcp
else
  CP := cp
endif

# depend on resources as well
all: res

# prepares the resource folder for external data
res: $(EXE)
	@mkdir -p $(BUILD_DIR)/res/sound
	@$(CP) -r -f textures/ $(BUILD_DIR)/res/
	@$(CP) -r -f $(BUILD_DIR)/textures/skybox_tiles/ $(BUILD_DIR)/res/textures/
	@$(CP) -f $(SOUND_BIN_DIR)/sound_data.ctl $(BUILD_DIR)/res/sound/
	@$(CP) -f $(SOUND_BIN_DIR)/sound_data.tbl $(BUILD_DIR)/res/sound/
	@$(CP) -f $(SOUND_BIN_DIR)/sequences.bin $(BUILD_DIR)/res/sound/
	@$(CP) -f $(SOUND_BIN_DIR)/bank_sets $(BUILD_DIR)/res/sound/
	@find actors -name \*.png -exec $(CP) --parents {} $(BUILD_DIR)/res/ \;
	@find levels -name \*.png -exec $(CP) --parents {} $(BUILD_DIR)/res/ \;

endif

clean:
	$(RM) -r $(BUILD_DIR_BASE)

cleantools:
	$(MAKE) -s -C tools clean

distclean:
	$(RM) -r $(BUILD_DIR_BASE)
	./extract_assets.py --clean

test: $(ROM)
	$(EMULATOR) $(EMU_FLAGS) $<

load: $(ROM)
	$(LOADER) $(LOADER_FLAGS) $<

libultra: $(BUILD_DIR)/libultra.a

asm/boot.s: $(BUILD_DIR)/lib/bin/ipl3_font.bin

$(BUILD_DIR)/lib/bin/ipl3_font.bin: lib/ipl3_font.png
	$(IPLFONTUTIL) e $< $@

#Required so the compiler doesn't complain about this not existing.
$(BUILD_DIR)/src/game/camera.o: $(BUILD_DIR)/include/text_strings.h

$(BUILD_DIR)/include/text_strings.h: include/text_strings.h.in
	$(TEXTCONV) charmap.txt $< $@

$(BUILD_DIR)/include/text_menu_strings.h: include/text_menu_strings.h.in
	$(TEXTCONV) charmap_menu.txt $< $@

$(BUILD_DIR)/include/text_options_strings.h: include/text_options_strings.h.in
	$(TEXTCONV) charmap.txt $< $@

ifeq ($(VERSION),eu)
TEXT_DIRS := text/de text/us text/fr

# EU encoded text inserted into individual segment 0x19 files,
# and course data also duplicated in leveldata.c
$(BUILD_DIR)/bin/eu/translation_en.o: $(BUILD_DIR)/text/us/define_text.inc.c
$(BUILD_DIR)/bin/eu/translation_de.o: $(BUILD_DIR)/text/de/define_text.inc.c
$(BUILD_DIR)/bin/eu/translation_fr.o: $(BUILD_DIR)/text/fr/define_text.inc.c
$(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/us/define_courses.inc.c
$(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/de/define_courses.inc.c
$(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/fr/define_courses.inc.c

else
ifeq ($(VERSION),sh)
TEXT_DIRS := text/jp
$(BUILD_DIR)/bin/segment2.o: $(BUILD_DIR)/text/jp/define_text.inc.c

else
TEXT_DIRS := text/$(VERSION)

# non-EU encoded text inserted into segment 0x02
$(BUILD_DIR)/bin/segment2.o: $(BUILD_DIR)/text/$(VERSION)/define_text.inc.c
endif
endif

$(BUILD_DIR)/text/%/define_courses.inc.c: text/define_courses.inc.c text/%/courses.h
	$(CPP) $(VERSION_CFLAGS) $< -o $@ -I text/$*/
	$(TEXTCONV) charmap.txt $@ $@

$(BUILD_DIR)/text/%/define_text.inc.c: text/define_text.inc.c text/%/courses.h text/%/dialogs.h
	$(CPP) $(VERSION_CFLAGS) $< -o $@ -I text/$*/
	$(TEXTCONV) charmap.txt $@ $@

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS) $(ASM_DIRS) $(GODDARD_SRC_DIRS) $(ULTRA_SRC_DIRS) $(ULTRA_ASM_DIRS) $(ULTRA_BIN_DIRS) $(BIN_DIRS) $(TEXTURE_DIRS) $(TEXT_DIRS) $(SOUND_SAMPLE_DIRS) $(addprefix levels/,$(LEVEL_DIRS)) include) $(MIO0_DIR) $(addprefix $(MIO0_DIR)/,$(VERSION)) $(SOUND_BIN_DIR) $(SOUND_BIN_DIR)/sequences/$(VERSION)

# Make sure build directory exists before compiling anything
DUMMY != mkdir -p $(ALL_DIRS)

$(BUILD_DIR)/include/text_strings.h: $(BUILD_DIR)/include/text_menu_strings.h
$(BUILD_DIR)/include/text_strings.h: $(BUILD_DIR)/include/text_options_strings.h

ifeq ($(VERSION),eu)
$(BUILD_DIR)/src/menu/file_select.o: $(BUILD_DIR)/include/text_strings.h $(BUILD_DIR)/bin/eu/translation_en.o $(BUILD_DIR)/bin/eu/translation_de.o $(BUILD_DIR)/bin/eu/translation_fr.o
$(BUILD_DIR)/src/menu/star_select.o: $(BUILD_DIR)/include/text_strings.h $(BUILD_DIR)/bin/eu/translation_en.o $(BUILD_DIR)/bin/eu/translation_de.o $(BUILD_DIR)/bin/eu/translation_fr.o
$(BUILD_DIR)/src/game/ingame_menu.o: $(BUILD_DIR)/include/text_strings.h $(BUILD_DIR)/bin/eu/translation_en.o $(BUILD_DIR)/bin/eu/translation_de.o $(BUILD_DIR)/bin/eu/translation_fr.o
$(BUILD_DIR)/src/game/options_menu.o: $(BUILD_DIR)/include/text_strings.h $(BUILD_DIR)/bin/eu/translation_en.o $(BUILD_DIR)/bin/eu/translation_de.o $(BUILD_DIR)/bin/eu/translation_fr.o
O_FILES += $(BUILD_DIR)/bin/eu/translation_en.o $(BUILD_DIR)/bin/eu/translation_de.o $(BUILD_DIR)/bin/eu/translation_fr.o
else
$(BUILD_DIR)/src/menu/file_select.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/menu/star_select.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/game/ingame_menu.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/game/options_menu.o: $(BUILD_DIR)/include/text_strings.h
endif

################################################################
# TEXTURE GENERATION                                           #
################################################################

# RGBA32, RGBA16, IA16, IA8, IA4, IA1, I8, I4
ifeq ($(EXTERNAL_DATA),1)
$(BUILD_DIR)/%: %.png
	$(ZEROTERM) "$(patsubst %.png,%,$^)" > $@
else
$(BUILD_DIR)/%: %.png
	$(N64GRAPHICS) -i $@ -g $< -f $(lastword $(subst ., ,$@))
endif

$(BUILD_DIR)/%.inc.c: $(BUILD_DIR)/% %.png
	hexdump -v -e '1/1 "0x%X,"' $< > $@
	echo >> $@

ifeq ($(EXTERNAL_DATA),0)
# Color Index CI8
$(BUILD_DIR)/%.ci8: %.ci8.png
	$(N64GRAPHICS_CI) -i $@ -g $< -f ci8

# Color Index CI4
$(BUILD_DIR)/%.ci4: %.ci4.png
	$(N64GRAPHICS_CI) -i $@ -g $< -f ci4
endif

################################################################

# compressed segment generation

# PC Area
$(BUILD_DIR)/%.table: %.aiff
	$(AIFF_EXTRACT_CODEBOOK) $< >$@

$(BUILD_DIR)/%.aifc: $(BUILD_DIR)/%.table %.aiff
	$(VADPCM_ENC) -c $^ $@

$(ENDIAN_BITWIDTH): tools/determine-endian-bitwidth.c
	$(CC) -c $(CFLAGS) -o $@.dummy2 $< 2>$@.dummy1; true
	grep -o 'msgbegin --endian .* --bitwidth .* msgend' $@.dummy1 > $@.dummy2
	head -n1 <$@.dummy2 | cut -d' ' -f2-5 > $@
	@rm $@.dummy1
	@rm $@.dummy2

$(SOUND_BIN_DIR)/sound_data.ctl: sound/sound_banks/ $(SOUND_BANK_FILES) $(SOUND_SAMPLE_AIFCS) $(ENDIAN_BITWIDTH)
	$(PYTHON) tools/assemble_sound.py $(BUILD_DIR)/sound/samples/ sound/sound_banks/ $(SOUND_BIN_DIR)/sound_data.ctl $(SOUND_BIN_DIR)/sound_data.tbl $(VERSION_CFLAGS) $$(cat $(ENDIAN_BITWIDTH))

$(SOUND_BIN_DIR)/sound_data.tbl: $(SOUND_BIN_DIR)/sound_data.ctl
	@true

ifeq ($(VERSION),sh)
$(SOUND_BIN_DIR)/sequences.bin: $(SOUND_BANK_FILES) sound/sequences.json sound/sequences/ sound/sequences/jp/ $(SOUND_SEQUENCE_FILES) $(ENDIAN_BITWIDTH)
	$(PYTHON) tools/assemble_sound.py --sequences $@ $(SOUND_BIN_DIR)/bank_sets sound/sound_banks/ sound/sequences.json $(SOUND_SEQUENCE_FILES) $(VERSION_CFLAGS) $$(cat $(ENDIAN_BITWIDTH))
else
$(SOUND_BIN_DIR)/sequences.bin: $(SOUND_BANK_FILES) sound/sequences.json sound/sequences/ sound/sequences/$(VERSION)/ $(SOUND_SEQUENCE_FILES) $(ENDIAN_BITWIDTH)
	$(PYTHON) tools/assemble_sound.py --sequences $@ $(SOUND_BIN_DIR)/bank_sets sound/sound_banks/ sound/sequences.json $(SOUND_SEQUENCE_FILES) $(VERSION_CFLAGS) $$(cat $(ENDIAN_BITWIDTH))
endif

$(SOUND_BIN_DIR)/bank_sets: $(SOUND_BIN_DIR)/sequences.bin
	@true

$(SOUND_BIN_DIR)/%.m64: $(SOUND_BIN_DIR)/%.o
	$(OBJCOPY) -j .rodata $< -O binary $@

$(SOUND_BIN_DIR)/%.o: $(SOUND_BIN_DIR)/%.s
	$(AS) $(ASFLAGS) -o $@ $<

ifeq ($(EXTERNAL_DATA),1)

$(SOUND_BIN_DIR)/sound_data.ctl.c: $(SOUND_BIN_DIR)/sound_data.ctl
	echo "unsigned char gSoundDataADSR[] = \"sound/sound_data.ctl\";" > $@
$(SOUND_BIN_DIR)/sound_data.tbl.c: $(SOUND_BIN_DIR)/sound_data.tbl
	echo "unsigned char gSoundDataRaw[] = \"sound/sound_data.tbl\";" > $@
$(SOUND_BIN_DIR)/sequences.bin.c: $(SOUND_BIN_DIR)/sequences.bin
	echo "unsigned char gMusicData[] = \"sound/sequences.bin\";" > $@
$(SOUND_BIN_DIR)/bank_sets.c: $(SOUND_BIN_DIR)/bank_sets
	echo "unsigned char gBankSetsData[] = \"sound/bank_sets\";" > $@

else

$(SOUND_BIN_DIR)/sound_data.ctl.c: $(SOUND_BIN_DIR)/sound_data.ctl
	echo "unsigned char gSoundDataADSR[] = {" > $@
	hexdump -v -e '1/1 "0x%X,"' $< >> $@
	echo "};" >> $@

$(SOUND_BIN_DIR)/sound_data.tbl.c: $(SOUND_BIN_DIR)/sound_data.tbl
	echo "unsigned char gSoundDataRaw[] = {" > $@
	hexdump -v -e '1/1 "0x%X,"' $< >> $@
	echo "};" >> $@

$(SOUND_BIN_DIR)/sequences.bin.c: $(SOUND_BIN_DIR)/sequences.bin
	echo "unsigned char gMusicData[] = {" > $@
	hexdump -v -e '1/1 "0x%X,"' $< >> $@
	echo "};" >> $@

$(SOUND_BIN_DIR)/bank_sets.c: $(SOUND_BIN_DIR)/bank_sets
	echo "unsigned char gBankSetsData[0x100] = {" > $@
	hexdump -v -e '1/1 "0x%X,"' $< >> $@
	echo "};" >> $@

endif

$(BUILD_DIR)/levels/scripts.o: $(BUILD_DIR)/include/level_headers.h

$(BUILD_DIR)/include/level_headers.h: levels/level_headers.h.in
	$(CPP) -I . levels/level_headers.h.in | $(PYTHON) tools/output_level_headers.py > $(BUILD_DIR)/include/level_headers.h

$(BUILD_DIR)/assets/mario_anim_data.c: $(wildcard assets/anims/*.inc.c)
	$(PYTHON) tools/mario_anims_converter.py > $@

$(BUILD_DIR)/assets/demo_data.c: assets/demo_data.json $(wildcard assets/demos/*.bin)
	$(PYTHON) tools/demo_data_converter.py assets/demo_data.json $(VERSION_CFLAGS) > $@

# Source code
$(BUILD_DIR)/levels/%/leveldata.o: OPT_FLAGS := -g
$(BUILD_DIR)/actors/%.o: OPT_FLAGS := -g
$(BUILD_DIR)/bin/%.o: OPT_FLAGS := -g
$(BUILD_DIR)/src/goddard/%.o: OPT_FLAGS := -g
$(BUILD_DIR)/src/goddard/%.o: MIPSISET := -mips1
$(BUILD_DIR)/src/audio/%.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/src/audio/load.o: OPT_FLAGS := -O2 -framepointer -Wo,-loopunroll,0
$(BUILD_DIR)/lib/src/%.o: OPT_FLAGS :=
$(BUILD_DIR)/lib/src/math/ll%.o: MIPSISET := -mips3 -32
$(BUILD_DIR)/lib/src/math/%.o: OPT_FLAGS := -O2
$(BUILD_DIR)/lib/src/math/ll%.o: OPT_FLAGS :=
$(BUILD_DIR)/lib/src/ldiv.o: OPT_FLAGS := -O2
$(BUILD_DIR)/lib/src/string.o: OPT_FLAGS := -O2
$(BUILD_DIR)/lib/src/gu%.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/al%.o: OPT_FLAGS := -O3

ifeq ($(VERSION),eu)
$(BUILD_DIR)/lib/src/_Litob.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/_Ldtob.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/_Printf.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/sprintf.o: OPT_FLAGS := -O3

# enable loop unrolling except for external.c (external.c might also have used
# unrolling, but it makes one loop harder to match)
$(BUILD_DIR)/src/audio/%.o: OPT_FLAGS := -O2
$(BUILD_DIR)/src/audio/load.o: OPT_FLAGS := -O2
$(BUILD_DIR)/src/audio/external.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
else

# The source-to-source optimizer copt is enabled for audio. This makes it use
# acpp, which needs -Wp,-+ to handle C++-style comments.
$(BUILD_DIR)/src/audio/effects.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0 -sopt,-inline=sequence_channel_process_sound,-scalaroptimize=1 -Wp,-+
$(BUILD_DIR)/src/audio/synthesis.o: OPT_FLAGS := -O2 -sopt,-scalaroptimize=1 -Wp,-+

# Add a target for build/eu/src/audio/*.copt to make it easier to see debug
$(BUILD_DIR)/src/audio/%.acpp: src/audio/%.c
	$(QEMU_IRIX) -silent -L $(IRIX_ROOT) $(IRIX_ROOT)/usr/lib/acpp $(TARGET_CFLAGS) $(INCLUDE_CFLAGS) $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -D__sgi -+ $< > $@ 
$(BUILD_DIR)/src/audio/%.copt: $(BUILD_DIR)/src/audio/%.acpp
	$(QEMU_IRIX) -silent -L $(IRIX_ROOT) $(IRIX_ROOT)/usr/lib/copt -signed -I=$< -CMP=$@ -cp=i -scalaroptimize=1
endif

# Rebuild files with 'GLOBAL_ASM' if the NON_MATCHING flag changes.
$(GLOBAL_ASM_O_FILES): $(GLOBAL_ASM_DEP).$(NON_MATCHING)
$(GLOBAL_ASM_DEP).$(NON_MATCHING):
	@rm -f $(GLOBAL_ASM_DEP).*
	touch $@

$(BUILD_DIR)/%.o: %.cpp
	@$(CXX) -fsyntax-only $(CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CXX) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c
	@$(CC_CHECK) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<


$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	@$(CC_CHECK) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.s
	$(AS) $(ASFLAGS) -MD $(BUILD_DIR)/$*.d -o $@ $<



$(EXE): $(O_FILES) $(MIO0_FILES:.mio0=.o) $(SOUND_OBJ_FILES) $(ULTRA_O_FILES) $(GODDARD_O_FILES)
	$(LD) -L $(BUILD_DIR) -o $@ $(O_FILES) $(SOUND_OBJ_FILES) $(ULTRA_O_FILES) $(GODDARD_O_FILES) $(LDFLAGS)

.PHONY: all clean distclean default diff test load libultra res
.PRECIOUS: $(BUILD_DIR)/bin/%.elf $(SOUND_BIN_DIR)/%.ctl $(SOUND_BIN_DIR)/%.tbl $(SOUND_SAMPLE_TABLES) $(SOUND_BIN_DIR)/%.s $(BUILD_DIR)/%
.DELETE_ON_ERROR:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
