set "CMD_HHC=%ProgramFiles(x86)%\HTML Help Workshop\hhc.exe"
set HHP_PROJ=EXE\htmlhelp\Backup.HHP

@rem hhc.exe returns 1 on success, and returns 0 on failure
"%CMD_HHC%" %HHP_PROJ%
if not errorlevel 1 (
	echo error %HHP_PROJ% errorlevel %errorlevel%
	"%CMD_HHC%" %HHP_PROJ%
)
if not errorlevel 1 (
	echo error %HHP_PROJ% errorlevel %errorlevel%
)
exit /b 0
