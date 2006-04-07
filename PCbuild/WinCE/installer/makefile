
# TODO: add dependencies for all the files that are in the .cab

# FIXME: find a way to avoid hardcoding this path
CEAPPMGR=c:\program files\microsoft activesync\ceappmgr.exe

# Assume current directory or PATH for now
EZSETUP=ezsetup.exe

# FIXME: don't harcode this
CABPLATFORM=PPC2003_ARM 


!IF "$(SDKROOT)" == ""
!ERROR Variable SDKROOT not set (e.g. SDKROOT=c:\program files\windows ce tools)
!ENDIF

!IF "$(OSVERSION)" == ""
!ERROR Variable OSVERSION not set (e.g. OSVERSION=WCE420)
!ENDIF

!IF "$(PLATFORM)" == ""
!ERROR Variable PLATFORM not set (e.g. PLATFORM=pocket pc 2003)
!ENDIF

# e.g. c:\program files\windows ce tools\wce420\pocket pc 2003
SDKDIR=$(SDKROOT)\$(OSVERSION)\$(PLATFORM)

CABWIZ=$(SDKDIR)\Tools\cabwiz.exe

# NOTE: the first part before the dot must be <= 8 characters
CABNAME=PythonCE.$(CABPLATFORM)
CABFILE=$(CABNAME).cab
# I think this .dat file is just an intermediate file
DATFILE=$(CABNAME).dat

SETUPEXE=PythonCESetup.exe

# Input files:
INFFILE=PythonCE.inf
INIFILE=PythonCE.ini

all: $(CABFILE) $(SETUPEXE)

$(CABFILE) : $(INFFILE)
	"$(CABWIZ)" "$(INFFILE)" /cpu $(CABPLATFORM)

$(SETUPEXE) : $(CABFILE) eula.txt readme.txt $(INIFILE)
	$(EZSETUP) -l english -i $(INIFILE) -r readme.txt -e eula.txt -o $(SETUPEXE)

clean:
	del $(CABFILE)
	del $(DATFILE)
	del $(SETUPEXE)

