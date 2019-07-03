debug\WiSim.exe -technology phs       -f sectors.cgeo
debug\WiSim.exe -technology wlan      -f sectors_wlan.cgeo
debug\WiSim.exe -technology cdma2000  -f sectors_cdma2000.cgeo
debug\WiSim.exe -technology wcdma     -f sectors_wcdma.cgeo

debug\WiSim.exe -technology phs       -init runcvg.ccmd
debug\WiSim.exe -technology wlan      -init runcvg_wlan.ccmd  
debug\WiSim.exe -technology cdma2000  -init runcvg_cdma2000.ccmd  
debug\WiSim.exe -technology wcdma     -init runcvg_wcdma.ccmd

gdb debug\WiSim.exe
set args -technology wlan -f sectors_wlan.cgeo
set args -technology cdma2000 -f sectors_cdma2000.cgeo

