setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
msbuild tests.sln /p:Configuration=Debug
msbuild tests.sln /p:Configuration=Release
endlocal