@echo off
if not defined DevEnvDir call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" x86
call make.exe %*
