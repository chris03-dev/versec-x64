@echo off

echo Running Windows clang build script...

if not exist ..\bin\ mkdir ..\bin\
cd ..\src\
clang -target x86_64-pc-windows-gnu -Wall -s -O2 -std=c99 -Wl,--gc-sections *.c -o ..\bin\verse.exe

if %ERRORLEVEL% neq 0 (echo Build failure.) else (echo Build successful.)