; ==================================================================================================
; Title:      Clusters_Globals.inc
; Author:     Héctor S. Enrique
; Version:    1.0.0
; Purpose:    Clusters global data definition.
; Notes:      Version 1.0.0, October 2023
;               - First release.
; ==================================================================================================


include Clusters_Shared.inc
CStr    szIcoName,  "ICON_APP"
%CStr   szAppTitle, "&APP_TITLE"
%CStr   szAboutText,"&ABOUT_TEXT", 13, 10, \ 
                    "Version &VER_PRODUCTVERSION_STR", 13, 10, \
                    "Designed with ObjAsm", 13, 10, \
                    "© &COPYRIGHT", 13, 10, 13, 10, \
                    @CatStr(<!">, %BUILD_DATE_STR, \
                    <  -  Build: >, %BUILD_NUMBER, <->, %ASSEMBLER, <->, \
                    %TARGET_MODE_STR, <->, %TARGET_BITNESS, <!">)