@echo off

REM  Set environment for the Pocket PC 2003 ARM target
REM  This batch file serves as a template only.
Title WCE ARM Environment


REM This batch file sets up an environment for building for a specific CPU from the command line.
REM The build environment defaults to an H/PC Ver. 2.00 platform. The macros that control the
REM platform are: PLATFORM, OSVERSION, WCEROOT and SDKROOT. The H/PC Ver. 2.00 default settings
REM are as follows:
REM    PLATFORM=MS HPC
REM 
REM        WCEROOT=C:\Windows CE Tools    //Root dir for VCCE
REM        SDKROOT=C:\Windows CE Tools    //Root dir for H/PC SDK
REM The Palm-size PC 2.01 default settings are:
REM    PLATFORM=ms palm size pc
REM        OSVERSION=WCE201
REM        WCEROOT=C:\Windows CE Tools    //Root dir for VCCE
REM        SDKROOT=C:\Windows CE Tools    //Root dir for PSPC SDK
REM The batch file uses these macros to set the PATH, INCLUDE, LIB macros for the default
REM platform. Please note that if the default setup options were altered during
REM installation (for example, if the install directories were changed), then the user needs
REM to modify these macros accordingly.

set OSVERSION=WCE420
set PLATFORM=pocket pc 2003
set WCEROOT=C:\Program Files\Microsoft eMbedded C++ 4.0
set SDKROOT=C:\Windows CE Tools

set PATH=%WCEROOT%\COMMON\EVC\bin;%WCEROOT%\EVC\%OSVERSION%\bin;%path%
set INCLUDE=%SDKROOT%\%OSVERSION%\%PLATFORM%\include\armv4;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\include;
set LIB=%SDKROOT%\%OSVERSION%\%PLATFORM%\lib\armv4;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\armv4;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\armv4;

set CC=clarm.exe
set TARGET_OSVERSION=420
set TARGETCPU=ARM
set CFG=ARMREL

set CEVERSION=420
set CESUBSYSTEM=windowsce,4.20
