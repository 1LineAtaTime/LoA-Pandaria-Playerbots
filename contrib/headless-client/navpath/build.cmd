@echo off
rem Builds navpath.exe into build\ with the newest Visual Studio C++ toolchain and Ninja.
rem Usage: build.cmd [jobs]   (default 4 parallel jobs)
setlocal
set "JOBS=%~1"
if "%JOBS%"=="" set "JOBS=4"
set "HERE=%~dp0"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo vswhere.exe not found; install Visual Studio 2022 with the C++ workload 1>&2
  exit /b 1
)
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR (
  echo no Visual Studio with the C++ x64 tools found 1>&2
  exit /b 1
)
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
where ninja >nul 2>nul
if errorlevel 1 (
  echo ninja not found on PATH 1>&2
  exit /b 1
)
cmake -S "%HERE%." -B "%HERE%build" -G Ninja -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build "%HERE%build" -j %JOBS% || exit /b 1
echo built "%HERE%build\navpath.exe"
