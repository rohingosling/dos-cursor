@echo off
rem **************************************************************************
rem  Clean Script: cursor
rem
rem  Description:
rem
rem    Removes all build artifacts: the object file, the executable, and the
rem    build log.
rem
rem  Usage:
rem
rem    clean          Remove CURSOR.OBJ, CURSOR.EXE, BUILD.LOG
rem
rem **************************************************************************

echo Cleaning build artifacts ...

if exist cursor.obj  del cursor.obj
if exist cursor.exe  del cursor.exe
if exist build.log   del build.log

echo Done.
