@echo off
REM  Set environment for the Pocket PC 2002 ARM target
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

if "%OSVERSION%"=="" set OSVERSION=WCE300
if "%PLATFORM%"=="" set PLATFORM=pocket pc 2002
if "%WCEROOT%"=="" set WCEROOT=C:\Program Files\Microsoft eMbedded Tools
if "%SDKROOT%"=="" set SDKROOT=C:\Windows CE Tools

set PATH=%WCEROOT%\COMMON\EVC\bin;%WCEROOT%\EVC\%OSVERSION%\bin;%path%
set INCLUDE=%SDKROOT%\%OSVERSION%\%PLATFORM%\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\include;
set LIB=%SDKROOT%\%OSVERSION%\%PLATFORM%\lib\arm;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\arm;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\arm;

set CC=clarm.exe
set TARGET_OSVERSION=300
set TARGETCPU=ARM
set CFG=ARMREL

set CEVERSION=300
set CESUBSYSTEM=windowsce,3.00
