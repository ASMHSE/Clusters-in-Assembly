; ==================================================================================================
; Title:      Clusters_Main.inc
; Author:     Héctor S. Enrique
; Version:    1.0.0
; Purpose:    Clusters Application definition.
; Notes:      Version 1.0.0, January 2024
;               - First release.
; ==================================================================================================


Object Application, MySdiAppID, SdiApp                  ;Single Document Interface App.
    RedefineMethod    Init                                ;Init method redefinition
    StaticMethod      Startup
    RedefineMethod    OnSize, WPARAM, LPARAM
    VirtualEvent      OnCommand,  WM_COMMAND
    VirtualEvent      OnClose,    WM_CLOSE, WM_QUERYENDSESSION
    VirtualEvent      OnPaint,    WM_PAINT
    VirtualEvent      OnCreate,   WM_CREATE
    
    VirtualMethod ShowData  
    VirtualMethod ShowResults, $ObjPtr(Collection)
    VirtualMethod Clustering
    VirtualMethod MakeTitle
    DefineVariable Strategy, XWORD, 0
    DefineVariable Data, XWORD, 0
    DefineVariable nClusters, XWORD, 3
    DefineVariable Strategysel, DWORD, IDM_STRAT1
    DefineVariable Datasel, DWORD, IDM_DATA1
    DefineVariable nClusterssel, DWORD, IDM_3CLUSTERS
    Embed graf1, Graphic
    Embed dataset, DataSet
    DefineVariable dataisloaded, XWORD, 0
ObjectEnd                                               ;Ends object definition

.data
        archivo1 db "dataset1.txt",0
        archivo2 db "dataset2.txt",0
        archivo3 db "dataset3.txt",0
        
        archivos    xword offset archivo1
                    xword offset archivo2
                    xword offset archivo3
                 
        estrategia1 db "K-Means     ",0
        estrategia2 db "Hierarquical",0
        estrategia3 db "D.B.S.C.A.N.",0
                 
        estrategias xword offset estrategia1         
                    xword offset estrategia2
                    xword offset estrategia3
                    
        clusters2 db "2-clusters",0            
        clusters3 db "3-clusters",0
                    
        nclusters   xword offset clusters2
                    xword offset clusters3
                
        sep$ db " , ",0                       
.code

; ==================================================================================================
;    Application implementation
; ==================================================================================================

CStr szAppClassName, "OA_Clusters"                  ;Creates szAppClassName string in .const
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:     Application.Init
; Purpose:    Initialize the SDI application object.
; Arguments:  None.
; Return:     Nothing.

Method Application.Init, uses xsi
  local dWndPosX:DWORD, dWndPosY:DWORD, dWndWidth:DWORD, dWndHeight:DWORD

  SetObject xsi
  ACall xsi.Init
  mov dWndWidth, 500                                    ;Default creation width
  mov dWndHeight, 350                                   ;Default creation height

  mov dWndPosX, $32($invoke(CenterForm, dWndWidth,  $32($invoke(GetSystemMetrics, SM_CXSCREEN))))
  mov dWndPosY, $32($invoke(CenterForm, dWndHeight, $32($invoke(GetSystemMetrics, SM_CYSCREEN))))

  invoke CreateWindowEx, WS_EX_LEFT or WS_EX_APPWINDOW, \
                         offset szAppClassName, offset szAppTitle, WS_OVERLAPPEDWINDOW, \
                         dWndPosX, dWndPosY, dWndWidth, dWndHeight, NULL, NULL, hInstance, pSelf

  invoke ShowWindow, [xsi].hWnd, SW_SHOWNORMAL          ;Show the window
  invoke UpdateWindow, [xsi].hWnd                       ;Update if necessary
  
  OCall [xsi].dataset::DataSet.Init, xsi, 100, 50, COL_MAX_CAPACITY
  
  invoke LoadAcceleratorsA, hInstance, $OfsCStrA("APPACCEL")
  mov [xsi].hAccelerators, xax

  OCall xsi.MakeTitle
  
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:     Application.OnCommand
; Purpose:    Event procedure for WM_COMMAND message.
; Arguments:  Arg1: First message parameter.
;             Arg2: Second message parameter.
; Return:     eax = Zero if handled.

	OptionsWndProc macro oID, OptionName, OptionValue
		.elseif ax == oID
			invoke GetMenu, [xsi].hWnd
			mov xbx, xax		
			invoke CheckMenuItem, xax, [xsi].&OptionName&sel, MF_UNCHECKED
			invoke CheckMenuItem, xbx, oID, MF_CHECKED
			mov [xsi].&OptionName, &OptionValue
			mov [xsi].&OptionName&sel, &oID
			OCall xsi.MakeTitle
		 	xor eax, eax
	endm

Method Application.OnCommand, uses xsi xbx, wParam:WPARAM, lParam:LPARAM
  local cBuffer[MAX_PATH]:CHR, Dlg:$Obj(DialogAbout), hIcon:HICON
 
  SetObject xsi
  mov xax, wParam
    .if ax == IDM_EXIT
	    invoke SendMessage, [xsi].hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL
	    xor eax, eax

    .elseif ax == IDM_ABOUT
	    New Dlg::DialogAbout
	    mov hIcon, $invoke(LoadIcon, hInstance, offset szIcoName)
	    OCall Dlg::DialogAbout.Init, xsi, [xsi].hWnd, hIcon, offset szAboutText
	    OCall Dlg::DialogAbout.Show
	    OCall Dlg::DialogAbout.Done
	    invoke DestroyIcon, hIcon
	    xor eax, eax

    .elseif ax == IDM_HELPA
        invoke ExpandEnvironmentStrings, $OfsCStr("Clusters.pdf"), \  
                                         addr cBuffer, lengthof(cBuffer)
        invoke PdfView, [xsi].hWnd, addr cBuffer, $OfsCStr("Clusters Help")
        xor eax, eax
        
    .elseif ax == IDM_HELPD
        invoke ExpandEnvironmentStrings, $OfsCStr("%OBJASM_PATH%\Help\ObjAsm_Reference_Volume-I.pdf"), \  
                                         addr cBuffer, lengthof(cBuffer)
        invoke PdfView, [xsi].hWnd, addr cBuffer, $OfsCStr("Introduction")
        xor eax, eax
    
    OptionsWndProc IDM_STRAT1, Strategy, 0  
    OptionsWndProc IDM_STRAT2, Strategy, 1  
    OptionsWndProc IDM_STRAT3, Strategy, 2  
    OptionsWndProc IDM_DATA1, Data, 0  
    OptionsWndProc IDM_DATA2, Data, 1  
    OptionsWndProc IDM_DATA3, Data, 2  
    OptionsWndProc IDM_2CLUSTERS, nClusters, 2  
    OptionsWndProc IDM_3CLUSTERS, nClusters, 3  
    .elseif ax == IDM_RUN
        .if [xsi].dataisloaded
            OCall xsi.Clustering
        	invoke RedrawWindow, [xsi].graf1.hWnd, NULL, NULL, \
                       RDW_ERASE or RDW_INVALIDATE or RDW_UPDATENOW or RDW_ALLCHILDREN
        .else
            invoke MessageBox,[xsi].hWnd, $OfsCStr("Data must be loaded first"), $OfsCStr("No data"),MB_ICONINFORMATION
        .endif
    .elseif ax == IDM_SHOW
        OCall xsi.ShowData
	    invoke RedrawWindow, [xsi].hWnd, NULL, NULL, \
                       RDW_ERASE or RDW_INVALIDATE or RDW_UPDATENOW or RDW_ALLCHILDREN
    .else
        invoke DefWindowProc, [xsi].hWnd, WM_COMMAND, wParam, lParam
    .endif
MethodEnd

Method Application.OnCreate, uses xsi, wParam:WPARAM, lParam:LPARAM
    ;DbgText "Application.OnCreate"
    SetObject xsi
	OCall [xsi].graf1::Graphic.Init, xsi, [xsi].hWnd
MethodEnd
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:     Application.OnClose
; Purpose:    Event procedure for WM_CLOSE message.
; Arguments:  Arg1: First message parameter.
;             Arg2: Second message parameter.
; Return:     eax = Zero if handled.

Method Application.OnClose, uses xsi, wParam:WPARAM, lParam:LPARAM
  SetObject xsi
  invoke MessageBox, [xsi].hWnd, $OfsCStr("Are you sure ?"), $OfsCStr("Application exit"), \
                       MB_YESNO or MB_ICONQUESTION
  .if eax == IDNO
    xor eax, eax
  .else
    invoke DefWindowProc, [xsi].hWnd, WM_CLOSE, wParam, lParam
  .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:     Application.OnPaint
; Purpose:    Event procedure for WM_PAINT message.
; Arguments:  Arg1: First message parameter.
;             Arg2: Second message parameter.
; Return:     eax = Zero if handled.

Method Application.OnPaint, uses xsi, wParam:WPARAM, lParam:LPARAM
  local PS:PAINTSTRUCT, hDC:HDC, Rct:RECT

  SetObject xsi
  mov hDC, $invoke(BeginPaint, [xsi].hWnd, addr PS)

  invoke GetClientRect, [xsi].hWnd, addr Rct
  invoke DrawEdge, hDC, addr Rct, EDGE_SUNKEN, BF_RECT

  invoke EndPaint, [xsi].hWnd, addr PS
  xor eax, eax
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:     Application.Startup
; Purpose:    Register the Application window class with the operating system.
; Arguments:  None.
; Return:     Nothing.

Method Application.Startup
  local WC:WNDCLASSEX

  mov WC.cbSize, sizeof(WNDCLASSEX)
  m2z WC.style
  m2m WC.lpfnWndProc, $MethodAddr(SdiApp.WndProc), xax
  m2z WC.cbClsExtra
  m2z WC.cbWndExtra
  m2m WC.hInstance, hInstance, xax
  invoke LoadBitmap, hInstance, $OfsCStr("BMP_BACKGROUND")
  mov WC.hbrBackground, $invoke(CreatePatternBrush, xax)
  c2m WC.lpszMenuName, $OfsCStr("MENU_APP"), xax  
  c2m WC.lpszClassName, offset szAppClassName, xax
  mov WC.hIcon, $invoke(LoadIcon, hInstance, offset szIcoName)
  mov WC.hCursor, $invoke(LoadCursor, 0, IDC_ARROW)
  m2z WC.hIconSm

  invoke RegisterClassEx, addr WC
MethodEnd
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:     Application.Clustering
; Purpose:    Principal function
; Arguments:  None.
; Return:     Nothing.

Method Application.Clustering, uses xsi
    local dataset : $ObjPtr(DataSet)
    local strategy : $ObjPtr(ClusteringStrategy)
    local pointsCategorized : POINTER
    
    SetObject xsi
if 0
    New DataSet
    mov dataset, xax
    mov xax, [xsi].Data
    mov xax, archivos[xax*XWORD]
    OCall dataset::DataSet.Load, xax
endif    

    .if [xsi].Strategy == 0
        New KMeansClusteringStrategy
        mov strategy, xax
        OCall strategy::KMeansClusteringStrategy.Init
    
    .elseif [xsi].Strategy == 1
        New HierarchicalClusteringStrategy
        mov strategy, xax
        OCall strategy::HierarchicalClusteringStrategy.Init
    .elseif [xsi].Strategy == 2
        New DBSCANClusteringStrategy
        mov strategy, xax
        OCall strategy::DBSCANClusteringStrategy.Init
    .endif 
    
    OCall strategy::ClusteringStrategy.Cluster, addr [xsi].dataset, [xsi].nClusters
    
    OCall xsi.ShowResults, xax
    
    mov pointsCategorized, xax

MethodEnd


Method Application.OnSize, uses xsi, wParam:WPARAM, lParam:LPARAM
    local rect : RECT
	 
    .if wParam != SIZE_MINIMIZED
	   SetObject xsi
	   	invoke GetClientRect, [xsi].hWnd, addr rect
      	mov eax , rect.right
      	mov __ecx , rect.bottom
      	invoke MoveWindow, [xsi].graf1.hWnd, 0, 0, eax, __ecx , TRUE
		invoke DefWindowProc, [xsi].hWnd, WM_SIZE, wParam, lParam
    .endif
MethodEnd

Method Application.ShowData, uses xsi
    SetObject xsi
    OCall [xsi].dataset::Collection.DisposeAll
    mov xax, [xsi].Data
    mov xcx, offset archivos
    mov xax, [xcx+xax*sizeof(POINTER)]
    OCall [xsi].dataset::DataSet.LoadDS, xax
    OCall [xsi].graf1::Graphic.BuscaExtremosDS, addr [xsi].dataset
    OCall [xsi].graf1::Graphic.DrawDataSet, addr [xsi].dataset
    mov [xsi].dataisloaded, 1
MethodEnd

Method Application.ShowResults, uses xsi, dataset : $ObjPtr(Collection)
    SetObject xsi
    ;DbgText "+ Application.ShowResults"
    OCall [xsi].graf1::Graphic.BuscaExtremosColDC, dataset
    OCall [xsi].graf1::Graphic.DrawColDC, dataset
    ;DbgText "- Application.ShowResults"
MethodEnd

Method Application.MakeTitle, uses xsi
    local buffer[248]:byte
    
    SetObject xsi

    invoke StrCopy, addr buffer, $OfsCStr("Clusters   [ ")

    mov xax, [xsi].Strategy
    mov xcx, offset estrategias
    mov xax, [xcx+xax*sizeof(POINTER)]
    invoke StrCat, addr buffer, xax
    invoke StrCat, addr buffer, addr sep$
    
    mov xax, [xsi].Data
    mov xcx, offset archivos
    mov xax, [xcx+xax*sizeof(POINTER)]
    invoke StrCat, addr buffer, xax

    .if [xsi].Strategy < 2
        invoke StrCat, addr buffer, addr sep$
        
	    mov xax, [xsi].nClusters
	    sub xax, 2
	    mov xcx, offset nclusters
	    mov xax, [xcx+xax*sizeof(POINTER)]
	    invoke StrCat, addr buffer, xax
    .endif
    invoke StrCat, addr buffer, $OfsCStr(" ]")
    
    invoke SetWindowText, [xsi].hWnd, addr buffer
    
MethodEnd