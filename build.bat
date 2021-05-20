@echo off

IF NOT EXIST build mkdir build
pushd build
call vcvarsall.bat x64

REM 64-bit build.

del *.pdb > NUL 2> NUL

echo Waiting to load code > lock.tmp

cl /Oi /MTd /Zi /FC /nologo /W4 /WX /wd4100 /wd4505 /wd4189 /wd4201 ..\source\playground.cpp /LD /link /incremental:no /PDB:playground_%random%.pdb /EXPORT:PlaygroundUpdateAndRender

del lock.tmp

cl /MTd /Zi /FC /nologo /W4 /WX /wd4100 /wd4505 /wd4189 /wd4201 ..\source\win32_playground.cpp /link /incremental:no /opt:ref user32.lib gdi32.lib winmm.lib

mt -nologo -manifest ..\build\win32_playground.exe.manifest -outputresource:..\build\win32_playground.exe;1

popd
