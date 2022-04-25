@ECHO ON
@setlocal

rem
rem Edit this line to run the batch file for Qt environment.
rem

set SHOGIMARU_VERSION=1.3
set BASEDIR=%~dp0
set TARGET=shogimaru-%SHOGIMARU_VERSION%

call "C:\Qt\6.2.3\msvc2019_64\bin\qtenv2.bat"

set VSVER=2019
set ARCH=amd64
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set VCVARSBAT=""

if exist %VSWHERE% (
  for /f "usebackq tokens=*" %%i in (`%VSWHERE% -find **\vcvarsall.bat`) do (
    echo %%i | find "%VSVER%" >NUL
    if not ERRORLEVEL 1 (
      set VCVARSBAT="%%i"
      goto :break
    )
  )
)
:break

if exist %VCVARSBAT% (
  echo %VCVARSBAT% %ARCH%
  call %VCVARSBAT% %ARCH%
) else (
  echo Error!  Compiler not found.
  pause
  exit /b
)

cd /D %BASEDIR%
qmake -r
nmake distclean
qmake -r CONFIG+=release
nmake

rd /s /q dist
md dist
copy /b /y shogimaru.exe dist\
xcopy /y /i /s /r assets dist\assets
rd /s /q dist\assets\YaneuraOu
copy /a /y docs\index.md dist\README.md

cd dist
windeployqt shogimaru.exe
move /y vc_redist.x64.exe ..

cd ..
move /y dist %TARGET%
powershell compress-archive %TARGET% %TARGET%_windows.zip
rd /s /q %TARGET%

pause
exit /b