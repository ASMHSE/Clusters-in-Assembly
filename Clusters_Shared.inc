; ==================================================================================================
; Title:      Clusters_Shared.inc
; Author:     Héctor S. Enrique
; Version:    1.0.0
; Purpose:    Clusters shared definitions.
; Notes:      Version 1.0.0, October 2023
;               - First release.
; ==================================================================================================


;General information
APP_TITLE                     textequ   <Clusters>
ABOUT_TEXT                    textequ   <Clusters>
COPYRIGHT                     textequ   <Héctor S. Enrique January 2024> 
VER_FILEVERSION               equ       2,0,0,0
VER_FILEVERSION_STR           textequ   <2.0>
VER_PRODUCTVERSION            equ       2,0,0,0
VER_PRODUCTVERSION_STR        equ       <2.0>

;Build information - updated automatically -
BUILD_NUMBER                  equ       901
BUILD_DATE_STR                textequ   <09/03/2024>
BUILD_BITNESS                 equ       64
BUILD_MODE                    equ       RLS

;Menu IDs
IDM_EXIT                      equ       1000
IDM_HELPA                     equ       1001
IDM_HELPD                     equ       1002
IDM_ABOUT                     equ       1999
IDM_RUN                       equ       2000
IDM_STRAT1                    equ       2001  
IDM_STRAT2                    equ       2002  
IDM_STRAT3                    equ       2003  
IDM_DATA1                     equ       2011  
IDM_DATA2                     equ       2012  
IDM_DATA3                     equ       2013  
IDM_2CLUSTERS                 equ       2022
IDM_3CLUSTERS                 equ       2023
IDM_SHOW                      equ       2030

BUILD_BIN_FORMAT              equ       EXE
BUILD_BIN_SUFFIX_STR          textequ   <>
