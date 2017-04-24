## ========================================================================== ##
## ----------------------------- MODULES ------------------------------------ ##
## ========================================================================== ##
MODULES						:= CORE
MODULES						+= HAL
MODULES						+= RBOOT
MODULES						+= FREERTOS
MODULES						+= DRIVER
MODULES						+= LWIP
MODULES						+= HTTPD
MODULES						+= MBEDTLS
MODULES						+= DHCPSERVER
MODULES						+= MQTT
MODULES						+= JSMN
MODULES						+= TFTP
MODULES						+= APP

## ========================================================================== ##
## --------------------------- DIRECTORIES ---------------------------------- ##
## ========================================================================== ##

## ------------------------------ COMMON ------------------------------------ ##
PROJECT_ROOT				= $(PWD)

BOOTLOADER_DIR				= $(PROJECT_ROOT)/bootloader
PLATFORM_DIR				= $(PROJECT_ROOT)/platform
	CORE_DIR				= $(PLATFORM_DIR)/core
		SDKLIB_DIR			= $(CORE_DIR)/sdklib
		NEWLIB_DIR			= $(CORE_DIR)/newlib
		STARTUP_DIR			= $(CORE_DIR)/startup
	HAL_DIR					= $(PLATFORM_DIR)/hal
	FREERTOS_DIR			= $(PLATFORM_DIR)/freertos
	DRIVER_DIR				= $(PLATFORM_DIR)/driver
FRAMEWORK_DIR				= $(PROJECT_ROOT)/framework
	LWIP_DIR				= $(FRAMEWORK_DIR)/lwip
	HTTPD_DIR				= $(FRAMEWORK_DIR)/httpd
	MBEDTLS_DIR				= $(FRAMEWORK_DIR)/mbedtls
	DHCPSERVER_DIR			= $(FRAMEWORK_DIR)/dhcpserver
	MQTT_DIR				= $(FRAMEWORK_DIR)/mqtt
	JSMN_DIR				= $(FRAMEWORK_DIR)/jsmn
	TFTP_DIR				= $(FRAMEWORK_DIR)/tftp
APP_DIR						= $(PROJECT_ROOT)/app
	FSDATA_DIR				= $(APP_DIR)/fsdata
LD_DIR						= $(PROJECT_ROOT)/ld
UTIL_DIR					= $(PROJECT_ROOT)/util
BUILD_DIR					= $(PROJECT_ROOT)/build
	OBJ_DIR					= $(BUILD_DIR)/obj
	LIB_DIR					= $(BUILD_DIR)/lib
	IMAGE_DIR				= $(BUILD_DIR)/image
	BIN_DIR					= $(BUILD_DIR)/bin

## ----------------------------- SOURCE ------------------------------------- ##
SRC_CORE					:= $(SDKLIB_DIR)/src
SRC_CORE					+= $(STARTUP_DIR)/src

SRC_HAL						:= $(HAL_DIR)/src

SRC_RBOOT					:= $(BOOTLOADER_DIR)/rboot/appcode

SRC_FREERTOS				:= $(FREERTOS_DIR)/src

SRC_DRIVER					:= $(DRIVER_DIR)/src

SRC_LWIP					:= $(LWIP_DIR)
SRC_LWIP					+= $(LWIP_DIR)/lwip/src/api
SRC_LWIP					+= $(LWIP_DIR)/lwip/src/core
SRC_LWIP					+= $(LWIP_DIR)/lwip/src/core/ipv4
SRC_LWIP					+= $(LWIP_DIR)/lwip/src/netif

SRC_HTTPD					:= $(HTTPD_DIR)/src

SRC_MBEDTLS					:= $(MBEDTLS_DIR)
SRC_MBEDTLS					+= $(MBEDTLS_DIR)/mbedtls/library

SRC_DHCPSERVER				:= $(DHCPSERVER_DIR)/src

SRC_MQTT					:= $(MQTT_DIR)/src

SRC_JSMN					:= $(JSMN_DIR)/src

SRC_TFTP					:= $(TFTP_DIR)/src

SRC_APP						:= $(APP_DIR)/src

define CreateSrcDirList
SRC_DIRS					+= $$(SRC_$$$(1))
endef

$(foreach userlib, $(MODULES), $(eval $(call CreateSrcDirList, \
							$(userlib), $(shell echo $(userlib) | tr A-Z a-z))))

## ---------------------------- INCLUSION ----------------------------------- ##
## CORE
INCLUDE_DIRS				:= $(SDKLIB_DIR)/include
INCLUDE_DIRS				+= $(NEWLIB_DIR)/include
INCLUDE_DIRS				+= $(STARTUP_DIR)/include
## HAL
INCLUDE_DIRS				+= $(HAL_DIR)/include
## RBOOT
INCLUDE_DIRS				+= $(BOOTLOADER_DIR)/rboot
INCLUDE_DIRS				+= $(BOOTLOADER_DIR)/rboot/appcode
## FREERTOS
INCLUDE_DIRS				+= $(FREERTOS_DIR)/include
## DRIVER
INCLUDE_DIRS				+= $(DRIVER_DIR)/include
## LWIP
INCLUDE_DIRS				+= $(LWIP_DIR)/include
INCLUDE_DIRS				+= $(LWIP_DIR)/lwip/src/include
INCLUDE_DIRS				+= $(LWIP_DIR)/lwip/src/include/ipv4
INCLUDE_DIRS				+= $(LWIP_DIR)/lwip/src/include/posix
## HTTPD
INCLUDE_DIRS				+= $(HTTPD_DIR)/include
## MBEDTLS
INCLUDE_DIRS				+= $(MBEDTLS_DIR)/include
INCLUDE_DIRS				+= $(MBEDTLS_DIR)/mbedtls
INCLUDE_DIRS				+= $(MBEDTLS_DIR)/mbedtls/include
## DHCPSERVER
INCLUDE_DIRS				+= $(DHCPSERVER_DIR)/include
## MQTT
INCLUDE_DIRS				+= $(MQTT_DIR)/include
## JSMN
INCLUDE_DIRS				+= $(JSMN_DIR)/include
## TFTP
INCLUDE_DIRS				+= $(TFTP_DIR)/include
## APP
INCLUDE_DIRS				+= $(APP_DIR)/include

## ========================================================================== ##
## ------------------------------ FILES ------------------------------------- ##
## ========================================================================== ##

PROJECT_NAME				= PlusFarm
## LINKERS
LD_ROM_FILE					:= $(LD_DIR)/rom.ld
LD_PROGRAM_FILE				:= $(LD_DIR)/program.ld
## IMAGE
IMAGE_FILE					:= $(IMAGE_DIR)/$(PROJECT_NAME).out
## MAP
MAP_FILE					:= $(IMAGE_DIR)/$(PROJECT_NAME).map
# BINARY
BIN_FILE					:= $(BIN_DIR)/$(PROJECT_NAME).bin
# ESPTOOL
ESPTOOL						:= esptool.py
# RBOOT
RBOOT_BIN_FILE				:= $(BOOTLOADER_DIR)/build/rboot.bin
PREBUILT_RBOOT_BIN_FILE		:= $(BOOTLOADER_DIR)/prebuild/rboot.bin
RBOOT_CONF_FILE				:= $(BOOTLOADER_DIR)/prebuild/blank_config.bin
ifeq (,$(wildcard $(RBOOT_BIN_FILE)))
RBOOT_BIN_FILE				:= $(PREBUILT_RBOOT_BIN_FILE)
endif
# FILTEROUTPUT
FILTEROUTPUT				:= $(UTIL_DIR)/filteroutput.py

## ----------------------------- OBJECT ------------------------------------- ##
define CreateObjFileList
$(1)_OBJ_FILES		:= $$(patsubst %, $$(OBJ_DIR)/%.o,						\
						$$(basename $$(notdir $$(filter %.c %.S,			\
							$$(wildcard $$(addsuffix /*, $$(SRC_$$$(1))))))))
endef

$(foreach objmodule, $(MODULES),											\
							$(eval $(call CreateObjFileList, $(objmodule))))

## ------------------------------- LIB -------------------------------------- ##
define CreateUserLibFileList
$(1)_LIB_FILE				:= $$(LIB_DIR)/userlib_$$$(2).a
USERLIB_FILES				+= $$($$$(1)_LIB_FILE)
endef

$(foreach userlib, $(MODULES), $(eval $(call CreateUserLibFileList, \
							$(userlib), $(shell echo $(userlib) | tr A-Z a-z))))

SDKLIB_FILES				:= $(LIB_DIR)/sdklib_main.a
SDKLIB_FILES				+= $(LIB_DIR)/sdklib_net80211.a
SDKLIB_FILES				+= $(LIB_DIR)/sdklib_phy.a
SDKLIB_FILES				+= $(LIB_DIR)/sdklib_pp.a
SDKLIB_FILES				+= $(LIB_DIR)/sdklib_wpa.a

NEWLIB_FILE					:= $(NEWLIB_DIR)/lib/libc.a

LIB_FILES					:= $(USERLIB_FILES)
LIB_FILES					+= $(SDKLIB_FILES)
LIB_FILES					+= $(NEWLIB_FILE)

## ========================================================================== ##
## --------------------------- COMPILATION ---------------------------------- ##
## ========================================================================== ##

## ---------------------------- COMPILER ------------------------------------ ##
AR					= xtensa-lx106-elf-ar
CC					= xtensa-lx106-elf-gcc
NM					= xtensa-lx106-elf-nm
OBJCOPY				= xtensa-lx106-elf-objcopy
SIZE				= xtensa-lx106-elf-size

## -------------------------- COMPILE FLAGS --------------------------------- ##
## OPTION
CFLAGS_OPT			:= -O2 -g -std=gnu99 -MD -MP
CFLAGS_OPT			+= -ffunction-sections -fdata-sections
CFLAGS_OPT			+= -nostdlib -Wl,-EL -Wall -Wno-address
CFLAGS_OPT			+= -mlongcalls
CFLAGS_OPT			+= -mtext-section-literals
FLAGS_OPT			+= -Wpointer-arith -Werror
CFLAGS_OPT			+= -fno-aggressive-loop-optimizations
## MACRO
CFLAGS_DEF			:= -D GITSHORTREV=\"31ef50c\"
CFLAGS_DEF			+= -D LWIP_HTTPD_CGI=1 -D LWIP_HTTPD_SSI=1
CFLAGS_DEF			+= -D LOG_VERBOSE=1
CFLAGS_DEF			+= -D USE_OS=1
#CFLAGS_DEF			+= -D USE_FULL_ASSERT=1

CFLAGS				:= $(CFLAGS_OPT) $(CFLAGS_DEF)
CFLAGS				+= $(addprefix -I, $(INCLUDE_DIRS))

VPATH				:= $(SRC_DIRS)

## -------------------------- LINKER FLAGS ---------------------------------- ##
LFLAGS				:= -T $(LD_ROM_FILE) -T $(LD_PROGRAM_FILE)
LFLAGS				+= -Wl,-Map=$(MAP_FILE)
LFLAGS				+= -O2 -g -nostdlib
LFLAGS				+= -Wl,--gc-sections
LFLAGS				+= -Wl,--no-check-sections
LFLAGS				+= -u call_user_start -u _printf_float -u _scanf_float
LFLAGS				+= -Wl,-static
LFLAGS				+= -Wl,--whole-archive
LFLAGS				+= $(CORE_LIB_FILE)
LFLAGS				+= -Wl,--no-whole-archive
LFLAGS				+= -Wl,--start-group
LFLAGS				+= $(HAL_LIB_FILE) $(FREERTOS_LIB_FILE) $(DRIVER_LIB_FILE)
LFLAGS				+= $(LWIP_LIB_FILE) $(HTTPD_LIB_FILE) $(MBEDTLS_LIB_FILE)
LFLAGS				+= $(DHCPSERVER_LIB_FILE) $(MQTT_LIB_FILE) $(JSMN_LIB_FILE)
LFLAGS				+= $(TFTP_LIB_FILE) $(SDKLIB_FILES) $(NEWLIB_FILE)
LFLAGS				+= $(RBOOT_LIB_FILE) $(APP_LIB_FILE)
LFLAGS				+= -lgcc -lhal
LFLAGS				+= -Wl,--end-group

## --------------------------- DEBUG FLAGS ---------------------------------- ##
VERBOSE				?= 0

ifeq ("$(VERBOSE)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

## ========================================================================== ##
## ----------------------------- TARGETS ------------------------------------ ##
## ========================================================================== ##
DEVICE_PORT			= /dev/ttyUSB0
FLASH_BAUD_RATE		= 460800
DEBUG_BAUD_RATE		= 460800
ESPTOOL_PARAMS		= --flash_size 32m --flash_mode qio --flash_freq 80m

# Default target
default-tgt: all

$(OBJ_DIR) $(LIB_DIR) $(IMAGE_DIR) $(BIN_DIR):
	$(Q) mkdir -p $@

$(OBJ_DIR)/%.o: %.S | $(OBJ_DIR)
	$(vecho) "  AS   $<"
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(vecho) "  CC   $<"
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

define MakeUserLibrary
$$(LIB_DIR)/userlib_$$$(1).a: $$($$$(2)_OBJ_FILES) | $$(LIB_DIR)
	$$(vecho) "  AR   $$@"
	$$(Q) $$(AR) cru $$@ $$^
	@echo ""
endef

$(foreach userlib, $(MODULES), $(eval $(call MakeUserLibrary, \
							$(shell echo $(userlib) | tr A-Z a-z), $(userlib))))

## Linking rules for SDK libraries
## SDK libraries are preprocessed to:
# - Remove object files named in <libname>.remove
# - Prefix all defined symbols with 'sdk_'
# - Weaken all global symbols so they can be overriden from the open SDK side
#
# Hacky, but prevents confusing error messages if one of these files disappears
$(SDKLIB_DIR)/process/lib%.remove:
	$(Q) touch $@

# Remove comment lines from <libname>.remove files
$(LIB_DIR)/lib%.remove: $(SDKLIB_DIR)/process/lib%.remove | $(LIB_DIR)
	$(Q) grep -v "^#" $< | cat > $@

# Stage 1: remove unwanted object files listed in <libname>.remove
# longside each library
$(LIB_DIR)/sdklib_%_stage1.a: $(SDKLIB_DIR)/lib/lib%.a \
										$(LIB_DIR)/lib%.remove | $(LIB_DIR)
	@echo "SDK processing stage 1: Removing unwanted objects from $<"
	$(Q) cat $< > $@
	$(Q) $(AR) d $@ @$(word 2,$^)

# Stage 2: Redefine all SDK symbols as sdk_, weaken all symbols.
$(LIB_DIR)/sdklib_%.a: $(LIB_DIR)/sdklib_%_stage1.a \
										$(SDKLIB_DIR)/process/allsymbols.rename
	@echo "SDK processing stage 2: Renaming symbols in SDK library $< -> $@"
	$(Q) $(OBJCOPY) --redefine-syms $(word 2,$^) --weaken $< $@

$(IMAGE_FILE): $(LIB_FILES) | $(IMAGE_DIR)
	@echo ""
	$(vecho) "  LD   $@"
	$(Q) $(CC) $(LFLAGS) -o $@
	$(Q) $(SIZE) $@

$(BIN_FILE): $(IMAGE_FILE) | $(BIN_DIR)
	$(Q) $(ESPTOOL) elf2image --version=2 $(ESPTOOL_PARAMS) $< -o $@

all: $(BIN_FILE)
	@echo ""

clean:
	$(Q) $(RM) -r $(BUILD_DIR)

flash:
	$(Q) $(ESPTOOL) --port $(DEVICE_PORT) --baud $(FLASH_BAUD_RATE) erase_flash
	$(Q) $(ESPTOOL) --port $(DEVICE_PORT) --baud $(FLASH_BAUD_RATE) \
		write_flash $(ESPTOOL_PARAMS) \
		0x0000 $(RBOOT_BIN_FILE) 0x1000 $(RBOOT_CONF_FILE) 0x2000 $(BIN_FILE)

debug:
	$(Q) $(FILTEROUTPUT) --port $(DEVICE_PORT) --baud $(DEBUG_BAUD_RATE)

.PHONY:

# Prevent "intermediate" files from being deleted
.SECONDARY:

# Include the automatically generated dependency files.
sinclude $(wildcard $(OBJ_DIR)/*.d)

