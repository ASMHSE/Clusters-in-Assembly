; ==================================================================================================
; Title:      Clusters.asm
; Author:     Héctor S. Enrique
; Version:    1.0.0
; Purpose:    Clusters from Win Simple template.
;
;         Adapted fron Nicolás Descartes
;         https://www.elementsofcomputerscience.com/posts/comparing-kmeans-and-others-for-data-clustering-01/
;         https://www.codeproject.com/Articles/5375470/Comparing-K-Means-and-Others-Algorithms-for-Data-C
;
; Notes:      Version 1.0.0, October 2023
;               - First release.
; ==================================================================================================
xword textequ <XWORD>

% include @Environ(OBJASM_PATH)\Code\Macros\Model.inc   ;Include & initialize standard modules
SysSetup OOP, WIN64, ANSI_STRING;, DEBUG(WND,TRACE,INFO)            ;Load OOP files and OS related objects

% include &COMPath&COM.inc
% include &COMPath&COM_Dispatch.inc
% include &MacPath&fMath.inc
% include &IncPath&Windows\sGUID.inc
% include &IncPath&Windows\olectl.inc

% includelib &LibPath&Windows\shell32.lib
% includelib &LibPath&Windows\Comctl32.lib
% includelib &LibPath&Windows\shlwapi.lib
% includelib &LibPath&Windows\OleAut32.lib

;Load or build the following objects
MakeObjects Primer, Stream, WinPrimer
MakeObjects Collection, DiskStream, Pixelmap
MakeObjects Window, Button, Hyperlink, Dialog, DialogModal, DialogAbout
MakeObjects WinApp, SdiApp
MakeObjects Vector, DWordVector, Real8Vector

    if @WordSize eq 4
      .686
      .mmx
      .xmm
    endif  
    include \masm32\macros\SmplMath\Math.inc
    fSlvSelectBackEnd FPU

    @reg32_64 ecx, r10
    @reg32_64 edx, r11

MakeObjects .\SpecialVectors
MakeObjects .\DataForClusters 
MakeObjects .\Graphic\Lensmap
MakeObjects .\Graphic\Graphic

MakeObjects .\StrategyForClusters 

include Clusters_Globals.inc                              ;Application globals
include Clusters_Main.inc                                 ;Application object

.code
start proc                                              ;Program entry point
  SysInit                                               ;Runtime initialization of OOP model

  DbgClearAll
    
  OCall $ObjTmpl(Application)::Application.Init         ;Initialize application
  OCall $ObjTmpl(Application)::Application.Run          ;Execute application
  OCall $ObjTmpl(Application)::Application.Done         ;Finalize application

  SysDone                                               ;Runtime finalization of the OOP model
  invoke ExitProcess, 0                                 ;Exit program returning 0 to the OS
start endp

end
