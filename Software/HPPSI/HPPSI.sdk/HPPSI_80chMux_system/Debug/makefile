#
# This file was created by Xilinx Vitis tool.
#


#
# tools
#

RM = rm -f
RMDIR = rm -rf

#
# variables
#

SDCARD_GEN = sdcard_gen
GENERATE_BIF = ::scw::generate_bif

XPFM_PATH = C:/Users/hr193/Desktop/HPPSI/HPPSI/HPPSI.sdk/HPPSI/export/HPPSI/HPPSI.xpfm
SYS_CONFIG = HPPSI
DOMAINS = standalone_domain
BIF_PATH = C:/Users/hr193/Desktop/HPPSI/HPPSI/HPPSI.sdk/HPPSI_80chMux_system/Debug/system.bif

BITSTREAM_PATH = C:/Users/hr193/Desktop/HPPSI/HPPSI/HPPSI.sdk/HPPSI_80chMux/_ide/bitstream/HPPSI.bit
SD_FILES = --elf C:/Users/hr193/Desktop/HPPSI/HPPSI/HPPSI.sdk/HPPSI_80chMux/Debug/HPPSI_80chMux.elf,ps7_cortexa9_0

#
# primary build targets
#

all: sd_card

sd_card:
	-@echo Generating bif file for the system project
	generate_system_bif.bat 49570 ${XPFM_PATH} ${DOMAINS} ${BIF_PATH}
	${SDCARD_GEN} --xpfm ${XPFM_PATH} --sys_config ${SYS_CONFIG} --bif ${BIF_PATH} --bitstream ${BITSTREAM_PATH} ${SD_FILES}

clean:
	$(RMDIR) ./_sds
	${RMDIR} sd_card
	${RM} system.bif

.PHONY: all sd_card clean


