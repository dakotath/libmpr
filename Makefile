#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)

include $(DEVKITPPC)/wii_rules

#-------------------------------------------------------------------------------
# library version
#-------------------------------------------------------------------------------
VERSION		:=	1.0.0

#-------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing header files
# INSTALL is the directory where the library will be installed
#-------------------------------------------------------------------------------
TARGET		:=	libmpr
BUILD		:=	build
SOURCES		:=	source source/SAM
DATA		:=	data
INCLUDES	:=	include

# Here be dragons
INSTALL		?=	$(DEVKITPRO)/libogc

#-------------------------------------------------------------------------------
# options for code generation
#-------------------------------------------------------------------------------
CFLAGS		:=	-g -DCC_BUILD_GCWII -DCC_BUILD_LIB_GCWII -O2 -Wall $(MACHDEP) $(INCLUDE) $(DEFINES) \
			-D__LINUX_ERRNO_EXTENSIONS__

CXXFLAGS	:=	$(CFLAGS)

ASFLAGS		:=	$(MACHDEP)

LIBS	:= -lwiiuse -ldi -lwiikeyboard -lmad -lgrrlib -lfreetype -lpngu -lpng -ljpeg -lbz2
LIBS	+= -lz -lfat
#LIBS	+= -lmodplay -laesnd
LIBS	+= -lvorbisidec -logg -lasnd
LIBS	+= -lbte -logc -lm

#-------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level
# containing include and lib
#-------------------------------------------------------------------------------
LIBDIRS	:= $(CURDIR) $(PORTLIBS)

#-------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#-------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#-------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#-------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#-------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#-------------------------------------------------------------------------------
	export LD	:=	$(CC)
#-------------------------------------------------------------------------------
else
#-------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------

export SRCFILES		:=	$(CPPFILES) $(CFILES) $(SFILES)
export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC)


.PHONY: dist-bin dist-src dist install all clean

#-------------------------------------------------------------------------------

dist-bin: all
	@[ -d lib/wii ] || mkdir -p lib/wii
	@cp $(TARGET).a lib/wii/
	@tar --exclude=*~ -cjf $(TARGET)-$(VERSION).tar.bz2 include lib

dist-src:
	@tar --exclude=*~ -cjf $(TARGET)-src-$(VERSION).tar.bz2 include source Makefile README.md

dist: dist-src dist-bin

install: dist-bin
	mkdir -p $(DESTDIR)$(INSTALL)
	bzip2 -cd $(TARGET)-$(VERSION).tar.bz2 | tar -xf - -C $(DESTDIR)$(INSTALL)

all: $(SRCFILES) $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

$(BUILD):
	@[ -d $@ ] || mkdir -p $@

#-------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -rf $(BUILD) $(TARGET).a lib

#-------------------------------------------------------------------------------
else
.PHONY:	all

#-------------------------------------------------------------------------------
# main targets
#-------------------------------------------------------------------------------
all		:	$(OUTPUT).a

$(OUTPUT).a	:	$(OFILES)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .jpg extension
#---------------------------------------------------------------------------------
%.jpg.o	:	%.jpg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .png extension
#---------------------------------------------------------------------------------
%.png.o	:	%.png
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)
#---------------------------------------------------------------------------------
# This rule links in binary data with the .ttf extension
#---------------------------------------------------------------------------------
%.ttf.o	:	%.ttf
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)
#---------------------------------------------------------------------------------
# This rule links in binary data with the .ogg extension
#---------------------------------------------------------------------------------
%.ogg.o	%_ogg.h :	%.ogg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

-include $(DEPENDS)

#-------------------------------------------------------------------------------
endif
#------------------------------------------------------------------------------- 
