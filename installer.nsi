!define APPNAME "JSON解析器"
!define APPNAME_EN "JsonParser"
!define VERSION "1.0.0"
!define PUBLISHER "JsonParser"
!define INSTDIR "$PROGRAMFILES64\${APPNAME_EN}"

Name "${APPNAME} ${VERSION}"
OutFile "JsonParser-Setup-${VERSION}.exe"
InstallDir "${INSTDIR}"
RequestExecutionLevel admin

!include "MUI2.nsh"

!define MUI_ICON "app.ico"
!define MUI_UNICON "app.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "SimpChinese"

Section "Install"
    SetOutPath $INSTDIR

    ; Main executable and icon
    File "build\JsonParser.exe"
    File "app.ico"

    ; Qt DLLs
    File "build\Qt6Core.dll"
    File "build\Qt6Gui.dll"
    File "build\Qt6Widgets.dll"

    ; ANGLE/D3D compiler
    File "build\d3dcompiler_47.dll"

    ; Qt plugins - platforms
    SetOutPath "$INSTDIR\platforms"
    File "build\platforms\qwindows.dll"

    ; Qt plugins - styles
    SetOutPath "$INSTDIR\styles"
    File "build\styles\qwindowsvistastyle.dll"

    ; Qt plugins - imageformats (needed for app.ico loading)
    SetOutPath "$INSTDIR\imageformats"
    File "build\imageformats\qico.dll"
    File "build\imageformats\qsvg.dll"

    ; Qt plugins - iconengines
    SetOutPath "$INSTDIR\iconengines"
    File "build\iconengines\qsvgicon.dll"

    ; Qt plugins - generic
    SetOutPath "$INSTDIR\generic"
    File "build\generic\qtuiotouchplugin.dll"

    ; Qt plugins - tls
    SetOutPath "$INSTDIR\tls"
    File "build\tls\qcertonlybackend.dll"
    File "build\tls\qschannelbackend.dll"

    ; Install VC++ runtime
    SetOutPath $INSTDIR
    File "build\vc_redist.x64.exe"
    ExecWait '"$INSTDIR\vc_redist.x64.exe" /install /quiet /norestart' $0
    Delete "$INSTDIR\vc_redist.x64.exe"

    ; Uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; Shortcuts
    CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\JsonParser.exe" "" "$INSTDIR\app.ico"
    CreateDirectory "$SMPROGRAMS\${APPNAME}"
    CreateShortCut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\JsonParser.exe" "" "$INSTDIR\app.ico"
    CreateShortCut "$SMPROGRAMS\${APPNAME}\卸载.lnk" "$INSTDIR\uninstall.exe"

    ; Registry for Add/Remove Programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_EN}" "DisplayName" "${APPNAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_EN}" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_EN}" "DisplayIcon" '"$INSTDIR\app.ico"'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_EN}" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_EN}" "Publisher" "${PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_EN}" "EstimatedSize" "15000"
SectionEnd

Section "Uninstall"
    ; Main files
    Delete "$INSTDIR\JsonParser.exe"
    Delete "$INSTDIR\app.ico"
    Delete "$INSTDIR\Qt6Core.dll"
    Delete "$INSTDIR\Qt6Gui.dll"
    Delete "$INSTDIR\Qt6Widgets.dll"
    Delete "$INSTDIR\d3dcompiler_47.dll"
    Delete "$INSTDIR\uninstall.exe"

    ; Plugin subdirectories
    Delete "$INSTDIR\platforms\qwindows.dll"
    RMDir "$INSTDIR\platforms"

    Delete "$INSTDIR\styles\qwindowsvistastyle.dll"
    RMDir "$INSTDIR\styles"

    Delete "$INSTDIR\imageformats\qico.dll"
    Delete "$INSTDIR\imageformats\qsvg.dll"
    RMDir "$INSTDIR\imageformats"

    Delete "$INSTDIR\iconengines\qsvgicon.dll"
    RMDir "$INSTDIR\iconengines"

    Delete "$INSTDIR\generic\qtuiotouchplugin.dll"
    RMDir "$INSTDIR\generic"

    Delete "$INSTDIR\tls\qcertonlybackend.dll"
    Delete "$INSTDIR\tls\qschannelbackend.dll"
    RMDir "$INSTDIR\tls"

    RMDir "$INSTDIR"

    ; Shortcuts
    Delete "$DESKTOP\${APPNAME}.lnk"
    Delete "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk"
    Delete "$SMPROGRAMS\${APPNAME}\卸载.lnk"
    RMDir "$SMPROGRAMS\${APPNAME}"

    ; Registry
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_EN}"
SectionEnd