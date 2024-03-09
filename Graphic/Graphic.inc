    GRAPHIC1_DRAW equ WM_USER

	xconf equ 484
	yconf equ 291
	mconf equ 20


    DefineDD macro a1,a2
		DefineVariable &a1, dword, &a2
	endm

	DefineRect macro a1
		DefineVariable &a1, RECT, {}
	endm

	compvalR macro a1,a2
		Fld	a1
		fcomp a2
		fstsw ax
	endm

	FP_LESSTHAN equ	01h

; ==================================================================================================
; Title:   Graphic.asm
; Author:  HSE @ January 2024
; Version: 1.0.0
; Purpose:  Graphic
; ==================================================================================================

Object Graphic,, Window
	
    RedefineMethod Init, POINTER, HWND       ;-> Owner, Parent
    RedefineMethod Done

    VirtualEvent	OnCommand,  WM_COMMAND
    VirtualEvent    OnPaint,    WM_PAINT
    StaticMethod	Startup
    ;----------------------------------------------------
    DefineVariable calculado, dword, 0 
    DefineVariable	minmin, REAL8, 10000000000000000000.0
    DefineVariable	maxmax, REAL8, -10000000000000000000.0	
    VirtualMethod 	DrawDataSet, POINTER
    VirtualMethod 	BuscaExtremosDS, POINTER
    VirtualMethod 	DrawColDC, POINTER
    VirtualMethod 	BuscaExtremosColDC, POINTER

    DefineVariable minimox, REAL8, 0
    DefineVariable minimoy, REAL8, 0
    DefineVariable maximox, REAL8, 0
    DefineVariable maximoy, REAL8, 0
    DefineVariable escalax, REAL8, 0
    DefineVariable escalay, REAL8, 0
    DefineVariable margin, DWORD, mconf
    Embed Pxm1, Lensmap
ObjectEnd

if IMPLEMENT

.data 

	CStr szGraphic1, "graphic"                           ;Creates string in .const
	nd254	dd 255
	nd128	dd 128

     colors dd 00FFFF00h
            dd 008000FFh
            dd 0000FFFFh
            dd 0000FF00h
            dd 004080FFh
            dd 00FF80FFh
            dd 00FF8080h
            dd 00008080h
            dd 00FFFF00h
            dd 00FFFF00h
            dd 00FFFF00h
            dd 00FFFF00h
            dd 00FFFF00h
            dd 00FFFF00h
            dd 00FFFF00h
            dd 00FFFF00h
            dd 00FFFF00h
            
            
             
	
.code

	CalculaPuntito macro eje1
			
		SetObject xbx, Puntito, punto
			
			mov		xax, &eje1
			
			fld		[xsi].minimoy
			fsub	[xbx].y
			fmul	[xsi].escalay
			fiadd	[xsi].bottom
			fistp	a1
			
			fld		[xbx].x
			fsub	[xsi].minimox
			fmul	[xsi].escalax
			fiadd	[xsi].rDibujo.left
			fistp	a3
			
	endm
	
; ==================================================================================================
;    Some demonstration procedures
; ==================================================================================================
; ########################################################################

MakeFont proc hgt:DWORD,wid:DWORD,weight:DWORD,italic:DWORD,lpFontName:XWORD

    invoke CreateFont, hgt, wid,NULL,NULL,weight,italic,NULL,NULL,
                      DEFAULT_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,
                      PROOF_QUALITY,DEFAULT_PITCH or FF_DONTCARE,
                      lpFontName
    ret

MakeFont endp

; ########################################################################


WULine proc pPXM:POINTER, r4Xi:REAL4, r4Yi:REAL4, r4Xd:REAL4, r4Yd:REAL4, dPoints:DWORD, dColor:DWORD
    local fregTLS()
    xor ecx, ecx
    .while ecx < dPoints
      freg_push xcx
      OCall pPXM::Pixelmap.BlendVirtualPixel, r4Xi, r4Yi, dColor
      freg_pop xcx
      inc ecx
      fld r4Xi
      fadd r4Xd
      fstp r4Xi
      fld r4Yi
      fadd r4Yd
      fstp r4Yi
    .endw
    ret
WULine endp

WUCircle proc uses xbx pPXM:POINTER, r4Xc:REAL4, r4Yc:REAL4, r4R:REAL4, dColor:DWORD, dPoints:DWORD
    local r4X:REAL4, r4Y:REAL4, r4Inc:REAL4

    fld1
    fld r4R
    fdivp st(1), st(0)
    fstp r4Inc
    fldz
    mov ebx, dPoints
@@:
    fld st(0)
    fsincos
    fmul r4R
    fadd r4Xc
    fstp r4X
    fmul r4R
    fadd r4Yc
    fstp r4Y
    OCall pPXM::Pixelmap.BlendVirtualPixel, r4X, r4Y, dColor
    fadd r4Inc
    dec ebx
    jnz @B

    fUnload
    ret
WUCircle endp


; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    ProjectWnd.Startup
; Purpose:   Register the object with the OS.
; Arguments: None.
; Return:    Nothing.

Method Graphic.Startup
    
	local WC:WNDCLASSEX
	
	mov WC.cbSize, sizeof WNDCLASSEX
	mov WC.style, CS_HREDRAW or CS_VREDRAW or CS_BYTEALIGNWINDOW or CS_BYTEALIGNCLIENT
	m2m WC.lpfnWndProc, $MethodAddr(Graphic.WndProc), edx
   
   	m2z WC.cbClsExtra
 	m2z WC.cbWndExtra
	m2m WC.hInstance, hInstance, edx
    mov WC.hbrBackground, NULL;$invoke(CreatePatternBrush, eax)

	m2z WC.lpszMenuName
	mov xax, offset szGraphic1
	mov WC.lpszClassName, xax
	m2z WC.hIcon
	m2z WC.hIconSm
	mov WC.hCursor, $invoke(LoadCursor, NULL, IDC_ARROW)

	invoke RegisterClassEx, addr WC
    
 MethodEnd


; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    ProjectWnd.Init
; Purpose:   Initialize the ProjectWnd object.
; Arguments: Arg1: -> Owner object.
;            Arg2: Parent window HANDLE.
; Return:    Nothing.

Method Graphic.Init, uses xsi, pOwner:POINTER, hParent:HWND
    local dim: dword
    local rect : RECT

	SetObject xsi
   
	ACall xsi.Init, pOwner, NULL
   
	invoke CreateWindowEx, WS_EX_TRANSPARENT , offset szGraphic1, $OfsCStr("Graphic1"), \
                           WS_CHILD  or WS_CLIPSIBLINGS	or WS_CLIPCHILDREN, 0, 0, xconf, yconf, \
                           hParent, NULL, hInstance, pSelf
   
	invoke ShowWindow, [xsi].hWnd, SW_SHOWNORMAL

	OCall [xsi].Pxm1::Pixelmap.Init, xsi, xconf, yconf
	OCall [xsi].Pxm1::Pixelmap.FillBkGnd, $RGB(0,0,128,255)
	
MethodEnd

Method Graphic.Done, uses xsi
    local v1 : dword
	
    SetObject xsi

    OCall [xsi].Pxm1::Pixelmap.Done
    
    ACall xsi.Done
	DbgText "Graphic0.Done"
MethodEnd

Method Graphic.OnEraseBKgnd, , wParam:WPARAM, lParam:LPARAM

    mov xax, TRUE

MethodEnd

Method Graphic.OnPaint, uses xsi, wParam:WPARAM, lParam:LPARAM
    local PS:PAINTSTRUCT, hDC:HDC, CRect:RECT

    SetObject xsi
    
    mov hDC, $invoke(BeginPaint, [xsi].hWnd, addr PS)

    invoke GetClientRect, [xsi].hWnd, addr CRect
    invoke StretchBlt, hDC, CRect.left, CRect.top, CRect.right, CRect.bottom, \
                 [xsi].Pxm1.hDC, 0, 0, xconf, yconf, SRCCOPY
    invoke EndPaint, [xsi].hWnd, addr PS
    mov eax, TRUE
MethodEnd

Method Graphic.OnCommand, uses xsi, wParam:WPARAM, lParam:LPARAM
    	
	SetObject xsi

	.if wParam == WM_USER
		OCall [xsi].Pxm1::Pixelmap.FillBkGnd, $RGB(0,0,128,255)
		.if [xsi].calculado == TRUE

		.endif		
         xor xax, xax
    .else
    	invoke DefWindowProc,[xsi].hWnd, WM_COMMAND, wParam, lParam
    .endif

MethodEnd

Method Graphic.DrawDataSet, uses xsi, dataset:POINTER
    local a1 : SDWORD, a3 : SDWORD, ancho : sdword, alto : sdword, bottom : sdword
    local fregTLS()
    
	SetObject xsi

	OCall [xsi].Pxm1::Pixelmap.FillBkGnd, $RGB(0,0,108,255)
	
	%mov ancho, (xconf-(mconf*2))
	%mov alto , (yconf-(mconf*2))
	%mov bottom, yconf 
	fSlv8 [xsi].escalax = ancho/([xsi].maximox-[xsi].minimox)
	fSlv8 [xsi].escalay = alto/([xsi].maximoy-[xsi].minimoy)

	mov xbx, dataset 
	.ColForEach [xbx]
	    assume xax: $ObjPtr(DataPoint)
			
			fld		[xsi].minimoy
			fsub	[xax].Y
			fmul	[xsi].escalay
			fisub	[xsi].margin
			fiadd   bottom
			fistp	a1
			
			fld		[xax].X
			fsub	[xsi].minimox
			fmul	[xsi].escalax
			fiadd	[xsi].margin
			fistp	a3
            freg_push xax
        	OCall [xsi].Pxm1::Lensmap.lens1, a3, a1, 0FFFFFFh
            freg_pop xax
	    assume xax:nothing
	.ColNext
	invoke RedrawWindow, [xsi].hWnd, NULL, NULL, \
                       RDW_ERASE or RDW_INVALIDATE or RDW_UPDATENOW or RDW_ALLCHILDREN
MethodEnd

Method Graphic.BuscaExtremosDS, uses xbx xsi, dataset: POINTER
    
	SetObject xsi
	
	fSlv8 [xsi].minimox = [xsi].minmin
	fSlv8 [xsi].minimoy = [xsi].minmin
	fSlv8 [xsi].maximox = [xsi].maxmax
	fSlv8 [xsi].maximoy = [xsi].maxmax
	
	mov xax, dataset 
	mov xbx, dataset 
	.ColForEach  [xbx]
	    mov xcx, xax
	    assume xcx: $ObjPtr(DataPoint)
		compvalR [xcx].X, [xsi].minimox 
		.if ( ah & FP_LESSTHAN)
			fld		[xcx].X
			fstp	[xsi].minimox
		.endif 
		compvalR [xsi].maximox , [xcx].X 
		.if ( ah & FP_LESSTHAN)
			fld		[xcx].X
			fstp	[xsi].maximox
		.endif 
		
		compvalR  [xcx].Y, [xsi].minimoy 
		.if ( ah & FP_LESSTHAN)
			fld		[xcx].Y
			fstp	[xsi].minimoy
		.endif 
		
		compvalR  [xsi].maximoy, [xcx].Y
		.if ( ah & FP_LESSTHAN)
			fld		[xcx].Y
			fstp	[xsi].maximoy
		.endif 
	    assume xcx:nothing
	.ColNext
	
MethodEnd

Method Graphic.BuscaExtremosColDC, uses xbx xsi, dataset: POINTER
    local fregTLS()
        
	SetObject xsi
	
	fSlv8 [xsi].minimox = [xsi].minmin
	fSlv8 [xsi].minimoy = [xsi].minmin
	fSlv8 [xsi].maximox = [xsi].maxmax
	fSlv8 [xsi].maximoy = [xsi].maxmax
	
	mov xax, dataset 
	mov xbx, dataset 
	.ColForEach  [xbx]
	
	    freg_push xbx
	    mov xbx, xax
	    .ColForEach [xbx]
	        .if xax != NULL
		    mov xcx, xax
		    assume xcx: $ObjPtr(DataPoint)
			compvalR [xcx].X, [xsi].minimox 
			.if ( ah & FP_LESSTHAN)
				fld		[xcx].X
				fstp	[xsi].minimox
			.endif 
			compvalR [xsi].maximox , [xcx].X 
			.if ( ah & FP_LESSTHAN)
				fld		[xcx].X
				fstp	[xsi].maximox
			.endif 
			
			compvalR  [xcx].Y, [xsi].minimoy 
			.if ( ah & FP_LESSTHAN)
				fld		[xcx].Y
				fstp	[xsi].minimoy
			.endif 
			
			compvalR  [xsi].maximoy, [xcx].Y
			.if ( ah & FP_LESSTHAN)
				fld		[xcx].Y
				fstp	[xsi].maximoy
			.endif 
		    assume xcx:nothing
		    .endif
		.ColNext
	    freg_pop xbx

	.ColNext    	
	
MethodEnd

Method Graphic.DrawColDC, uses xsi, dataset:POINTER
    local a1 : SDWORD, a3 : SDWORD, ancho : sdword, alto : sdword, bottom : sdword
    local fregTLS()
    local counter : xword
    
	SetObject xsi

	OCall [xsi].Pxm1::Pixelmap.FillBkGnd, $RGB(0,0,108,255)
	
	%mov ancho, (xconf-(mconf*2))
	%mov alto , (yconf-(mconf*2))
	%mov bottom, yconf 
	fSlv8 [xsi].escalax = ancho/([xsi].maximox-[xsi].minimox)
	fSlv8 [xsi].escalay = alto/([xsi].maximoy-[xsi].minimoy)

    mov counter, 0
	mov xbx, dataset 
	.ColForEach [xbx]

	    freg_push xbx
	    mov xbx, xax
	    inc counter
	    .ColForEach [xbx]
	        .if xax != NULL
		    assume xax: $ObjPtr(DataPoint)
				
				fld		[xsi].minimoy
				fsub	[xax].Y
				fmul	[xsi].escalay
				fisub	[xsi].margin
				fiadd   bottom
				fistp	a1
				
				fld		[xax].X
				fsub	[xsi].minimox
				fmul	[xsi].escalax
				fiadd	[xsi].margin
				fistp	a3
	            freg_push xax
	            mov xcx, counter
	            lea xax, colors
	            mov eax, [xax+xcx*4-4]
	        	OCall [xsi].Pxm1::Lensmap.lens1, a3, a1, eax
	            freg_pop xax
		    assume xax:nothing
		    .endif
		.ColNext
		freg_pop xbx
	.ColNext
	invoke RedrawWindow, [xsi].hWnd, NULL, NULL, \
                       RDW_ERASE or RDW_INVALIDATE or RDW_UPDATENOW or RDW_ALLCHILDREN
MethodEnd

endif
