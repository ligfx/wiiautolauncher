.SUFFIXES:

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

TARGET		:=	boot
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data

CFLAGS		=	-g -O3 -std=gnu99 -Wall -Wextra -Wformat $(MACHDEP) $(INCLUDE) 
LDFLAGS		=	$(MACHDEP) -Wl,--section-start,.init=0x80B00000

LIBS	:=	-lz -lfat -logc
LIBDIRS	:=	"$(CURDIR)" $(PORTLIBS)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	"$(CURDIR)/$(TARGET)"

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	"$(CURDIR)/$(BUILD)"

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))

ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.S=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC)
					
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib -L$(dir)/lib/wii) \
					-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)

.PHONY: $(BUILD) all clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
       
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).bin $(OUTPUT).dol


else

DEPENDS	:=	$(OFILES:.o=.d)
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif
