 Object Lensmap,, Pixelmap
     VirtualMethod lens1, dword, dword, dword
 ObjectEnd

    tmg textequ <5>
     
    takecolor macro color, cc1, cc2, cc3
		mov	eax , color
		and eax , 00000FFh
		mov cc1, eax
		mov	eax , color
		and eax , 000FF00h
		shr eax, 8
		mov cc2, eax
		mov	eax , color
		and eax , 0FF0000h
		shr eax, 16
		mov cc3, eax
	endm
   	
   	remakecolor macro dilucion, cc1, cc2, cc3, c1
	    ifidn abb_backend_entity,<FSLV_X86_FPU_ENTITY>
		    fSlv8 = dilucion;0.85
	    else;	FSLV_X86_SSE2_FPU_ENTITY
		    fSlv8 tempR8 = dilucion;0.85
		    fld tempR8
	    endif
		fld   st
		fld   st
		fild  cc1
		fmul
		fistp c1 
		fimul cc2
		fistp c2 
		fimul cc3
		fistp c3
		mov		eax, c1;c3
		shl		eax, 8
		add 	eax, c2
		shl 	eax, 8
		add 	eax, c3;c1
		mov 	c1, eax
	endm

Method Lensmap.lens1, uses xsi xdi, x:dword, y:dword, color:dword
	
	local i:dword, j:dword
	local x1:dword, y1:dword
	local r:real8
	local  c1:dword, c2:dword, c3:dword
	local cc1:dword, cc2:dword,cc3: dword
	local fSlvTLS()
	local fregTLS(,40)

	;// (1-r)^2

	SetObject xsi

		mov eax, color
		bswap eax
		ror eax, 8
		mov color,  eax

	takecolor color, cc1, cc2, cc3

	ForLp i, -tmg-1, tmg+1, eax
		ForLp j, -tmg-1, tmg+1, eax
		    fSlv8 r = (i*i+j*j)^0.5/(tmg+1)
			.if fLT(r, 1.0)
				remakecolor <(1-r)^0.5>, cc1, cc2, cc3, c1
				mov 	eax , x
				add 	eax, i 
				mov 	x1, eax
				mov 	eax , y
				add 	eax, j 
				mov 	y1, eax
				OCall xsi.SetPixel, x1, y1, c1
			.endif
		Next j
	Next i
MethodEnd
