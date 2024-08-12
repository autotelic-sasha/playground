call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
msbuild {{appname}}.sln /p:Configuration=Debug
msbuild {{appname}}.sln /p:Configuration=Release
