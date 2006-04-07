#
#	ARM Release
#
PLATFORM_OUTDIR=ARMRel$(TARGET_OSVERSION)
PLATFORM_INTDIR=ARMRel$(TARGET_OSVERSION)
PLATFORM_CPP=clarm.exe
PLATFORM_CPP_PROJ=/O2 /D "ARM" /D "_ARM_" /D "NDEBUG" $(CFLAGS) /D /Oxs 
PLATFORM_LINK32_FLAGS=/subsystem:$(CESubsystem) /align:"4096" /MACHINE:ARM 
PLATFORM_LINK32_DLL_FLAGS=/base:"0x00100000" /entry:"_DllMainCRTStartup" $(PLATFORM_LINK32_FLAGS)
PLATFORM_RSCFLAGS=/d "ARM" /d "_ARM_" /r  /d "NDEBUG"

