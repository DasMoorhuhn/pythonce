#
#	Announce what we are building
#
!IF "$(BUILD)" != ""
!MESSAGE Building $(BUILD)
!ENDIF

#
#	Common make definitions for ALL builds
#
!IF "$(CFG)" == "" || "$(CFG)" == "none"
CFG=ARMREL
!MESSAGE No configuration specified. Defaulting to $(CFG)
!ENDIF 

!IF "$(CESUBSYSTEM)" == ""
CESubsystem=windowsce,3.00
!MESSAGE Variable CESubsystem not specified. Defaulting to windowsce,3.00
!ELSE
CESubsystem=$(CESUBSYSTEM)
!ENDIF 

!IF "$(CEVERSION)" == ""
CEVersion=300
!MESSAGE Variable CEVersion not specified. Defaulting to 300
!ELSE
CEVersion=$(CEVERSION)
!ENDIF 

!IF "$(CE_PLATFORM)"==""
CePlatform=WIN32_PLATFORM_PSPC=$(CEVERSION)
!ELSE 
CePlatform=$(CE_PLATFORM)
!ENDIF 

NMAKE=nmake /nologo

RSC=rc.exe

SWIG = $(SWIGROOT)\swig -python -I$(SWIGROOT)\swig_lib -DMS_WINCE -d nul:

!IF "$(CFG)" != "MIPSREL" && "$(CFG)" != "MIPSDEB" && "$(CFG)" != "SH4REL" && "$(CFG)" != "SH4DEB" && "$(CFG)" != "SH3REL" && "$(CFG)" != "SH3DEB" && "$(CFG)" != "ARMREL" && "$(CFG)" != "ARMDEB" && "$(CFG)" != "X86REL" && "$(CFG)" != "X86DEB"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f makefile.pythoncore CFG="ARMREL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MIPSREL (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "MIPSDEB" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "SH4REL" (based on "Win32 (WCE SH4) Dynamic-Link Library")
!MESSAGE "SH4DEB" (based on "Win32 (WCE SH4) Dynamic-Link Library")
!MESSAGE "SH3REL" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE "SH3DEB" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE "ARMREL" (based on "Win32 (WCE ARM) Dynamic-Link Library")
!MESSAGE "ARMDEB" (based on "Win32 (WCE ARM) Dynamic-Link Library")
!MESSAGE "X86REL" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE "X86DEB" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

#
#	Python Version (2.3)
#
PYTHON_VERSION=23

#
#	Common Compiler Flags
#
CFLAGS = /I ..\..\Include /I ..\..\PC\WinCE \
	 /D Py_ENABLE_SHARED \
	 /D _WIN32_WCE=$(CEVersion) /D $(CePlatform) /D UNDER_CE=$(CEVersion) \
	 /D UNICODE /D _UNICODE /D _USRDLL /D WIN32PPC /D WIN32 \
	 /D STRICT /D INTERNATIONAL /D USA /D INTLMSG_CODEPAGE \
	 /nologo /W3 /Fp"$(INTDIR)\$(RESULT).pch" /Fo"$(INTDIR)\\" /c
CFLAGS_DLL = /I ..\..\Python /D Py_BUILD_CORE /D PYTHONCORE_EXPORTS
CFLAGS_DEBUG=/D "DEBUG" /Zi /Fd"$(INTDIR)\\" /YX
LINK32=link.exe

#
#	Include the platform specific stuff
#
!INCLUDE $(CFG).mk

#
#	Now we can cobble up the definitions
#
OUTDIR=binaries\$(PLATFORM_OUTDIR)\$(BUILD)
INTDIR=binaries\$(PLATFORM_INTDIR)\$(BUILD)
CPP=$(PLATFORM_CPP)
CPP_PROJ=$(PLATFORM_CPP_PROJ)
LINK32_FLAGS=$(PLATFORM_LINK32_FLAGS)
LINK32_DLL_FLAGS=$(PLATFORM_LINK32_DLL_FLAGS)
PYTHONCORE_LIB=binaries\$(PLATFORM_OUTDIR)\pythoncore\python$(PYTHON_VERSION)$(PLATFORM_DLL_SUFFIX).lib
