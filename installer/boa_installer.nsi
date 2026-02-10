; Boa Language NSIS Installer Script
; Requires: NSIS (Nullsoft Scriptable Install System)
;
; Build with: makensis installer\boa_installer.nsi

!include "MUI2.nsh"

; General settings
Name "Boa Programming Language"
OutFile "boa-0.1.0-setup.exe"
InstallDir "$PROGRAMFILES64\Boa"
InstallDirRegKey HKLM "Software\Boa" "InstallDir"
RequestExecutionLevel admin

; Interface settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; Sections
Section "Boa Runtime (required)" SecRuntime
    SectionIn RO

    SetOutPath "$INSTDIR"
    File "..\build\Release\boa.exe"

    ; Store installation folder
    WriteRegStr HKLM "Software\Boa" "InstallDir" "$INSTDIR"

    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Add to Add/Remove Programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Boa" \
        "DisplayName" "Boa Programming Language"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Boa" \
        "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Boa" \
        "DisplayVersion" "0.1.0"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Boa" \
        "Publisher" "Boa Language Contributors"
SectionEnd

Section "Add to PATH" SecPath
    ; Add to system PATH
    ReadRegStr $0 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
    StrCpy $0 "$0;$INSTDIR"
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path" "$0"

    ; Notify system of environment change
    SendMessage ${HWND_BROADCAST} ${WM_SETTINGCHANGE} 0 "STR:Environment" /TIMEOUT=5000
SectionEnd

Section "File Association (.boa)" SecFileAssoc
    WriteRegStr HKCR ".boa" "" "BoaScript"
    WriteRegStr HKCR "BoaScript" "" "Boa Script"
    WriteRegStr HKCR "BoaScript\shell\open\command" "" '"$INSTDIR\boa.exe" "%1"'
SectionEnd

Section "Start Menu Shortcuts" SecShortcuts
    CreateDirectory "$SMPROGRAMS\Boa"
    CreateShortCut "$SMPROGRAMS\Boa\Boa REPL.lnk" "$INSTDIR\boa.exe"
    CreateShortCut "$SMPROGRAMS\Boa\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Examples" SecExamples
    SetOutPath "$INSTDIR\examples"
    File "..\examples\*.boa"
SectionEnd

; Descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecRuntime} "The Boa interpreter and REPL (required)."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPath} "Add Boa to the system PATH for command-line access."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssoc} "Associate .boa files with the Boa interpreter."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} "Create Start Menu shortcuts."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecExamples} "Install example Boa scripts."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Uninstaller
Section "Uninstall"
    Delete "$INSTDIR\boa.exe"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir /r "$INSTDIR\examples"
    RMDir "$INSTDIR"

    ; Remove Start Menu shortcuts
    Delete "$SMPROGRAMS\Boa\*.lnk"
    RMDir "$SMPROGRAMS\Boa"

    ; Remove registry entries
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Boa"
    DeleteRegKey HKLM "Software\Boa"
    DeleteRegKey HKCR ".boa"
    DeleteRegKey HKCR "BoaScript"

    ; Note: PATH removal is complex and not included here.
    ; Users should manually remove the Boa directory from PATH if needed.
SectionEnd
