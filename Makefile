TARGET   := libClassicInjector.so
ECHO     ?= @echo "$(shell basename $@)";

SOURCES  ?= $(CURDIR)/.
INCLUDES := $(SOURCES)

CFLAGS   :=
CFLAGS   := $(CFLAGS) -Wall -Wextra -Wno-unused-parameter -Os -s -fPIC
#CFLAGS   := $(CFLAGS) -flto=4 -fwhole-program -fuse-linker-plugin \
#  -fdata-sections -ffunction-sections -Wl,--gc-sections
CFLAGS   := $(CFLAGS) -fno-stack-protector -fno-ident -fomit-frame-pointer \
  -falign-functions=1 -falign-jumps=1 -falign-loops=1 \
  -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-unroll-loops \
  -fmerge-all-constants -fno-math-errno \
  -mthumb -march=armv7 -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard

CFLAGS   := $(CFLAGS) -Wl,--as-needed
#CFLAGS   := $(CFLAGS) -DBUILD_COMMIT="\"`git rev-parse --short HEAD``git diff --shortstat --quiet || echo ' (dirty)'`\"" -DBUILD_DATE="\"`date -u +'%Y-%m-%d %H:%M:%S %Z'`\""

CXXFLAGS := $(CFLAGS) -std=gnu++11
CFLAGS   := $(CFLAGS) -std=c99

LIBDIR   :=
LDFLAGS  := -lm -shared -Wl,--version-script=link.T -Wl,--no-undefined -Wl,--as-needed -L./lib -lSDL2 -lpthread -ldl

INCLUDE  += $(foreach dir,$(INCLUDES),-I$(dir))
CFILES   += $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES += $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

OBJS     := $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
DEPENDS  := $(OBJS:.o=.d)

OUTPUT   := $(CURDIR)
ELF      := $(OUTPUT)/$(TARGET)

export VPATH := $(foreach dir,$(SOURCES),$(dir))

CROSS_COMPILE ?= arm-linux-
GCC ?= $(CROSS_COMPILE)gcc
STRIP ?= $(CROSS_COMPILE)strip

.PHONY: all clean
all: $(OUTPUT) $(ELF)

%.o:%.c Makefile
	$(ECHO)$(GCC) -MMD $(CFLAGS) $(INCLUDE) -c "$<" -o "$@"

%.o:%.cpp Makefile
	$(ECHO)$(GCC) -MMD $(CXXFLAGS) $(INCLUDE) -c "$<" -o "$@"

-include $(DEPENDS)

$(OUTPUT):
	$(ECHO)mkdir -p "$@"

$(ELF): $(OBJS)
	$(ECHO)$(GCC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -o "$@" && retrolink "$@" && chmod 644 "$@"

$(ELF).upx: $(ELF)
	$(ECHO)upx -qq --ultra-brute "$<" -o "$@" -f

clean:
	$(ECHO)rm -f *.o *.d *.elf "$(TARGET)"
