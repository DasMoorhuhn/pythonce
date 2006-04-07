#
#	Top-level makefile:  Make the individual components
#

#
#	How we do makes
#
NMAKE=nmake /nologo

#
#	This is the list of Python external components we know how to build
#
EXTERNAL_COMPONENTS = \
		pywin32	   \

#
#	This is the list of Python core components we know how to build
#
CORE_COMPONENTS = \
		pythoncore \
	        python     \
		lib	   \
		select	   \
		unicodedata\
		winsound   \
		_socket	   \
		_testcapi  \
		_tkinter   \
		pyexpat

#
#	Now make a list of all the possible things we can build
#
ALL_COMPONENTS = $(CORE_COMPONENTS) $(EXTERNAL_COMPONENTS)

#
#	The default target is the core components
#
CORE: $(CORE_COMPONENTS)

#
#	This is the target if we want to make everything
#
ALL: $(ALL_COMPONENTS)

#
#	Python Core DLL: Use its makefile
#
pythoncore:
	@$(NMAKE) -f makefile.pythoncore

#
#	Python Executable: Use its makefile
#
python:
	@$(NMAKE) -f makefile.python

#
#	Python library modules
#
lib:
	@$(NMAKE) -f makefile.lib

#
#	_tkinter: The makefile is here inline
#
INCLUDE_DIRECTIVE=!INCLUDE
_tkinter:
	@$(NMAKE) -f <<
MODULE=_tkinter
ADDITIONAL_CFLAGS = /I ..\..\PC\WinCE\_tkinter
ADDITIONAL_OBJECTS=$$(INTDIR)\dynload_tcltk.obj
$(INCLUDE_DIRECTIVE) makefile.pyd
SOURCE=..\..\PC\WinCE\_tkinter\dynload_tcltk.c
$$(INTDIR)\dynload_tcltk.obj: $$(SOURCE)
  $$(CPP) $$(CPP_PROJ) $$(SOURCE)
<<

#
#	External components
#
pywin32:
	@cd External-Components\PyWin32
	@$(NMAKE)
	@cd ..\..

#
#	The winsock library we use depends on TARGET_OSVERSION
#
!IF $(TARGET_OSVERSION) <= 300
WINSOCK_LIBRARY=winsock.lib
!ELSE
WINSOCK_LIBRARY=ws2.lib
!ENDIF

pyexpat:
	@$(NMAKE) -f makefile.pyexpat

#
#	Simple modules -- just invoke makefile.pyd with module name
#
select:
	@$(NMAKE) -f makefile.pyd MODULE=select SOURCE_FILE=selectmodule.c LIBRARIES=$(WINSOCK_LIBRARY)
unicodedata:
	@$(NMAKE) -f makefile.pyd MODULE=unicodedata
winsound:
	@$(NMAKE) -f makefile.pyd MODULE=winsound SOURCE_DIRECTORY=..\..\PC
_socket:
	@$(NMAKE) -f makefile.pyd MODULE=_socket SOURCE_FILE=socketmodule.c LIBRARIES=$(WINSOCK_LIBRARY)
_testcapi:
	@$(NMAKE) -f makefile.pyd MODULE=_testcapi SOURCE_FILE=_testcapimodule.c
#
#	Clean up everything
#
clean:
	@$(NMAKE) -f makefile.lib clean
	@$(NMAKE) -f makefile.pythoncore clean
	@$(NMAKE) -f makefile.python clean
	@$(NMAKE) -f makefile.pyexpat clean
	@$(NMAKE) -f makefile.pyd MODULE=select clean
	@$(NMAKE) -f makefile.pyd MODULE=unicodedata clean
	@$(NMAKE) -f makefile.pyd MODULE=winsound clean
	@$(NMAKE) -f makefile.pyd MODULE=_socket clean
	@$(NMAKE) -f makefile.pyd MODULE=_testcapi clean
	@$(NMAKE) -f makefile.pyd MODULE=_tkinter clean
	@cd External-Components
	@cd PyWin32
	@-$(NMAKE) -f makefile clean
	@cd ..
	@cd ..


#
#	These PYCs are not built because they need external sources that we don't yet have
#
#	_bsddb	- Needs the bsd db library
#	bz2	- Needs the bz2 library
#	_ssl	- Needs the openssl library

