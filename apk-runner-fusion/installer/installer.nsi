Unicode true
!include "MUI2.nsh"

!define PRODUCT_NAME "APKRunner Fusion"
!define PRODUCT_VERSION "2.1.0"
!define PRODUCT_KEY "Software\Classes\APKRunnerFusion"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "..\\..\\build-fusion\\APKRunner-Universal-Setup.exe"
InstallDir "$LOCALAPPDATA\Programs\APKRunnerFusion"
RequestExecutionLevel user
SetCompressor /SOLID lzma
ShowInstDetails show
ShowUnInstDetails show

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Section "Install APKRunner Fusion" SEC_MAIN
    SetOutPath "$INSTDIR"
    File "..\\..\\build-fusion\\APKRunner-Fusion.exe"
    File "..\\..\\build-fusion\\adb.exe"
    File "..\\..\\build-fusion\\AdbWinApi.dll"
    File "..\\..\\build-fusion\\AdbWinUsbApi.dll"
        File "..\\..\\build-fusion\\NOTICE.txt"
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    CreateDirectory "$SMPROGRAMS\APKRunner Fusion"
    CreateShortcut "$SMPROGRAMS\APKRunner Fusion\APKRunner Fusion.lnk" \
        "$INSTDIR\APKRunner-Fusion.exe" "--help"
    CreateShortcut "$SMPROGRAMS\APKRunner Fusion\Uninstall.lnk" \
        "$INSTDIR\Uninstall.exe"
    CreateShortcut "$DESKTOP\APKRunner Fusion.lnk" \
        "$INSTDIR\APKRunner-Fusion.exe" "--help"

    WriteRegStr HKCU "${PRODUCT_KEY}" "" "APKRunner Fusion APK Installer"
    WriteRegStr HKCU "${PRODUCT_KEY}\DefaultIcon" "" "$INSTDIR\APKRunner-Fusion.exe,0"
    WriteRegStr HKCU "${PRODUCT_KEY}\shell\open\command" "" \
        '"$INSTDIR\APKRunner-Fusion.exe" open "%1"'
    WriteRegStr HKCU "Software\Classes\.apk\OpenWithProgids" \
        "APKRunnerFusion" ""
    WriteRegStr HKCU "Software\Classes\Applications\APKRunner-Fusion.exe\SupportedTypes" \
        ".apk" ""
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\APKRunnerFusion" \
        "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\APKRunnerFusion" \
        "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Uninstall"
    Delete "$DESKTOP\APKRunner Fusion.lnk"
    Delete "$SMPROGRAMS\APKRunner Fusion\APKRunner Fusion.lnk"
    Delete "$SMPROGRAMS\APKRunner Fusion\Uninstall.lnk"
    RMDir "$SMPROGRAMS\APKRunner Fusion"

    DeleteRegValue HKCU "Software\Classes\.apk\OpenWithProgids" "APKRunnerFusion"
    DeleteRegKey HKCU "${PRODUCT_KEY}"
    DeleteRegKey HKCU "Software\Classes\Applications\APKRunner-Fusion.exe"
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\APKRunnerFusion"

    Delete "$INSTDIR\APKRunner-Fusion.exe"
    Delete "$INSTDIR\adb.exe"
    Delete "$INSTDIR\AdbWinApi.dll"
    Delete "$INSTDIR\AdbWinUsbApi.dll"
        Delete "$INSTDIR\NOTICE.txt"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir "$INSTDIR"
SectionEnd