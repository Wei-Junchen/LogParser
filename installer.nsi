; ========================================
; LogParser Windows Installer Script
; NSIS (Nullsoft Scriptable Install System)
; Download NSIS: https://nsis.sourceforge.io/
; ========================================

!include "MUI2.nsh"

; ========================================
; Application Info
; ========================================
!define APP_NAME "LogParser"
!define APP_VERSION "1.0.0"
!define APP_PUBLISHER "Wei-Junchen"
!define APP_URL "https://github.com/Wei-Junchen/Logparser"
!define APP_COPYRIGHT "Copyright (C) 2026 WeiJunchen"
!define APP_DESCRIPTION "CSV Log Data Visualization Tool"

; Basic Info
Name "${APP_NAME}"
OutFile "LogParser_Setup.exe"
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKLM "Software\${APP_NAME}" "Install_Dir"
RequestExecutionLevel admin

; ========================================
; Version Info (shown in file properties)
; ========================================
VIProductVersion "${APP_VERSION}.0"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "ProductVersion" "${APP_VERSION}"
VIAddVersionKey "CompanyName" "${APP_PUBLISHER}"
VIAddVersionKey "LegalCopyright" "${APP_COPYRIGHT}"
VIAddVersionKey "FileDescription" "${APP_DESCRIPTION}"
VIAddVersionKey "FileVersion" "${APP_VERSION}"

; ========================================
; UI Settings
; ========================================
!define MUI_ABORTWARNING

; Custom Icon
!define MUI_ICON "resources\app.ico"
!define MUI_UNICON "resources\app.ico"

; Branding text at bottom of installer
BrandingText "${APP_NAME} v${APP_VERSION} - ${APP_PUBLISHER}"

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
    
    ; Main executable
    File "build-release\LogParser.exe"
    
    ; Required Qt DLLs
    File "build-release\Qt6Charts.dll"
    File "build-release\Qt6Core.dll"
    File "build-release\Qt6Gui.dll"
    File "build-release\Qt6Widgets.dll"
    
    ; MinGW runtime libraries
    File "build-release\libgcc_s_seh-1.dll"
    File "build-release\libstdc++-6.dll"
    File "build-release\libwinpthread-1.dll"
    
    ; Required Qt plugins
    SetOutPath $INSTDIR\platforms
    File "build-release\platforms\qwindows.dll"
    
    SetOutPath $INSTDIR\styles
    File "build-release\styles\qmodernwindowsstyle.dll"
    
    SetOutPath $INSTDIR\imageformats
    File "build-release\imageformats\qico.dll"
    File "build-release\imageformats\qjpeg.dll"
    
    SetOutPath $INSTDIR
    
    ; Write registry
    WriteRegStr HKLM "Software\LogParser" "Install_Dir" "$INSTDIR"
    
    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    ; Add to program list
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                     "DisplayName" "${APP_NAME} - ${APP_DESCRIPTION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                     "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                     "DisplayIcon" "$INSTDIR\LogParser.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                     "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                     "URLInfoAbout" "${APP_URL}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                     "DisplayVersion" "${APP_VERSION}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                      "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
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
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
    DeleteRegKey HKLM "Software\${APP_NAME}"
    
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
