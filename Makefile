## ========================================================================== ##
## --------------------------- DIRECTORIES ---------------------------------- ##
## ========================================================================== ##

## ------------------------------ COMMON ------------------------------------ ##
PROJECT_ROOT		?= ${PWD}

OUTPUT_DIR			?= ${PROJECT_ROOT}/output
PLATFORM_DIR		?= ${PROJECT_ROOT}/platform
TOOL_DIR			?= ${PROJECT_ROOT}/tools
APP_DIR				?= ${PROJECT_ROOT}/app

DRIVER_DIR			?= ${PLATFORM_DIR}/driver
ESPRESSIF_DIR		?= ${PLATFORM_DIR}/espressif
FREERTOS_DIR		?= ${PLATFORM_DIR}/freertos
JSON_DIR			?= ${PLATFORM_DIR}/json
LWIP_DIR			?= ${PLATFORM_DIR}/lwip

LIB_DIR				?= ${ESPRESSIF_DIR}/lib
LD_DIR				?= ${ESPRESSIF_DIR}/ld

OBJ_DIR				?= ${OUTPUT_DIR}/obj
IMAGE_DIR			?= ${OUTPUT_DIR}/image
BIN_DIR				?= ${OUTPUT_DIR}/bin

## ----------------------------- SOURCE ------------------------------------- ##
## APP
VPATH				:= :${APP_DIR}/src
## DRIVER
VPATH				+= :${DRIVER_DIR}/src
## ESPRESSIF
VPATH				+= :${ESPRESSIF_DIR}/upgrade
## FREERTOS
VPATH				+= :${FREERTOS_DIR}/src
## JSON
VPATH				+= :${JSON_DIR}/src
## LWIP
VPATH				+= :${LWIP_DIR}/src/api
VPATH				+= :${LWIP_DIR}/src/apps
VPATH				+= :${LWIP_DIR}/src/arch
VPATH				+= :${LWIP_DIR}/src/core
VPATH				+= :${LWIP_DIR}/src/core/ipv4
VPATH				+= :${LWIP_DIR}/src/core/ipv6
VPATH				+= :${LWIP_DIR}/src/netif

## ========================================================================== ##
## ------------------------------ FILES ------------------------------------- ##
## ========================================================================== ##
PROJECT_NAME		= SmartFarm
## LINKER
LD_FILE				:= ${LD_DIR}/eagle.app.v6.ld
## IMAGE
IMAGE_FILE			:= ${IMAGE_DIR}/${PROJECT_NAME}.app.out
## BINARY
BIN_FILE			:= ${BIN_DIR}/${PROJECT_NAME}.app.bin
## BINARY GENERATION TOOL
GEN_TOOL_FILE		:= ${TOOL_DIR}/gen_appbin.py

## ----------------------------- OBJECT ------------------------------------- ##
## APP
OBJ_FILES			:= ${OBJ_DIR}/main.o
OBJ_FILES			+= ${OBJ_DIR}/bh1750.o
OBJ_FILES			+= ${OBJ_DIR}/sht1x.o
## DRIVER
OBJ_FILES			+= ${OBJ_DIR}/gpio.o
OBJ_FILES			+= ${OBJ_DIR}/uart.o
OBJ_FILES			+= ${OBJ_DIR}/i2cm.o
## ESPRESSIF
OBJ_FILES			+= ${OBJ_DIR}/upgrade_crc32.o
OBJ_FILES			+= ${OBJ_DIR}/upgrade_lib.o
OBJ_FILES			+= ${OBJ_DIR}/upgrade.o
## FREERTOS
OBJ_FILES			+= ${OBJ_DIR}/croutine.o
OBJ_FILES			+= ${OBJ_DIR}/heap_4.o
OBJ_FILES			+= ${OBJ_DIR}/list.o
OBJ_FILES			+= ${OBJ_DIR}/port.o
OBJ_FILES			+= ${OBJ_DIR}/queue.o
OBJ_FILES			+= ${OBJ_DIR}/tasks.o
OBJ_FILES			+= ${OBJ_DIR}/timers.o
## JSON
OBJ_FILES			+= ${OBJ_DIR}/cJSON.o
## LWIP
OBJ_FILES			+= ${OBJ_DIR}/api_lib.o
OBJ_FILES			+= ${OBJ_DIR}/api_msg.o
OBJ_FILES			+= ${OBJ_DIR}/err.o
OBJ_FILES			+= ${OBJ_DIR}/netbuf.o
OBJ_FILES			+= ${OBJ_DIR}/netdb.o
OBJ_FILES			+= ${OBJ_DIR}/netifapi.o
OBJ_FILES			+= ${OBJ_DIR}/sockets.o
OBJ_FILES			+= ${OBJ_DIR}/tcpip.o
OBJ_FILES			+= ${OBJ_DIR}/sntp_time.o
OBJ_FILES			+= ${OBJ_DIR}/sntp.o
OBJ_FILES			+= ${OBJ_DIR}/time.o
OBJ_FILES			+= ${OBJ_DIR}/sys_arch.o
OBJ_FILES			+= ${OBJ_DIR}/autoip.o
OBJ_FILES			+= ${OBJ_DIR}/icmp.o
OBJ_FILES			+= ${OBJ_DIR}/igmp.o
OBJ_FILES			+= ${OBJ_DIR}/ip_frag.o
OBJ_FILES			+= ${OBJ_DIR}/ip4_addr.o
OBJ_FILES			+= ${OBJ_DIR}/ip4.o
OBJ_FILES			+= ${OBJ_DIR}/dhcp6.o
OBJ_FILES			+= ${OBJ_DIR}/ethip6.o
OBJ_FILES			+= ${OBJ_DIR}/icmp6.o
OBJ_FILES			+= ${OBJ_DIR}/inet6.o
OBJ_FILES			+= ${OBJ_DIR}/ip6_addr.o
OBJ_FILES			+= ${OBJ_DIR}/ip6_frag.o
OBJ_FILES			+= ${OBJ_DIR}/ip6.o
OBJ_FILES			+= ${OBJ_DIR}/mld6.o
OBJ_FILES			+= ${OBJ_DIR}/nd6.o
OBJ_FILES			+= ${OBJ_DIR}/def.o
OBJ_FILES			+= ${OBJ_DIR}/dhcp.o
OBJ_FILES			+= ${OBJ_DIR}/dhcpserver.o
OBJ_FILES			+= ${OBJ_DIR}/dns.o
OBJ_FILES			+= ${OBJ_DIR}/inet_chksum.o
OBJ_FILES			+= ${OBJ_DIR}/init.o
OBJ_FILES			+= ${OBJ_DIR}/lwiptimers.o
OBJ_FILES			+= ${OBJ_DIR}/mem.o
OBJ_FILES			+= ${OBJ_DIR}/memp.o
OBJ_FILES			+= ${OBJ_DIR}/netif.o
OBJ_FILES			+= ${OBJ_DIR}/pbuf.o
OBJ_FILES			+= ${OBJ_DIR}/raw.o
OBJ_FILES			+= ${OBJ_DIR}/stats.o
OBJ_FILES			+= ${OBJ_DIR}/sys.o
OBJ_FILES			+= ${OBJ_DIR}/tcp_in.o
OBJ_FILES			+= ${OBJ_DIR}/tcp_out.o
OBJ_FILES			+= ${OBJ_DIR}/tcp.o
OBJ_FILES			+= ${OBJ_DIR}/udp.o
OBJ_FILES			+= ${OBJ_DIR}/etharp.o
OBJ_FILES			+= ${OBJ_DIR}/ethernetif.o
OBJ_FILES			+= ${OBJ_DIR}/slipif.o

## ========================================================================== ##
## ------------------------------ FLAGS ------------------------------------- ##
## ========================================================================== ##

## ---------------------------- COMPILER ------------------------------------ ##
AR					= xtensa-lx106-elf-ar
CC					= xtensa-lx106-elf-gcc
NM					= xtensa-lx106-elf-nm
OBJCOPY				= xtensa-lx106-elf-objcopy
SIZE				= xtensa-lx106-elf-size

## ------------------------------- CPU -------------------------------------- ##
CFLAGS_CPU			:= -Os -g
CFLAGS_CPU			+= -MD -MP
CFLAGS_CPU			+= -Wpointer-arith
CFLAGS_CPU			+= -Wundef
CFLAGS_CPU			+= -Werror
CFLAGS_CPU			+= -Wl,-EL
CFLAGS_CPU			+= -fno-inline-functions
CFLAGS_CPU			+= -nostdlib
CFLAGS_CPU			+= -mlongcalls
CFLAGS_CPU			+= -mtext-section-literals
CFLAGS_CPU			+= -ffunction-sections
CFLAGS_CPU			+= -fdata-sections
CFLAGS_CPU			+= -fno-builtin-printf
CFLAGS_CPU			+= -fno-aggressive-loop-optimizations
#CFLAGS_CPU			+= -Wall

## ------------------------------ MACRO ------------------------------------- ##
CFLAGS_DEF			:= -D ICACHE_FLASH -D MEMLEAK_DEBUG
CFLAGS_DEF			+= -D LWIP_OPEN_SRC
CFLAGS_DEF			+= -D PBUF_RSV_FOR_WLAN -D EBUF_LWIP

## ---------------------------- INCLUSION ----------------------------------- ##
## APP
CFLAGS_INC			:= -I ${APP_DIR}/include
## DRIVER
CFLAGS_INC			+= -I ${DRIVER_DIR}/include
## ESPRESSIF
CFLAGS_INC			+= -I ${ESPRESSIF_DIR}/include
CFLAGS_INC			+= -I ${ESPRESSIF_DIR}/include/esp8266
CFLAGS_INC			+= -I ${ESPRESSIF_DIR}/include/xtensa
## FREERTOS
CFLAGS_INC			+= -I ${FREERTOS_DIR}/include
## JSON
CFLAGS_INC			+= -I ${JSON_DIR}/include
## LWIP
CFLAGS_INC			+= -I ${LWIP_DIR}/include
CFLAGS_INC			+= -I ${LWIP_DIR}/include/ipv4
CFLAGS_INC			+= -I ${LWIP_DIR}/include/ipv6

## ------------------------ C COMPILER OPTION ------------------------------- ##
CFLAGS				= ${CFLAGS_CPU} ${CFLAGS_DEF} ${CFLAGS_INC}

## ----------------------------- LINKER ------------------------------------- ##
LD_FLAGS			:= -L${LIB_DIR}
LD_FLAGS			+= -Wl,--gc-sections
LD_FLAGS			+= -nostdlib
LD_FLAGS			+= -T${LD_FILE}
LD_FLAGS			+= -Wl,--no-check-sections
LD_FLAGS			+= -u call_user_start
LD_FLAGS			+= -Wl,-static
LD_FLAGS			+= -Wl,--start-group
LD_FLAGS			+= -lcirom
LD_FLAGS			+= -lmirom
LD_FLAGS			+= -lgcc
LD_FLAGS			+= -lhal
LD_FLAGS			+= -lphy
LD_FLAGS			+= -lpp
LD_FLAGS			+= -lnet80211
LD_FLAGS			+= -lcrypto
LD_FLAGS			+= -lwpa
LD_FLAGS			+= -lmain
LD_FLAGS			+= ${OBJ_FILES}
LD_FLAGS			+= -Wl,--end-group

## ========================================================================== ##
## ----------------------------- TARGETS ------------------------------------ ##
## ========================================================================== ##
FREQ_DIV			= 0
BAUD_RATE			= 460800
# SPI_MODE:
#  0 = QIO
#  1 = QOUT
#  2 = DIO
#  3 = DOUT
SPI_MODE			= 0
# SIZE_MAP:
#  0 = 512KB  (256KB  + 256KB)
#  2 = 1024KB (512KB  + 512KB)
#  3 = 2048KB (512KB  + 512KB)
#  4 = 4096KB (512KB  + 512KB)
#  5 = 2048KB (1024KB + 1024KB)
#  6 = 4096KB (1024KB + 1024KB)
SIZE_MAP			= 4

.PHONY:

all: ${OBJ_FILES} ${IMAGE_FILE} ${BIN_FILE}

${OBJ_DIR}/%.o: %.c
	@mkdir -p ${OBJ_DIR}
	@echo "  CC   ${<}"
	@${CC} ${CFLAGS} -o ${@} -c ${<}

${IMAGE_FILE}: ${OBJ_FILES}
	@mkdir -p ${IMAGE_DIR}
	@echo "  LD   ${@}"
	@${CC} ${LD_FLAGS} -o ${@}
	@${SIZE} ${@} 

${BIN_FILE}: ${IMAGE_FILE}
	@mkdir -p ${BIN_DIR}

	@${OBJCOPY} --only-section .text -O binary ${<} eagle.app.v6.text.bin
	@${OBJCOPY} --only-section .data -O binary ${<} eagle.app.v6.data.bin
	@${OBJCOPY} --only-section .rodata -O binary ${<} eagle.app.v6.rodata.bin
	@${OBJCOPY} --only-section .irom0.text -O binary ${<} eagle.app.v6.irom0text.bin

	@python ${GEN_TOOL_FILE} ${<} 0 ${SPI_MODE} ${FREQ_DIV} ${SIZE_MAP}
	@mv eagle.app.flash.bin ${BIN_DIR}/${PROJECT_NAME}.flash.bin
	@mv eagle.app.v6.irom0text.bin ${BIN_DIR}/${PROJECT_NAME}.irom0text.bin
	@rm eagle.app*
	@echo ""

clean:
	@${RM} -r ${OBJ_DIR}
	@${RM} -r ${IMAGE_DIR}
	@${RM} ${BIN_DIR}/${PROJECT_NAME}*

flash:
	@esptool.py --port /dev/ttyUSB0 --baud ${BAUD_RATE} erase_flash
	@esptool.py --port /dev/ttyUSB0 --baud ${BAUD_RATE} write_flash \
	--flash_mode qio --flash_freq 80m --flash_size 32m \
	0x00000 ${BIN_DIR}/${PROJECT_NAME}.flash.bin \
	0x10000 ${BIN_DIR}/${PROJECT_NAME}.irom0text.bin \
	0x3FC000 ${BIN_DIR}/esp_init_data_default.bin

debug:
	@sudo minicom --device /dev/ttyUSB0 --baudrate ${BAUD_RATE}

flash_and_debug: flash debug

# Include the automatically generated dependency files.
sinclude ${wildcard ${OBJ_DIR}/*.d}

