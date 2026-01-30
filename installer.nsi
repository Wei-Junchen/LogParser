; ========================================
; LogParser Windows 安装程序脚本
; 使用 NSIS (Nullsoft Scriptable Install System)
; 下载 NSIS: https://nsis.sourceforge.io/
; ========================================

!include "MUI2.nsh"

; 基本信息
Name "LogParser"
OutFile "LogParser_Setup.exe"
InstallDir "$PROGRAMFILES64\LogParser"
InstallDirRegKey HKLM "Software\LogParser" "Install_Dir"
RequestExecutionLevel admin

; 界面设置
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; 安装页面
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; 卸载页面
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; 语言
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"

; ========================================
; 安装部分
; ========================================
Section "LogParser (必需)" SecMain
    SectionIn RO
    
    SetOutPath $INSTDIR
    
    ; 复制所有文件（从 deploy 目录）
    File /r "build-release\deploy\*.*"
    
    ; 写入注册表
    WriteRegStr HKLM "Software\LogParser" "Install_Dir" "$INSTDIR"
    
    ; 创建卸载程序
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    ; 添加到程序列表
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LogParser" \
                     "DisplayName" "LogParser - CSV数据可视化工具"
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

Section "桌面快捷方式" SecDesktop
    CreateShortcut "$DESKTOP\LogParser.lnk" "$INSTDIR\LogParser.exe"
SectionEnd

Section "开始菜单快捷方式" SecStartMenu
    CreateDirectory "$SMPROGRAMS\LogParser"
    CreateShortcut "$SMPROGRAMS\LogParser\LogParser.lnk" "$INSTDIR\LogParser.exe"
    CreateShortcut "$SMPROGRAMS\LogParser\卸载 LogParser.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

; ========================================
; 卸载部分
; ========================================
Section "Uninstall"
    ; 删除注册表项
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LogParser"
    DeleteRegKey HKLM "Software\LogParser"
    
    ; 删除快捷方式
    Delete "$DESKTOP\LogParser.lnk"
    RMDir /r "$SMPROGRAMS\LogParser"
    
    ; 删除安装目录
    RMDir /r "$INSTDIR"
SectionEnd

; 组件描述
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "LogParser 主程序文件（必需）"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "在桌面创建快捷方式"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "在开始菜单创建快捷方式"
!insertmacro MUI_FUNCTION_DESCRIPTION_END
