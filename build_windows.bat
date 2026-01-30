@echo off
REM ========================================
REM LogParser Windows 构建脚本
REM ========================================
REM 
REM 前置条件：
REM 1. 安装 Qt (推荐 Qt 5.15 或 Qt 6.x)
REM    下载地址: https://www.qt.io/download-qt-installer
REM    安装时选择 MinGW 或 MSVC 编译器
REM
REM 2. 安装 CMake
REM    下载地址: https://cmake.org/download/
REM
REM 使用方法：
REM 1. 打开 Qt 命令行 (Qt 5.15.x MinGW 64-bit)
REM 2. 进入项目目录
REM 3. 运行此脚本
REM ========================================

echo [1/4] 创建构建目录...
if not exist build-release mkdir build-release
cd build-release

echo [2/4] 运行 CMake 配置...
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto error

echo [3/4] 编译项目...
mingw32-make -j%NUMBER_OF_PROCESSORS%
if errorlevel 1 goto error

echo [4/4] 部署 Qt 依赖...
if not exist deploy mkdir deploy
copy LogParser.exe deploy\
cd deploy
windeployqt LogParser.exe --release --no-translations
cd ..

echo.
echo ========================================
echo 构建成功！
echo 可执行文件位于: build-release\deploy\
echo ========================================
goto end

:error
echo.
echo 构建失败！请检查错误信息。
exit /b 1

:end
pause
