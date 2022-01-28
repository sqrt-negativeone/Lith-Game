@echo off

set application_name=app
set build_options= -DBUILD_WIN32=1
set warnings= /W4 /wd4211 /wd4189 /wd4533 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4457 /WX
set compile_flags= -FC -GR- -EHa- -nologo -Zi %warnings% /I ../source/ /I ../extern/include/
set common_link_flags= opengl32.lib -incremental:no
set platform_link_flags= gdi32.lib user32.lib Kernel32.lib winmm.lib %common_link_flags%

if not exist build mkdir build
pushd build
del app_*.pdb > NUL 2> NUL
set time_stamp=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%

start /b /wait "" "cl.exe"  %build_options% %compile_flags% ../source/win32/win32_main.cpp /link %platform_link_flags% /out:%application_name%.exe

start /b /wait "" "cl.exe"  %build_options% %compile_flags% ../source/app.cpp /LD /link %common_link_flags% /out:%application_name%_code.dll  -PDB:%application_name%_%time_stamp: =%.pdb
popd
