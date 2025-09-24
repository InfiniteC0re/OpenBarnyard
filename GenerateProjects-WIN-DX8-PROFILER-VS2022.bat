@echo off
.\vendor\premake\premake5.exe vs2022 --arch=x86 --renderer=DX8 --profiler=perfmem
pause