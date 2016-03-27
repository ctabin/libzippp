
@echo off

SET vs2012devprompt=C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Tools\VsDevCmd.bat
SET zlib=lib\zlib-1.2.8
SET libzip=lib\libzip-1.1.2

if not exist "%zlib%" goto error_zlib_not_found
if not exist "%libzip%" goto error_libzip_not_found
if not exist "%vs2012devprompt%" goto error_vs2012_not_found

call "%vs2012devprompt%"

:compile_zlib
if exist "%zlib%\build" goto compile_libzip
echo Compiling zlib...
cd "%zlib%"
mkdir build
cd "build"
cmake .. -G"Visual Studio 11" -DCMAKE_INSTALL_PREFIX="install"
if %ERRORLEVEL% GEQ 1 goto error_zlib
msbuild /P:Configuration=Debug INSTALL.vcxproj
if %ERRORLEVEL% GEQ 1 goto error_zlib
msbuild /P:Configuration=Release INSTALL.vcxproj
if %ERRORLEVEL% GEQ 1 goto error_zlib
cd "..\..\.."

:compile_libzip
if exist "%libzip%\build" goto prepare_libzippp
echo Compiling libzip...
cd "%libzip%"
mkdir build
cd "build"
cmake .. -G"Visual Studio 11" -DCMAKE_PREFIX_PATH="../../%zlib%/build/install"
if %ERRORLEVEL% GEQ 1 goto error_libzip
msbuild /P:Configuration=Debug ALL_BUILD.vcxproj
if %ERRORLEVEL% GEQ 1 goto error_libzip
msbuild /P:Configuration=Release ALL_BUILD.vcxproj
if %ERRORLEVEL% GEQ 1 goto error_libzip
cd "..\..\.."

:prepare_libzippp
if exist "build" goto compile_libzippp
echo Compiling lizippp...
mkdir build
cd "build"
cmake .. -G"Visual Studio 11" -DCMAKE_PREFIX_PATH="%zlib%/build/install"
if %ERRORLEVEL% GEQ 1 goto error_libzippp
cd ".."

:compile_libzippp
cd "build"
if exist "libzippp_static.lib" goto package_libzippp
msbuild /P:Configuration=Debug ALL_BUILD.vcxproj
if %ERRORLEVEL% GEQ 1 goto error_libzippp
msbuild /P:Configuration=Release ALL_BUILD.vcxproj
if %ERRORLEVEL% GEQ 1 goto error_libzippp
cd ".."

:package_libzippp
if exist "dist\libzippp_static.lib" goto end
mkdir "dist"
cd "dist"
mkdir release
copy ..\build\Release\libzippp_shared_test.exe release
copy ..\build\Release\libzippp_static_test.exe release
copy ..\build\Release\libzippp.dll release
copy ..\build\Release\libzippp.lib release
copy ..\build\Release\libzippp_static.lib release
copy ..\%zlib%\build\Release\zlib.dll release
copy ..\%libzip%\build\lib\Release\zip.dll release
mkdir debug
copy ..\build\Debug\libzippp_shared_test.exe debug
copy ..\build\Debug\libzippp_static_test.exe debug
copy ..\build\Debug\libzippp.dll debug
copy ..\build\Debug\libzippp.lib debug
copy ..\build\Debug\libzippp_static.lib debug
copy ..\%zlib%\build\Release\zlib.dll debug
copy ..\%libzip%\build\lib\Debug\zip.dll debug
cd ..

goto end

:error_zlib_not_found
echo [ERROR] The path was not found: %zlib%.
echo         You have to download zlib 1.2.8 and put in the folder %zlib%.
goto end

:error_zlib
echo [ERROR] Unable to compile zlib
goto end

:error_libzip_not_found
echo [ERROR] The path was not found: %libzip%.
echo         You have to download libzip 1.0.1 and put it in the folder %libzip%.
goto end

:error_libzip
echo [ERROR] Unable to compile libzip
goto end

:error_vs2012_not_found
echo [ERROR] VS2012 was not found (path not found: %vs2012devprompt%).
goto end

:error_libzippp
echo [ERROR] Unable to compile libzippp
goto end

:end
cmd
