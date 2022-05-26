@echo off

set code=%cd%
set application_name=Lith
set build_options= -DBUILD_WIN32=1
set warnings= /W4 /wd4211 /wd4189 /wd4533 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4457 /WX
set compile_flags= -FC -GR- -EHa- -nologo -Zi %warnings% /I "%code%/code/"
set common_link_flags= opengl32.lib -incremental:no
set platform_link_flags= Ws2_32.lib gdi32.lib user32.lib Kernel32.lib winmm.lib %common_link_flags%
set app_link_flags= /LIBPATH:"%code%/code/third_party" %common_link_flags%


if not exist build mkdir build
pushd build
del %application_name%_*.pdb > NUL 2> NUL
set time_stamp=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%

start /b /wait "" "cl.exe"  %build_options% %compile_flags% ../code/game/game_main.cpp /LD /link %app_link_flags% /out:%application_name%_code.dll  -PDB:%application_name%_%time_stamp: =%.pdb

start /b /wait "" "cl.exe"  %build_options% %compile_flags% ../code/os/win32/win32_main.cpp /link %platform_link_flags% /out:%application_name%.exe

if not exist game1 (
 mkdir game1 
 cp -r ..\data game1
)

if not exist game2 (
 mkdir game2
 cp -r ..\data game2
)

if not exist game3 (
 mkdir game3 
 cp -r ..\data game3
)

if not exist game4 (
 mkdir game4 
 cp -r ..\data game4
)

cp %application_name%_code.dll %application_name%.exe game1
cp %application_name%_code.dll %application_name%.exe game2
cp %application_name%_code.dll %application_name%.exe game3

popd

