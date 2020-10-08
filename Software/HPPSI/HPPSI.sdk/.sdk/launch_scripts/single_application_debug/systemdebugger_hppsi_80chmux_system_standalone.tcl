connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent Cora Z7 - 7007S 210370A92F23A" && level==0} -index 1
fpga -file C:/Users/hr193/Desktop/HPPSI/HPPSI/HPPSI.sdk/HPPSI_80chMux/_ide/bitstream/HPPSI.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw C:/Users/hr193/Desktop/HPPSI/HPPSI/HPPSI.sdk/HPPSI/export/HPPSI/hw/HPPSI.xsa -mem-ranges [list {0x40000000 0xbfffffff}]
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source C:/Users/hr193/Desktop/HPPSI/HPPSI/HPPSI.sdk/HPPSI_80chMux/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow C:/Users/hr193/Desktop/HPPSI/HPPSI/HPPSI.sdk/HPPSI_80chMux/Debug/HPPSI_80chMux.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "*A9*#0"}
con
