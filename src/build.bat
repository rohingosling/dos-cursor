@echo off
rem **************************************************************************
rem  Build Script: cursor
rem  Compiler:     Borland Turbo C++ (bcc)
rem  Environment:  DOS / DOSBox
rem
rem  Description:
rem
rem    Compiles and links CURSOR.C into CURSOR.EXE using the small memory
rem    model (-ms) and the default 8086 code generation (the -2 / 80286 flag
rem    is intentionally omitted). All compiler and linker output is captured
rem    in BUILD.LOG for review.
rem
rem  Usage:
rem
rem    build          Build CURSOR.EXE  (all output logged to BUILD.LOG)
rem
rem **************************************************************************

echo ============================================ > build.log
echo  cursor - Build Log >> build.log
echo  Compiler: Borland Turbo C++ (bcc) >> build.log
echo  Model:    small (-ms), 8086 >> build.log
echo ============================================ >> build.log
echo. >> build.log

echo Building CURSOR.EXE ...
echo ---- bcc -ms -ecursor.exe cursor.c ---- >> build.log
bcc -ms -ecursor.exe cursor.c >> build.log
if errorlevel 1 goto fail

echo. >> build.log
echo ============================================ >> build.log
echo  Build successful: CURSOR.EXE >> build.log
echo ============================================ >> build.log

echo.
echo Build successful: CURSOR.EXE   (details in BUILD.LOG)
goto end

rem --------------------------------------------------------------------------
rem  Error
rem --------------------------------------------------------------------------

:fail
echo. >> build.log
echo *** BUILD FAILED *** >> build.log

echo.
echo Build FAILED.  See BUILD.LOG for details.

:end
