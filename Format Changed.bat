@echo off
rem Formats only the C++ files changed since the last commit (modified, staged or new),
rem with the repository's .clang-format and the bundled clang-format.
rem Third-party code (Vendor folders, Opcode, ImGui, stb, tinyxml2, json) is skipped.
setlocal EnableDelayedExpansion
cd /d "%~dp0"

set "CLANGFORMAT=.\Vendor\clang-format.exe"
if not exist "%CLANGFORMAT%" (
	echo clang-format not found at %CLANGFORMAT%
	exit /b 1
)

set /a COUNT=0
for /f "usebackq delims=" %%F in (`git diff HEAD --name-only --diff-filter=ACMR -- "*.cpp" "*.h" 2^>nul ^& git ls-files --others --exclude-standard -- "*.cpp" "*.h" 2^>nul`) do (
	set "FILE=%%F"
	set "SKIP="
	echo !FILE! | findstr /i /c:"/Vendor/" /c:"/ThirdParty/" /c:"Opcode" /c:"imgui" /c:"imstb" /c:"stb_" /c:"tinyxml2" /c:"json.hpp" >nul && set "SKIP=1"
	if not defined SKIP if exist "!FILE!" (
		"%CLANGFORMAT%" -i "!FILE!"
		set /a COUNT+=1
	)
)

echo Formatted !COUNT! file(s).
endlocal
