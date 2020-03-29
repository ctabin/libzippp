
@echo off

SET root=%cd%
SET zlib=lib\zlib-1.2.11
SET libzip=lib\libzip-1.6.1

if not exist "%zlib%" goto error_zlib_not_found
if not exist "%libzip%" goto error_libzip_not_found

:compile_zlib
if exist "%zlib%\build" goto compile_libzip
echo Compiling zlib...
cd "%zlib%"
mkdir build
cd "build"
cmake .. -DCMAKE_INSTALL_PREFIX="%root%/lib/install"
if %ERRORLEVEL% GEQ 1 goto error_zlib
cmake --build . --config Debug --target install
if %ERRORLEVEL% GEQ 1 goto error_zlib
cmake --build . --config Release --target install
if %ERRORLEVEL% GEQ 1 goto error_zlib
cd "..\..\.."

:compile_libzip
if exist "%libzip%\build" goto prepare_libzippp
echo Compiling libzip...
cd "%libzip%"
mkdir build
cd "build"
cmake .. -DCMAKE_INSTALL_PREFIX="%root%/lib/install" -DCMAKE_PREFIX_PATH="%root%/lib/install" -DENABLE_COMMONCRYPTO=OFF -DENABLE_GNUTLS=OFF -DENABLE_MBEDTLS=OFF -DENABLE_OPENSSL=OFF -DENABLE_WINDOWS_CRYPTO=ON -DBUILD_TOOLS=OFF -DBUILD_REGRESS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_DOC=OFF
if %ERRORLEVEL% GEQ 1 goto error_libzip
cmake --build . --config Debug --target install
if %ERRORLEVEL% GEQ 1 goto error_libzip
cmake --build . --config Release --target install
if %ERRORLEVEL% GEQ 1 goto error_libzip
cd "..\..\.."

:prepare_libzippp
echo Compiling lizippp...
rmdir /q /s "build"
mkdir build
cd "build"
cmake .. -DCMAKE_PREFIX_PATH="%root%/lib/install"
if %ERRORLEVEL% GEQ 1 goto error_libzippp
cd ".."

:compile_libzippp
cmake --build build --config Debug
if %ERRORLEVEL% GEQ 1 goto error_libzippp
cmake --build build --config Release
if %ERRORLEVEL% GEQ 1 goto error_libzippp
cd ".."

:package_libzippp
if exist "dist\libzippp_static.lib" goto end
mkdir "dist"
cd "dist"
mkdir release
copy ..\src\libzippp.h release
copy ..\build\Release\libzippp_shared_test.exe release
copy ..\build\Release\libzippp_static_test.exe release
copy ..\build\Release\libzippp.dll release
copy ..\build\Release\libzippp.lib release
copy ..\build\Release\libzippp_static.lib release
copy ..\%zlib%\build\Release\zlib.dll release
copy ..\%libzip%\build\lib\Release\zip.dll release
mkdir debug
copy ..\src\libzippp.h debug
copy ..\build\Debug\libzippp_shared_test.exe debug
copy ..\build\Debug\libzippp_static_test.exe debug
copy ..\build\Debug\libzippp.dll debug
copy ..\build\Debug\libzippp.lib debug
copy ..\build\Debug\libzippp_static.lib debug
copy ..\%zlib%\build\Debug\zlibd.dll debug
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
cd %root%
cmd
