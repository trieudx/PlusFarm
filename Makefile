#############################################################
# Required variables for each makefile
# Discard this section from all parent makefiles
# Expected variables (with automatic defaults):
#   CSRCS (all "C" files in the dir)
#   SUBDIRS (all subdirs with a Makefile)
#   GEN_LIBS - list of libs to be generated ()
#   GEN_IMAGES - list of object file images to be generated ()
#   GEN_BINS - list of binaries to be generated ()
#   COMPONENTS_xxx - a list of libs/objs in the form
#     subdir/lib to be extracted and rolled up into
#     a generated lib/image xxx.a ()
#
TARGET = eagle
#FLAVOR = release
FLAVOR = debug

#EXTRA_CCFLAGS += -u

ifndef PDIR # {
GEN_IMAGES= eagle.app.v6.out
GEN_BINS= eagle.app.v6.bin
SPECIAL_MKTARGETS=$(APP_MKTARGETS)
SUBDIRS=    \
	user    \
	driver  \
	upgrade \
	freertos \
	json \
	lwip

endif # } PDIR

LDDIR = $(PDIR)ld

CCFLAGS += -Os

TARGET_LDFLAGS =		\
	-nostdlib		\
	-Wl,-EL \
	--longcalls \
	--text-section-literals

ifeq ($(FLAVOR),debug)
    TARGET_LDFLAGS += -g -O2
endif

ifeq ($(FLAVOR),release)
    TARGET_LDFLAGS += -g -O0
endif

dummy: all

COMPONENTS_eagle.app.v6 = \
	user/libuser.a  \
	driver/libdriver.a \
	upgrade/libupgrade.a \
	freertos/libfreertos.a \
	json/libjson.a \
	lwip/liblwip.a

LINKFLAGS_eagle.app.v6 = \
	-L$(PDIR)lib        \
	-Wl,--gc-sections   \
	-nostdlib	\
    -T$(LD_FILE)   \
	-Wl,--no-check-sections	\
    -u call_user_start	\
	-Wl,-static						\
	-Wl,--start-group					\
	-lcirom \
	-lmirom	\
	-lgcc					\
	-lhal					\
	-lphy	\
	-lpp	\
	-lnet80211	\
	-lcrypto \
	-lwpa	\
	-lmain	\
	$(DEP_LIBS_eagle.app.v6)					\
	-Wl,--end-group

DEPENDS_eagle.app.v6 = \
                $(LD_FILE) \
                $(LDDIR)/eagle.rom.addr.v6.ld

#############################################################
# Configuration i.e. compile options etc.
# Target specific stuff (defines etc.) goes in here!
# Generally values applying to a tree are captured in the
#   makefile at its root level - these are then overridden
#   for a subtree within the makefile rooted therein
#

#UNIVERSAL_TARGET_DEFINES =		\

# Other potential configuration flags include:
#	-DTXRX_TXBUF_DEBUG
#	-DTXRX_RXBUF_DEBUG
#	-DWLAN_CONFIG_CCX
CONFIGURATION_DEFINES =	-DICACHE_FLASH

DEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)

DDEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)


#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

INCLUDES := -I $(PDIR)include
INCLUDES += -I $(PDIR)include/driver
INCLUDES += -I $(PDIR)include/espressif
INCLUDES += -I $(PDIR)include/freertos
INCLUDES += -I $(PDIR)include/json
INCLUDES += -I $(PDIR)include/lwip
INCLUDES += -I $(PDIR)include/lwip/ipv4
INCLUDES += -I $(PDIR)include/lwip/ipv6
INCLUDES += -I $(PDIR)include/lwip/posix
INCLUDES += -I $(PDIR)include/xtensa

#  copyright (c) 2010 Espressif System
#
ifndef PDIR

endif

ifeq ($(COMPILE), xcc)
    AR = xt-ar
	CC = xt-xcc
	NM = xt-nm
	CPP = xt-xt++
	OBJCOPY = xt-objcopy
	OBJDUMP = xt-objdump
else
	AR = xtensa-lx106-elf-ar
	CC = xtensa-lx106-elf-gcc
	NM = xtensa-lx106-elf-nm
	CPP = xtensa-lx106-elf-g++
	OBJCOPY = xtensa-lx106-elf-objcopy
	OBJDUMP = xtensa-lx106-elf-objdump
endif

# boot mode: none/old/new
BOOT?=none
# app file(s):
#  0: eagle.flash.bin + eagle.irom0text.bin
#  1: user1.bin
#  2: user2.bin
APP?=0
# spi speed: 20/26.7/40/80
SPI_SPEED?=40
# spi mode: QIO/QOUT/DIO/DOUT
SPI_MODE?=QIO
# spi size map:
#  0= 512KB( 256KB+ 256KB)
#  2=1024KB( 512KB+ 512KB)
#  3=2048KB( 512KB+ 512KB)
#  4=4096KB( 512KB+ 512KB)
#  5=2048KB(1024KB+1024KB)
#  6=4096KB(1024KB+1024KB)
SPI_SIZE_MAP?=4

ifeq ($(BOOT), new)
    boot = new
else
    ifeq ($(BOOT), old)
        boot = old
    else
        boot = none
    endif
endif

ifeq ($(APP), 1)
    app = 1
else
    ifeq ($(APP), 2)
        app = 2
    else
        app = 0
    endif
endif

ifeq ($(SPI_SPEED), 26.7)
    freqdiv = 1
else
    ifeq ($(SPI_SPEED), 20)
        freqdiv = 2
    else
        ifeq ($(SPI_SPEED), 80)
            freqdiv = 15
        else
            freqdiv = 0
        endif
    endif
endif


ifeq ($(SPI_MODE), QOUT)
    mode = 1
else
    ifeq ($(SPI_MODE), DIO)
        mode = 2
    else
        ifeq ($(SPI_MODE), DOUT)
            mode = 3
        else
            mode = 0
        endif
    endif
endif

addr = 0x01000

ifeq ($(SPI_SIZE_MAP), 1)
  size_map = 1
  flash = 256
else
  ifeq ($(SPI_SIZE_MAP), 2)
    size_map = 2
    flash = 1024
    ifeq ($(app), 2)
      addr = 0x81000
    endif
  else
    ifeq ($(SPI_SIZE_MAP), 3)
      size_map = 3
      flash = 2048
      ifeq ($(app), 2)
        addr = 0x81000
      endif
    else
      ifeq ($(SPI_SIZE_MAP), 4)
        size_map = 4
        flash = 4096
        ifeq ($(app), 2)
          addr = 0x81000
        endif
      else
        ifeq ($(SPI_SIZE_MAP), 5)
          size_map = 5
          flash = 2048
          ifeq ($(app), 2)
            addr = 0x101000
          endif
        else
          ifeq ($(SPI_SIZE_MAP), 6)
            size_map = 6
            flash = 4096
            ifeq ($(app), 2)
              addr = 0x101000
            endif
          else
            size_map = 0
            flash = 512
            ifeq ($(app), 2)
              addr = 0x41000
            endif
          endif
        endif
      endif
    endif
  endif
endif

LD_FILE = $(LDDIR)/eagle.app.v6.ld

ifneq ($(boot), none)
ifneq ($(app),0)
    ifeq ($(size_map), 6)
      LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).2048.ld
    else
      ifeq ($(size_map), 5)
        LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).2048.ld
      else
        ifeq ($(size_map), 4)
          LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).1024.app$(app).ld
        else
          ifeq ($(size_map), 3)
            LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).1024.app$(app).ld
          else
            ifeq ($(size_map), 2)
              LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).1024.app$(app).ld
            else
              ifeq ($(size_map), 0)
                LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).512.app$(app).ld
              endif
            endif
	      endif
	    endif
	  endif
	endif
	BIN_NAME = user$(app).$(flash).$(boot).$(size_map)
endif
else
    app = 0
endif

CSRCS ?= $(wildcard *.c)
CPPSRCS ?= $(wildcard *.cpp)
ASRCs ?= $(wildcard *.s)
ASRCS ?= $(wildcard *.S)
SUBDIRS ?= $(patsubst %/,%,$(dir $(wildcard */Makefile)))

BIN_PATH := $(PDIR)bin

ODIR := .output
OBJODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/obj

OBJS := $(CSRCS:%.c=$(OBJODIR)/%.o) \
        $(CPPSRCS:%.cpp=$(OBJODIR)/%.o) \
        $(ASRCs:%.s=$(OBJODIR)/%.o) \
        $(ASRCS:%.S=$(OBJODIR)/%.o)

DEPS := $(CSRCS:%.c=$(OBJODIR)/%.d) \
        $(CPPSRCS:%.cpp=$(OBJODIR)/%.d) \
        $(ASRCs:%.s=$(OBJODIR)/%.d) \
        $(ASRCS:%.S=$(OBJODIR)/%.d)

LIBODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/lib
OLIBS := $(GEN_LIBS:%=$(LIBODIR)/%)

IMAGEODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/image
OIMAGES := $(GEN_IMAGES:%=$(IMAGEODIR)/%)

BINODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/bin
OBINS := $(GEN_BINS:%=$(BINODIR)/%)

CCFLAGS += 			\
	-g			\
	-Wpointer-arith		\
	-Wundef			\
	-Werror			\
	-Wl,-EL			\
	-fno-inline-functions	\
	-nostdlib       \
	-mlongcalls	\
	-mtext-section-literals \
	-ffunction-sections \
	-fdata-sections	\
	-fno-builtin-printf
#	-Wall			

CFLAGS = $(CCFLAGS) $(DEFINES) $(EXTRA_CCFLAGS) $(INCLUDES)
DFLAGS = $(CCFLAGS) $(DDEFINES) $(EXTRA_CCFLAGS) $(INCLUDES)


#############################################################
# Functions
#

define ShortcutRule
$(1): .subdirs $(2)/$(1)
endef

define MakeLibrary
DEP_LIBS_$(1) = $$(foreach lib,$$(filter %.a,$$(COMPONENTS_$(1))),$$(dir $$(lib))$$(LIBODIR)/$$(notdir $$(lib)))
DEP_OBJS_$(1) = $$(foreach obj,$$(filter %.o,$$(COMPONENTS_$(1))),$$(dir $$(obj))$$(OBJODIR)/$$(notdir $$(obj)))
$$(LIBODIR)/$(1).a: $$(OBJS) $$(DEP_OBJS_$(1)) $$(DEP_LIBS_$(1)) $$(DEPENDS_$(1))
	@mkdir -p $$(LIBODIR)
	$$(if $$(filter %.a,$$?),mkdir -p $$(EXTRACT_DIR)_$(1))
	$$(if $$(filter %.a,$$?),cd $$(EXTRACT_DIR)_$(1); $$(foreach lib,$$(filter %.a,$$?),$$(AR) xo $$(UP_EXTRACT_DIR)/$$(lib);))
	$$(AR) ru $$@ $$(filter %.o,$$?) $$(if $$(filter %.a,$$?),$$(EXTRACT_DIR)_$(1)/*.o)
	$$(if $$(filter %.a,$$?),$$(RM) -r $$(EXTRACT_DIR)_$(1))
endef

define MakeImage
DEP_LIBS_$(1) = $$(foreach lib,$$(filter %.a,$$(COMPONENTS_$(1))),$$(dir $$(lib))$$(LIBODIR)/$$(notdir $$(lib)))
DEP_OBJS_$(1) = $$(foreach obj,$$(filter %.o,$$(COMPONENTS_$(1))),$$(dir $$(obj))$$(OBJODIR)/$$(notdir $$(obj)))
$$(IMAGEODIR)/$(1).out: $$(OBJS) $$(DEP_OBJS_$(1)) $$(DEP_LIBS_$(1)) $$(DEPENDS_$(1))
	@mkdir -p $$(IMAGEODIR)
	$$(CC) $$(LDFLAGS) $$(if $$(LINKFLAGS_$(1)),$$(LINKFLAGS_$(1)),$$(LINKFLAGS_DEFAULT) $$(OBJS) $$(DEP_OBJS_$(1)) $$(DEP_LIBS_$(1))) -o $$@ 
endef

$(BINODIR)/%.bin: $(IMAGEODIR)/%.out
	@mkdir -p $(BIN_PATH)
	@mkdir -p $(BINODIR)
	
ifeq ($(APP), 0)
	@$(RM) -r $(BIN_PATH)/eagle.S $(BIN_PATH)/eagle.dump
	@$(OBJDUMP) -x -s $< > $(BIN_PATH)/eagle.dump
	@$(OBJDUMP) -S $< > $(BIN_PATH)/eagle.S
else
	@mkdir -p $(BIN_PATH)/upgrade
	@$(RM) -r $(BIN_PATH)/upgrade/$(BIN_NAME).S $(BIN_PATH)/upgrade/$(BIN_NAME).dump
	@$(OBJDUMP) -x -s $< > $(BIN_PATH)/upgrade/$(BIN_NAME).dump
	@$(OBJDUMP) -S $< > $(BIN_PATH)/upgrade/$(BIN_NAME).S
endif

	@$(OBJCOPY) --only-section .text -O binary $< eagle.app.v6.text.bin
	@$(OBJCOPY) --only-section .data -O binary $< eagle.app.v6.data.bin
	@$(OBJCOPY) --only-section .rodata -O binary $< eagle.app.v6.rodata.bin
	@$(OBJCOPY) --only-section .irom0.text -O binary $< eagle.app.v6.irom0text.bin

ifeq ($(app), 0)
	@python $(PDIR)tools/gen_appbin.py $< 0 $(mode) $(freqdiv) $(size_map)
	@mv eagle.app.flash.bin $(BIN_PATH)/eagle.flash.bin
	@mv eagle.app.v6.irom0text.bin $(BIN_PATH)/eagle.irom0text.bin
	@rm eagle.app.v6.*
	@echo "BIN_PATH: $(BIN_PATH)"
	@echo ""
	@echo "No boot needed."
	@echo "Generate eagle.flash.bin and eagle.irom0text.bin successully in BIN_PATH"
	@echo "eagle.flash.bin-------->0x00000"
	@echo "eagle.irom0text.bin---->0x10000"
else
	@echo "BIN_PATH: $(BIN_PATH)/upgrade"
	@echo ""

    ifneq ($(boot), new)
		@python $(PDIR)tools/gen_appbin.py $< 1 $(mode) $(freqdiv) $(size_map)
		@echo "Support boot_v1.1 and +"
    else
		@python $(PDIR)tools/gen_appbin.py $< 2 $(mode) $(freqdiv) $(size_map)

    	ifeq ($(size_map), 6)
		@echo "Support boot_v1.4 and +"
        else
            ifeq ($(size_map), 5)
		@echo "Support boot_v1.4 and +"
            else
		@echo "Support boot_v1.2 and +"
            endif
        endif
    endif

	@mv eagle.app.flash.bin $(BIN_PATH)/upgrade/$(BIN_NAME).bin
	@rm eagle.app.v6.*
	@echo "Generate $(BIN_NAME).bin successully in BIN_PATH"
	@echo "boot.bin------------>0x00000"
	@echo "$(BIN_NAME).bin--->$(addr)"
endif

	@echo "!!!"

#############################################################
# Rules base
# Should be done in top-level makefile only
#

all:	.subdirs $(OBJS) $(OLIBS) $(OIMAGES) $(OBINS) $(SPECIAL_MKTARGETS)

clean:
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) clean;)
	$(RM) -r $(ODIR)/$(TARGET)/$(FLAVOR)

clobber: $(SPECIAL_CLOBBER)
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) clobber;)
	$(RM) -r $(ODIR)

.subdirs:
	@set -e; $(foreach d, $(SUBDIRS), $(MAKE) -C $(d);)

#.subdirs:
#	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d))

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),clobber)
ifdef DEPS
sinclude $(DEPS)
endif
endif
endif

$(OBJODIR)/%.o: %.c
	@mkdir -p $(OBJODIR);
	$(CC) $(if $(findstring $<,$(DSRCS)),$(DFLAGS),$(CFLAGS)) $(COPTS_$(*F)) -o $@ -c $<

$(OBJODIR)/%.d: %.c
	@mkdir -p $(OBJODIR);
#	@echo DEPEND: $(CC) -M $(CFLAGS) $<
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	
$(OBJODIR)/%.o: %.cpp
	@mkdir -p $(OBJODIR);
	$(CPP) $(if $(findstring $<,$(DSRCS)),$(DFLAGS),$(CFLAGS)) $(COPTS_$(*F)) -o $@ -c $<

$(OBJODIR)/%.d: %.cpp
	@mkdir -p $(OBJODIR);
#	@echo DEPEND: $(CPP) -M $(CFLAGS) $<
	@set -e; rm -f $@; \
	$(CPP) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJODIR)/%.o: %.s
	@mkdir -p $(OBJODIR);
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJODIR)/%.d: %.s
	@mkdir -p $(OBJODIR); \
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJODIR)/%.o: %.S
	@mkdir -p $(OBJODIR);
	$(CC) $(CFLAGS) -D__ASSEMBLER__ -o $@ -c $<

$(OBJODIR)/%.d: %.S
	@mkdir -p $(OBJODIR); \
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(foreach lib,$(GEN_LIBS),$(eval $(call ShortcutRule,$(lib),$(LIBODIR))))

$(foreach image,$(GEN_IMAGES),$(eval $(call ShortcutRule,$(image),$(IMAGEODIR))))

$(foreach bin,$(GEN_BINS),$(eval $(call ShortcutRule,$(bin),$(BINODIR))))

$(foreach lib,$(GEN_LIBS),$(eval $(call MakeLibrary,$(basename $(lib)))))

$(foreach image,$(GEN_IMAGES),$(eval $(call MakeImage,$(basename $(image)))))

#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

.PHONY: FORCE
FORCE:

