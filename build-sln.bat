@echo off
set platform=%1
set configuration=%2
set "CMD_MSBUILD=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"

if "%platform%" == "Win32" (
	@rem OK
) else if "%platform%" == "x64" (
	@rem OK
) else (
	exit /b 1
)

if "%configuration%" == "Release" (
	@rem OK
) else if "%configuration%" == "Debug" (
	@rem OK
) else (
	exit /b 1
)

set SLN_FILE=EXE\Backup.sln

@rem https://www.appveyor.com/docs/environment-variables/

if "%APPVEYOR%"=="True" (
    set EXTRA_CMD=/verbosity:minimal /logger:"%ProgramFiles%\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
) else (
    set EXTRA_CMD=
)


@echo "%CMD_MSBUILD%" %SLN_FILE% /p:Platform=%platform% /p:Configuration=%configuration%  /t:"Build" %EXTRA_CMD%
      "%CMD_MSBUILD%" %SLN_FILE% /p:Platform=%platform% /p:Configuration=%configuration%  /t:"Build" %EXTRA_CMD%
if errorlevel 1 (
	echo ERROR in msbuild.exe errorlevel %errorlevel%
	exit /b 1
)

exit /b 0
