@echo off

set code=%cd%
set application_name=server_lobby
set warnings= /W4 /wd4211 /wd4189 /wd4533 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4457 /WX
set compile_flags= -FC -GR- -EHa- -nologo -Zi %warnings% /I "%code%/source/" /I "%code%/extern/include/"
set lobby_link_flags= Ws2_32.lib gdi32.lib user32.lib Kernel32.lib winmm.lib -incremental:no


if not exist build mkdir build
pushd build

start /b /wait "" "cl.exe" %compile_flags% ../source/server_lobby/lobby_main.cpp /link %lobby_link_flags% /out:%application_name%.exe

popd
