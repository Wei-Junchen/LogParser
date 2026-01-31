; ========================================
; LogParser Windows Installer Script
; NSIS (Nullsoft Scriptable Install System)
; Download NSIS: https://nsis.sourceforge.io/
; ========================================

!include "MUI2.nsh"

; Basic Info
Name "LogParser"
OutFile "LogParser_Setup.exe"
InstallDir "$PROGRAMFILES64\LogParser"
InstallDirRegKey HKLM "Software\LogParser" "Install_Dir"
RequestExecutionLevel admin

; UI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Install Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstall Pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"

; ========================================
; Install Section
; ========================================
Section "LogParser (Required)" SecMain
    SectionIn RO
    
    SetOutPath $INSTDIR
    
    ; Copy all files from deploy directory
    File /r "build-release\deploy\*.*"
    
    ; Write registry
    WriteRegStr HKLM "Software\LogParser" "Install_Dir" "$INSTDIR"
    
    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    ; Add to program list
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LogParser" \
                     "DisplayName" "LogParser - CSV Data Visualization Tool"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LogParser" \
                     "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LogParser" \
                     "DisplayIcon" "$INSTDIR\LogParser.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LogParser" \
                     "Publisher" "LogParser Team"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LogParser" \
                     "DisplayVersion" "1.0.0"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LogParser" \
                      "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LogParser" \
                      "NoRepair" 1
SectionEnd

Section "Desktop Shortcut" SecDesktop
    CreateShortcut "$DESKTOP\LogParser.lnk" "$INSTDIR\LogParser.exe"
SectionEnd

Section "Start Menu Shortcut" SecStartMenu
    CreateDirectory "$SMPROGRAMS\LogParser"
    CreateShortcut "$SMPROGRAMS\LogParser\LogParser.lnk" "$INSTDIR\LogParser.exe"
    CreateShortcut "$SMPROGRAMS\LogParser\Uninstall LogParser.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

; ========================================
; Uninstall Section
; ========================================
Section "Uninstall"
    ; Delete registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LogParser"
    DeleteRegKey HKLM "Software\LogParser"
    
    ; Delete shortcuts
    Delete "$DESKTOP\LogParser.lnk"
    RMDir /r "$SMPROGRAMS\LogParser"
    
    ; Delete install directory
    RMDir /r "$INSTDIR"
SectionEnd

; Component Descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "LogParser main program files (required)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Create desktop shortcut"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "Create Start Menu shortcuts"
!insertmacro MUI_FUNCTION_DESCRIPTION_END
