cd /d "%~dp0"

mkdir build

cd build

cmake .. -G "Visual Studio 16 2019" -A x64

pause