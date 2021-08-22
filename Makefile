################################################################################
############################### SM64PC Makefile ################################
################################################################################

## Default target ##
default: all

################################ Build Options #################################

## These options can either be changed by modifying the makefile, or
## by building with 'make SETTING=value'. 'make clean' may be required.

# Build debug version
DEBUG ?= 0
# Version of the game to build
VERSION ?= us
# Graphics microcode used
GRUCODE ?= f3dex2e
# If NON_MATCHING is 1, define the NON_MATCHING and AVOID_UB macros when building (recommended)
NON_MATCHING ?= 1
# Build and optimize for Raspberry Pi(s)
TARGET_RPI ?= 0
# Build for Emscripten/WebGL
TARGET_WEB ?= 0

# Build for Nintendo Switch
TARGET_SWITCH ?= 0

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
# Enable Discord Rich Presence
DISCORDRPC ?= 1
# Various workarounds for weird toolchains
NO_BZERO_BCOPY ?= 0
NO_LDIV ?= 0

## Backend selection

# Renderers: GL, GL_LEGACY, D3D11, D3D12
RENDER_API ?= GL
# Window managers: SDL2, DXGI (forced if D3D11 or D3D12 in RENDER_API)
WINDOW_API ?= SDL2
# Audio backends: SDL2
AUDIO_API ?= SDL2
# Controller backends (can have multiple, space separated): SDL2
CONTROLLER_API ?= SDL2

## External assets
NO_COPY ?= 0

################################# OS Detection #################################

WINDOWS_BUILD ?= 0

ifeq ($(OS),Windows_NT)
  HOST_OS ?= Windows
else
  HOST_OS ?= $(shell uname -s 2>/dev/null || echo Unknown)
  # some weird MINGW/Cygwin env that doesn't define $OS
  ifneq (,$(findstring MINGW,HOST_OS))
    HOST_OS := Windows
  endif
endif

ifeq ($(TARGET_WEB)$(TARGET_RPI)$(TARGET_SWITCH),000)
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

# macOS overrides
ifeq ($(HOST_OS),Darwin)
  OSX_BUILD := 1
  # Using MacPorts?
  ifeq ($(shell test -d /opt/local/lib && echo y),y)
    OSX_GCC_VER = $(shell find /opt/local/bin/gcc* | grep -oE '[[:digit:]]+' | sort -n | uniq | tail -1)
    CC := gcc-mp-$(OSX_GCC_VER)
    CXX := g++-mp-$(OSX_GCC_VER)
    CPP := cpp-mp-$(OSX_GCC_VER) -P
    PLATFORM_CFLAGS := -I /opt/local/include
    PLATFORM_LDFLAGS := -L /opt/local/lib
  else
    # Using Homebrew?
    ifeq ($(shell which brew >/dev/null 2>&1 && echo y),y)
      OSX_GCC_VER = $(shell find `brew --prefix`/bin/gcc* | grep -oE '[[:digit:]]+' | sort -n | uniq | tail -1)
      CC := gcc-$(OSX_GCC_VER)
      CXX := g++-$(OSX_GCC_VER)
      CPP := cpp-$(OSX_GCC_VER) -P
      PLATFORM_CFLAGS := -I /usr/local/include
      PLATFORM_LDFLAGS := -L /usr/local/lib
    else
      $(error No suitable macOS toolchain found, have you installed Homebrew?)
    endif
  endif
endif

ifneq ($(TARGET_BITS),0)
  BITS := -m$(TARGET_BITS)
endif

# Release (version) flag defs
VERSION_DEF := VERSION_US

TARGET := moon64.$(VERSION)
VERSION_CFLAGS := -D$(VERSION_DEF) -D_LANGUAGE_C
VERSION_ASFLAGS := --defsym $(VERSION_DEF)=1

# Stuff for showing the git hash in the title bar
GIT_HASH := $(shell git rev-parse --short HEAD)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
VERSION_CFLAGS += -DGIT_HASH="\"$(GIT_HASH)\""

ifeq ($(shell git rev-parse --abbrev-ref HEAD),nightly)
  VERSION_CFLAGS += -DNIGHTLY
endif

GRUCODE_DEF := F3DEX_GBI_2E
TARGET := $(TARGET).f3dex2e

GRUCODE_CFLAGS := -D$(GRUCODE_DEF)
GRUCODE_ASFLAGS := $(GRUCODE_ASFLAGS) --defsym $(GRUCODE_DEF)=1

# Default build is for PC now
VERSION_CFLAGS := $(VERSION_CFLAGS) -DNON_MATCHING -DAVOID_UB

ifeq ($(TARGET_RPI),1) # Define RPi to change SDL2 title & GLES2 hints
      VERSION_CFLAGS += -DUSE_GLES
endif

ifeq ($(TARGET_SWITCH),1)
      VERSION_CFLAGS += -DTARGET_SWITCH -DLUA_USE_LINUX
endif
ifeq ($(OSX_BUILD),1) # Modify GFX & SDL2 for OSX GL
  VERSION_CFLAGS += -DOSX_BUILD
endif

VERSION_ASFLAGS := --defsym AVOID_UB=1

ifeq ($(TARGET_WEB),1)
  VERSION_CFLAGS := $(VERSION_CFLAGS) -DTARGET_WEB -DUSE_GLES
endif

# Check backends

ifneq (,$(filter $(RENDER_API),D3D11 D3D12))
  ifneq ($(WINDOWS_BUILD),1)
    $(error DirectX is only supported on Windows)
  endif
  ifneq ($(WINDOW_API),DXGI)
    $(warning DirectX renderers require DXGI, forcing WINDOW_API value)
    WINDOW_API := DXGI
  endif
else
  ifeq ($(WINDOW_API),DXGI)
    $(error DXGI can only be used with DirectX renderers)
  endif
endif

############################ Universal Dependencies ############################

# (This is a bit hacky, but a lot of rules implicitly depend
# on tools and assets, and we use directory globs further down
# in the makefile that we want should cover assets.)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleantools)
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
DUMMY != CC=$(CC) CXX=$(CXX) $(MAKE) -C tools >&2 || echo FAIL
ifeq ($(DUMMY),FAIL)
  $(error Failed to build tools)
endif

endif
endif
endif

######################### Target Executable and Sources ########################

# BUILD_DIR is location where all build artifacts are placed
BUILD_DIR_BASE := build

ifeq ($(TARGET_WEB),1)
  BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)_web
else ifeq ($(TARGET_SWITCH),1)
  BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)_nx
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
SRC_DIRS := src src/engine src/game src/audio src/menu src/buffers actors levels bin data assets src/text src/text/libs src/pc src/pc/gfx src/pc/audio src/pc/controller src/pc/fs src/pc/fs/packtypes src/nx

ifeq ($(WINDOWS_BUILD),1)
  VERSION_CFLAGS += -DDISABLE_CURL_SUPPORT
endif

################################
#      Moon64 Source Code      #
################################

MOON_SRC := $(shell find src/moon/ -type d)
SRC_DIRS += $(MOON_SRC)

################################

ifeq ($(DISCORDRPC),1)
  ifneq ($(TARGET_SWITCH)$(TARGET_WEB)$(TARGET_RPI),000)
    $(echo Discord RPC does not work on this target)
    DISCORDRPC := 0
  else
    SRC_DIRS += src/pc/discord
  endif
endif

BIN_DIRS := bin bin/$(VERSION)

ULTRA_SRC_DIRS := lib/src lib/src/math
ULTRA_ASM_DIRS := lib/asm lib/data
ULTRA_BIN_DIRS := lib/bin

GODDARD_SRC_DIRS := src/goddard src/goddard/dynlists

MIPSISET := -mips2
MIPSBIT := -32

ifeq ($(DEBUG),1)
  OPT_FLAGS := -g
else
  OPT_FLAGS := -O2
endif

# Set BITS (32/64) to compile for
OPT_FLAGS += $(BITS)

ifeq ($(TARGET_WEB),1)
  OPT_FLAGS := -O2 -g4 --source-map-base http://localhost:8080/
endif

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

# Source code files
LEVEL_C_FILES := $(wildcard levels/*/leveldata.c) $(wildcard levels/*/script.c) $(wildcard levels/*/geo.c)
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) $(LEVEL_C_FILES)
CXX_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
S_FILES := $(foreach dir,$(ASM_DIRS),$(wildcard $(dir)/*.s))
GODDARD_C_FILES := $(foreach dir,$(GODDARD_SRC_DIRS),$(wildcard $(dir)/*.c))

GENERATED_C_FILES := $(BUILD_DIR)/assets/mario_anim_data.c $(BUILD_DIR)/assets/demo_data.c

ULTRA_C_FILES := \
  alBnkfNew.c \
  guLookAtRef.c \
  guMtxF2L.c \
  guNormalize.c \
  guOrthoF.c \
  guPerspectiveF.c \
  guRotateF.c \
  guScaleF.c \
  guTranslateF.c \
  ldiv.c

C_FILES := $(filter-out src/game/main.c,$(C_FILES))
ULTRA_C_FILES := $(addprefix lib/src/,$(ULTRA_C_FILES))

# "If we're not N64, use the above"

SOUND_BANK_FILES := $(wildcard sound/sound_banks/*.json)
SOUND_SEQUENCE_FILES := $(wildcard sound/sequences/$(VERSION)/*.m64) \
    $(wildcard sound/sequences/*.m64) \
    $(foreach file,$(wildcard sound/sequences/$(VERSION)/*.s),$(BUILD_DIR)/$(file:.s=.m64)) \
    $(foreach file,$(wildcard sound/sequences/*.s),$(BUILD_DIR)/$(file:.s=.m64))

SOUND_SAMPLE_DIRS := $(wildcard sound/samples/*)
SOUND_SAMPLE_AIFFS := $(foreach dir,$(SOUND_SAMPLE_DIRS),$(wildcard $(dir)/*.aiff))
SOUND_SAMPLE_TABLES := $(foreach file,$(SOUND_SAMPLE_AIFFS),$(BUILD_DIR)/$(file:.aiff=.table))
SOUND_SAMPLE_AIFCS := $(foreach file,$(SOUND_SAMPLE_AIFFS),$(BUILD_DIR)/$(file:.aiff=.aifc))
SOUND_OBJ_FILES := $(SOUND_BIN_DIR)/sound_data.o

# Object files
O_FILES :=  $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
            $(foreach file,$(CXX_FILES),$(BUILD_DIR)/$(file:.cpp=.o)) \
            $(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
            $(foreach file,$(GENERATED_C_FILES),$(file:.c=.o))

ULTRA_O_FILES :=  $(foreach file,$(ULTRA_S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
                  $(foreach file,$(ULTRA_C_FILES),$(BUILD_DIR)/$(file:.c=.o))

GODDARD_O_FILES := $(foreach file,$(GODDARD_C_FILES),$(BUILD_DIR)/$(file:.c=.o))

RPC_LIBS :=
ifeq ($(DISCORDRPC),1)
  ifeq ($(WINDOWS_BUILD),1)
    RPC_LIBS := lib/discord/libdiscord-rpc.dll
  else ifeq ($(OSX_BUILD),1)
    # needs testing
    RPC_LIBS := lib/discord/libdiscord-rpc.dylib
  else
    RPC_LIBS := lib/discord/libdiscord-rpc.so
  endif
endif

# Automatic dependency files
DEP_FILES := $(O_FILES:.o=.d) $(ULTRA_O_FILES:.o=.d) $(GODDARD_O_FILES:.o=.d) $(BUILD_DIR)/$(LD_SCRIPT).d

# Segment elf files
SEG_FILES := $(SEGMENT_ELF_FILES) $(ACTOR_ELF_FILES) $(LEVEL_ELF_FILES)

############################ Compiler Options ##################################

INCLUDE_CFLAGS := -I include -I $(BUILD_DIR) -I $(BUILD_DIR)/include -I src -I .
ENDIAN_BITWIDTH := $(BUILD_DIR)/endian-and-bitwidth

# Huge deleted N64 section was here

ifeq ($(TARGET_SWITCH),1)
  ifeq ($(strip $(DEVKITPRO)),)
    $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
  endif
  export PATH := $(DEVKITPRO)/devkitA64/bin:$(PATH)
  include $(DEVKITPRO)/devkitA64/base_tools
  NXPATH := $(DEVKITPRO)/portlibs/switch/bin
  PORTLIBS ?= $(DEVKITPRO)/portlibs/switch
  LIBNX ?= $(DEVKITPRO)/libnx
  CROSS ?= aarch64-none-elf-
  SDLCROSS := $(NXPATH)/
  CC := $(CROSS)gcc
  CXX := $(CROSS)g++
  STRIP := $(CROSS)strip
  NXARCH	:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
  APP_TITLE := Moon64 - [$(GIT_BRANCH)]
  APP_AUTHOR := Nintendo, n64decomp team, UnderVolt team
  APP_VERSION := $(GIT_HASH)
  APP_ICON := $(CURDIR)/textures/logo/moon64-logo.jpg
  INCLUDE_CFLAGS += -I$(LIBNX)/include -I$(PORTLIBS)/include
  OPT_FLAGS := -O2
  LIBDIRS	:= $(PORTLIBS) $(LIBNX)
  export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
endif

# for some reason sdl-config in dka64 is not prefixed, while pkg-config is
SDLCROSS ?=

AS := $(CROSS)as

ifeq ($(OSX_BUILD),1)
AS := i686-w64-mingw32-as
endif

ifneq ($(TARGET_WEB),1) # As in, not-web PC port
  CC ?= $(CROSS)gcc
  CXX ?= $(CROSS)g++
else
  CC := emcc
  CXX := emcc
endif

LD := $(CC)

ifeq ($(DISCORDRPC),1)
  LD := $(CXX)
else ifeq ($(WINDOWS_BUILD),1)
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
  OBJDUMP := i686-w64-mingw32-objdump
  OBJCOPY := i686-w64-mingw32-objcopy
else # Linux & other builds
  CPP := $(CROSS)cpp -P
  OBJCOPY := $(CROSS)objcopy
  OBJDUMP := $(CROSS)objdump
endif

PYTHON := python3
SDLCONFIG := $(SDLCROSS)sdl2-config

# configure backend flags

BACKEND_CFLAGS := -DRAPI_$(RENDER_API)=1 -DWAPI_$(WINDOW_API)=1 -DAAPI_$(AUDIO_API)=1
# can have multiple controller APIs
BACKEND_CFLAGS += $(foreach capi,$(CONTROLLER_API),-DCAPI_$(capi)=1)
BACKEND_LDFLAGS :=
SDL2_USED := 0

# for now, it's either SDL+GL or DXGI+DirectX, so choose based on WAPI
ifeq ($(WINDOW_API),DXGI)
  DXBITS := `cat $(ENDIAN_BITWIDTH) | tr ' ' '\n' | tail -1`
  ifeq ($(RENDER_API),D3D12)
    BACKEND_CFLAGS += -Iinclude/dxsdk
  endif
  BACKEND_LDFLAGS += -ld3dcompiler -ldxgi -ldxguid
  BACKEND_LDFLAGS += -lsetupapi -ldinput8 -luser32 -lgdi32 -limm32 -lole32 -loleaut32 -lshell32 -lwinmm -lversion -luuid -static
else ifeq ($(WINDOW_API),SDL2)
  ifeq ($(WINDOWS_BUILD),1)
    BACKEND_LDFLAGS += -lglew32 -lglu32 -lopengl32
  else ifneq ($(TARGET_RPI)$(TARGET_SWITCH),00)
    BACKEND_LDFLAGS += -lglad -lglapi -ldrm_nouveau -lm
  else ifeq ($(OSX_BUILD),1)
    BACKEND_LDFLAGS += -framework OpenGL $(shell pkg-config --libs glew)
  else
    BACKEND_LDFLAGS += -lGL
  endif
  SDL_USED := 2
endif

ifeq ($(AUDIO_API),SDL2)
  SDL_USED := 2
endif

ifneq (,$(findstring SDL,$(CONTROLLER_API)))
  SDL_USED := 2
endif

# SDL can be used by different systems, so we consolidate all of that shit into this
ifeq ($(SDL_USED),2)
  BACKEND_CFLAGS += -DHAVE_SDL2=1 $(shell $(SDLCONFIG) --cflags)
  ifeq ($(WINDOWS_BUILD),1)
    BACKEND_LDFLAGS += $(shell $(SDLCONFIG) --static-libs) -lsetupapi -luser32 -limm32 -lole32 -loleaut32 -lshell32 -lwinmm -lversion
  else
    BACKEND_LDFLAGS += $(shell $(SDLCONFIG) --libs)
  endif
endif

ifeq ($(WINDOWS_BUILD),1)
  CC_CHECK := $(CC) -fsyntax-only -fsigned-char $(BACKEND_CFLAGS) $(INCLUDE_CFLAGS) -Wall -Wextra -Wno-format-security $(VERSION_CFLAGS) $(GRUCODE_CFLAGS)
  CFLAGS := $(OPT_FLAGS) $(INCLUDE_CFLAGS) $(BACKEND_CFLAGS) $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -fno-strict-aliasing -fwrapv

else ifeq ($(TARGET_WEB),1)
  CC_CHECK := $(CC) -fsyntax-only -fsigned-char $(BACKEND_CFLAGS) $(INCLUDE_CFLAGS) -Wall -Wextra -Wno-format-security $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -s USE_SDL=2
  CFLAGS := $(OPT_FLAGS) $(INCLUDE_CFLAGS) $(BACKEND_CFLAGS) $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -fno-strict-aliasing -fwrapv -s USE_SDL=2

else ifeq ($(TARGET_SWITCH),1)
  CC_CHECK := $(CC) $(NXARCH) -fsyntax-only -fsigned-char $(BACKEND_CFLAGS) $(INCLUDE_CFLAGS) -Wall -Wextra -Wno-format-security $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -D__SWITCH__=1
  CFLAGS := $(NXARCH) $(OPT_FLAGS) $(INCLUDE_CFLAGS) $(BACKEND_CFLAGS) $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -fno-strict-aliasing -ftls-model=local-exec -fPIC -fwrapv -D__SWITCH__=1

# Linux / Other builds below
else
  CC_CHECK := $(CC) -fsyntax-only -fsigned-char $(BACKEND_CFLAGS) $(PLATFORM_CFLAGS) $(INCLUDE_CFLAGS) -Wall -Wextra -Wno-format-security $(VERSION_CFLAGS) $(GRUCODE_CFLAGS)
  CFLAGS := $(OPT_FLAGS) $(PLATFORM_CFLAGS) $(INCLUDE_CFLAGS) $(BACKEND_CFLAGS) $(VERSION_CFLAGS) $(GRUCODE_CFLAGS) -fno-strict-aliasing -fwrapv

endif

################################
#     Moon64 Custom Flags      #
################################

# Moon64 Enable filesystem library and C++17
CXXFLAGS := -std=c++17
LDFLAGS += -lstdc++fs

CC_CHECK += -DGIT_HASH=\"$(GIT_HASH)\"
CFLAGS   += -DGIT_HASH=\"$(GIT_HASH)\"

CC_CHECK += -DGIT_BRANCH=\"$(GIT_BRANCH)\"
CFLAGS   += -DGIT_BRANCH=\"$(GIT_BRANCH)\"

ifeq ($(WINDOWS_BUILD),1)
  CC_CHECK += -DDISABLE_CURL_SUPPORT
  CFLAGS += -DDISABLE_CURL_SUPPORT
endif

ifeq ($(DEBUG),1)
  CC_CHECK += -DGAME_DEBUG
  CFLAGS += -DGAME_DEBUG
endif

ifeq ($(DEBUG),2)
  CC_CHECK += -DGAME_DEBUG -DSM64_DEBUG
  CFLAGS += -DGAME_DEBUG -DSM64_DEBUG
endif

################################

# Check for enhancement options

# Check for Puppycam option
ifeq ($(BETTERCAMERA),1)
  CC_CHECK += -DBETTERCAMERA
  CFLAGS += -DBETTERCAMERA
endif

# Check for no drawing distance option
ifeq ($(NODRAWINGDISTANCE),1)
  CC_CHECK += -DNODRAWINGDISTANCE
  CFLAGS += -DNODRAWINGDISTANCE
endif

# Check for Discord Rich Presence option
ifeq ($(DISCORDRPC),1)
  CC_CHECK += -DDISCORDRPC
  CFLAGS += -DDISCORDRPC
endif

# Check for texture fix option
ifeq ($(TEXTURE_FIX),1)
  CC_CHECK += -DTEXTURE_FIX
  CFLAGS += -DTEXTURE_FIX
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

# tell skyconv to write names instead of actual texture data and save the split tiles so we can use them later
SKYTILE_DIR := $(BUILD_DIR)/textures/skybox_tiles

ASFLAGS := -I include -I $(BUILD_DIR) $(VERSION_ASFLAGS)

ifeq ($(TARGET_WEB),1)
LDFLAGS := -lm -lGL -lSDL2 -no-pie -s TOTAL_MEMORY=20MB -g4 --source-map-base http://localhost:8080/ -s "EXTRA_EXPORTED_RUNTIME_METHODS=['callMain']"

else ifeq ($(WINDOWS_BUILD),1)
  LDFLAGS := $(BITS) -march=$(TARGET_ARCH) -Llib -lpthread $(BACKEND_LDFLAGS) -static
  ifeq ($(CROSS),)
    LDFLAGS += -no-pie
  endif
  ifeq ($(WINDOWS_CONSOLE),1)
    LDFLAGS += -mconsole
  endif

else ifeq ($(TARGET_RPI),1)
  LDFLAGS := $(OPT_FLAGS) -lm $(BACKEND_LDFLAGS) -no-pie -lcurl

else ifeq ($(OSX_BUILD),1)
  LDFLAGS := -lm $(PLATFORM_LDFLAGS) $(BACKEND_LDFLAGS) -lpthread -lcurl

else ifeq ($(TARGET_SWITCH),1)
   LDFLAGS := -specs=$(LIBNX)/switch.specs $(NXARCH) $(OPT_FLAGS) -no-pie -L$(LIBNX)/lib $(BACKEND_LDFLAGS) -lnx -lm ` $(NXPATH)/curl-config --libs`

else
  LDFLAGS := $(BITS) -march=$(TARGET_ARCH) -lm $(BACKEND_LDFLAGS) -no-pie -lpthread -lcurl
  ifeq ($(DISCORDRPC),1)
    LDFLAGS += -ldl -Wl,-rpath .
  endif

endif # End of LDFLAGS

LDFLAGS += -lstdc++

# Prevent a crash with -sopt
export LANG := C

################################# Other Tools ##################################
# N64 conversion tools
TOOLS_DIR = tools
AIFF_EXTRACT_CODEBOOK = $(TOOLS_DIR)/aiff_extract_codebook
VADPCM_ENC = $(TOOLS_DIR)/vadpcm_enc
EXTRACT_DATA_FOR_MIO = $(TOOLS_DIR)/extract_data_for_mio
ZEROTERM = $(PYTHON) $(TOOLS_DIR)/zeroterm.py

############################### Dependency Check ###############################

# Stubbed

#################################### Targets ###################################

all: $(EXE)
ifeq ($(TARGET_SWITCH),1)
all: $(EXE).nro
endif

ADDONS      := addons
ADDONS_PATH := $(BUILD_DIR)/$(ADDONS)/
BASEPACK_LST := $(BUILD_DIR)/basepack.lst

# depend on resources as well
all: $(BASEPACK_LST)

# phony target for building resources
res: $(BASEPACK_LST)

ifneq ($(NO_COPY),1)
# prepares the basepack.lst
$(BASEPACK_LST): $(EXE)
	@touch $(BASEPACK_LST)
	@echo "$(BUILD_DIR)/sound/bank_sets sound/bank_sets" >> $(BASEPACK_LST)
	@echo "$(BUILD_DIR)/sound/sequences.bin sound/sequences.bin" >> $(BASEPACK_LST)
	@echo "$(BUILD_DIR)/sound/sound_data.ctl sound/sound_data.ctl" >> $(BASEPACK_LST)
	@echo "$(BUILD_DIR)/sound/sound_data.tbl sound/sound_data.tbl" >> $(BASEPACK_LST)
	@$(foreach f, $(wildcard $(SKYTILE_DIR)/*), echo $(f) gfx/$(f:$(BUILD_DIR)/%=%) >> $(BASEPACK_LST);)
	@find actors -name \*.png -exec echo "{} gfx/{}" >> $(BASEPACK_LST) \;
	@find levels -name \*.png -exec echo "{} gfx/{}" >> $(BASEPACK_LST) \;
	@find textures -name \*.png -exec echo "{} gfx/{}" >> $(BASEPACK_LST) \;
	@find texts -name \*.json -exec echo "{} {}" >> $(BASEPACK_LST) \;
	@find fonts -name \*.ttf -exec echo "{} {}" >> $(BASEPACK_LST) \;
	@$(PYTHON) $(TOOLS_DIR)/mkzip.py $(BASEPACK_LST) $(BUILD_DIR)
endif

clean:
	$(RM) -r $(BUILD_DIR_BASE)

cleantools:
	$(MAKE) -s -C tools clean

distclean:
	$(RM) -r $(BUILD_DIR_BASE)
	./extract_assets.py --clean

$(BUILD_DIR)/$(RPC_LIBS):
	mkdir -p $(@D)
	cp $(RPC_LIBS) $(@D)

libultra: $(BUILD_DIR)/libultra.a

$(BUILD_DIR)/asm/boot.o: $(IPL3_RAW_FILES)
$(BUILD_DIR)/src/game/crash_screen.o: $(CRASH_TEXTURE_C_FILES)

$(BUILD_DIR)/lib/rsp.o: $(BUILD_DIR)/rsp/rspboot.bin $(BUILD_DIR)/rsp/fast3d.bin $(BUILD_DIR)/rsp/audio.bin

RSP_DIRS := $(BUILD_DIR)/rsp
ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS) $(ASM_DIRS) $(GODDARD_SRC_DIRS) $(ULTRA_SRC_DIRS) $(ULTRA_ASM_DIRS) $(ULTRA_BIN_DIRS) $(BIN_DIRS) $(TEXTURE_DIRS) $(SOUND_SAMPLE_DIRS) $(addprefix levels/,$(LEVEL_DIRS)) include) $(MIO0_DIR) $(addprefix $(MIO0_DIR)/,$(VERSION)) $(SOUND_BIN_DIR) $(SOUND_BIN_DIR)/sequences/$(VERSION) $(RSP_DIRS) $(ADDONS_PATH)

# Make sure build directory exists before compiling anything
DUMMY != mkdir -p $(ALL_DIRS)
# compressed segment generation

# PC Area
$(BUILD_DIR)/%.table: %.aiff
	$(AIFF_EXTRACT_CODEBOOK) $< >$@

$(BUILD_DIR)/%.aifc: $(BUILD_DIR)/%.table %.aiff
	$(VADPCM_ENC) -c $^ $@

$(BUILD_DIR)/rsp/%.bin $(BUILD_DIR)/rsp/%_data.bin: rsp/%.s
	$(RSPASM) -sym $@.sym -definelabel $(VERSION_DEF) 1 -definelabel $(GRUCODE_DEF) 1 -strequ CODE_FILE $(BUILD_DIR)/rsp/$*.bin -strequ DATA_FILE $(BUILD_DIR)/rsp/$*_data.bin $<

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

$(SOUND_BIN_DIR)/sequences.bin: $(SOUND_BANK_FILES) sound/sequences.json sound/sequences/ sound/sequences/$(VERSION)/ $(SOUND_SEQUENCE_FILES) $(ENDIAN_BITWIDTH)
	$(PYTHON) tools/assemble_sound.py --sequences $@ $(SOUND_BIN_DIR)/bank_sets sound/sound_banks/ sound/sequences.json $(SOUND_SEQUENCE_FILES) $(VERSION_CFLAGS) $$(cat $(ENDIAN_BITWIDTH))

$(SOUND_BIN_DIR)/bank_sets: $(SOUND_BIN_DIR)/sequences.bin
	@true

$(SOUND_BIN_DIR)/%.m64: $(SOUND_BIN_DIR)/%.o
	$(OBJCOPY) -j .rodata $< -O binary $@

$(SOUND_BIN_DIR)/%.o: $(SOUND_BIN_DIR)/%.s
	$(AS) $(ASFLAGS) -o $@ $<

$(SOUND_BIN_DIR)/%.inc.c: $(SOUND_BIN_DIR)/%
	$(ZEROTERM) "$(patsubst $(BUILD_DIR)/%,%,$^)" | hexdump -v -e '1/1 "0x%X,"' > $@

$(SOUND_BIN_DIR)/sound_data.o: $(SOUND_BIN_DIR)/sound_data.ctl.inc.c $(SOUND_BIN_DIR)/sound_data.tbl.inc.c $(SOUND_BIN_DIR)/sequences.bin.inc.c $(SOUND_BIN_DIR)/bank_sets.inc.c

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

# The source-to-source optimizer copt is enabled for audio. This makes it use
# acpp, which needs -Wp,-+ to handle C++-style comments.
$(BUILD_DIR)/src/audio/effects.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0 -sopt,-inline=sequence_channel_process_sound,-scalaroptimize=1 -Wp,-+
$(BUILD_DIR)/src/audio/synthesis.o: OPT_FLAGS := -O2 -sopt,-scalaroptimize=1 -Wp,-+
#$(BUILD_DIR)/src/audio/seqplayer.o: OPT_FLAGS := -O2 -sopt,-inline_manual,-scalaroptimize=1 -Wp,-+ #-Wo,-v,-bb,-l,seqplayer_list.txt

# Rebuild files with 'GLOBAL_ASM' if the NON_MATCHING flag changes.
$(GLOBAL_ASM_O_FILES): $(GLOBAL_ASM_DEP).$(NON_MATCHING)
$(GLOBAL_ASM_DEP).$(NON_MATCHING):
	@rm -f $(GLOBAL_ASM_DEP).*
	touch $@

$(BUILD_DIR)/%.o: %.cpp
	@$(CXX) -fsyntax-only $(CFLAGS) $(CXXFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c
	@$(CC_CHECK) $(CC_CHECK_CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	@$(CC_CHECK) $(CC_CHECK_CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.s
	$(AS) $(ASFLAGS) -MD $(BUILD_DIR)/$*.d -o $@ $<

$(EXE): $(O_FILES) $(MIO0_FILES:.mio0=.o) $(SOUND_OBJ_FILES) $(ULTRA_O_FILES) $(GODDARD_O_FILES) $(BUILD_DIR)/$(RPC_LIBS)
	$(LD) -L $(BUILD_DIR) -o $@ $(O_FILES) $(SOUND_OBJ_FILES) $(ULTRA_O_FILES) $(GODDARD_O_FILES) $(LDFLAGS)

ifeq ($(TARGET_SWITCH), 1)

# add `--icon=$(APP_ICON)` to this when we get a suitable icon
%.nro: %.stripped %.nacp
	@elf2nro $< $@ --nacp=$*.nacp --icon=$(APP_ICON)
	@echo built ... $(notdir $@)
	@echo $(APP_ICON)

%.nacp:
	@nacptool --create "$(APP_TITLE)" "$(APP_AUTHOR)" "$(APP_VERSION)" $@ $(NACPFLAGS)
	@echo built ... $(notdir $@)

%.stripped: %
	@$(STRIP) -o $@ $<
	@echo stripped ... $(notdir $<)

endif

testclean:
	@rm -rf $(BUILD_DIR)/$(SRC_DIRS)

.PHONY: all clean distclean default diff libultra res
.PRECIOUS: $(BUILD_DIR)/bin/%.elf $(SOUND_BIN_DIR)/%.ctl $(SOUND_BIN_DIR)/%.tbl $(SOUND_SAMPLE_TABLES) $(SOUND_BIN_DIR)/%.s $(BUILD_DIR)/%
.DELETE_ON_ERROR:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true