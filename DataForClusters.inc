; ==================================================================================================
; Title:      DataForClusters.inc
; Author:     Héctor S. Enrique
; Version:    1.0.0
; Purpose:    Clusters Application Data.
; Notes:      Version 1.0.0, January 2024
;               - First release.
; ==================================================================================================

Object DataPoint,, Primer
    DefineVariable X, real8, 0.0
    DefineVariable Y, real8, 0.0
    DefineVariable hashx, XWORD, 0
    DefineVariable hashy, XWORD, 0

    VirtualMethod  ComputeDistance, $ObjPtr(DataPoint)
    VirtualMethod  Equals, $ObjPtr(DataPoint)
    VirtualMethod  GetHashCode
ObjectEnd

    Object DataSet,, Collection

        VirtualMethod  LoadDS, PSTRING
    ObjectEnd

    Object DataCluster,, Collection
        DefineVariable  Cluster, XWORD, 0
        DefineVariable  avgx, real8, 0.0
        DefineVariable  avgy, real8, 0.0
        RedefineMethod Init, POINTER, XWORD
        VirtualMethod  CalculateCentroid
        VirtualMethod  GetCentroid
        VirtualMethod  Equals, $ObjPtr(DataCluster)
        VirtualMethod  GetHashCode
        Embed centroid, DataPoint
    ObjectEnd    

if IMPLEMENT

    Method DataPoint.ComputeDistance, uses xsi, other:$ObjPtr(DataPoint)
        local X0 : real8, Y0 : real8
        local temp1 : REAL8

        SetObject xsi
        
        mov xax, other
        fSlv8 X0 = [xax].$Obj(DataPoint).X
        fSlv8 Y0 = [xax].$Obj(DataPoint).Y
        
        fSlv8 = (([xsi].X-X0)^2 + ([xsi].Y-Y0)^2)^0.5
    MethodEnd

    Method DataPoint.Equals, uses xsi, other:$ObjPtr(DataPoint)
        local X0 : real8, Y0 : real8
        local fSlvTLS()

        SetObject xsi

        mov xcx, other
        fSlv8 X0 = [xcx].$Obj(DataPoint).X 
        fSlv8 Y0 = [xcx].$Obj(DataPoint).Y
        mov xax, 0
        .if other != NULL
            .if fEQ([xsi].X,[xcx].$Obj(DataPoint).X)
                .if fEQ([xsi].Y,[xcx].$Obj(DataPoint).Y)
                    mov xax, 1
                .endif
            .endif 
        .endif
             
    MethodEnd

    Method DataPoint.GetHashCode, uses xsi
        SetObject xsi
        fSlv xax = [xsi].hashx ^ [xsi].hashy
    MethodEnd
    
    Method DataSet.LoadDS, uses xsi, filename : PSTRING

        local DskStream : $ObjPtr(DiskStream)
        local pMemBlock : POINTER
        local inicio : XWORD, final:XWORD
        local line[248]:byte, number1[32]:byte, number2[32]:byte
        local stored : XWORD, pos : XWORD, pos2 : XWORD 
        local x:real8, y:real8
        local fregTLS()
        
        SetObject xsi
        
        New DiskStream
        mov DskStream, xax

        OCall DskStream::DiskStream.Init, xsi, filename , GENERIC_READ or GENERIC_WRITE, \
                                    0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
                       
        mov xbx, $OCall(DskStream::Stream.GetSize)
        ;DbgDec xbx
        .if eax != -1
	        MemAlloc ebx
	        .if xax != NULL
	            mov pMemBlock, xax
	            OCall DskStream::Stream.BinRead, pMemBlock, ebx
	            .if eax != 0
	                mov inicio, 0
	                mov final, 0  
	                ; lee------------------------
	                mov xdx, pMemBlock
	                mov xdi, -1
	                .repeat
	                    mov al, [xdx]
	                    .if inicio == 0     ;   omite la primer línea
	                        .if al == 0Dh
	                            mov inicio, 1
	                            mov xcx, 0
	                        .endif    
	                    .else
	                        .if al == 0Dh
                              @linea:	                            
	                            mov line[xcx], 0
	                            mov pos, 0
	                            mov stored, xdx
	                            lea xdx, line
	                            @loop1:                    
	                                mov al, [xdx]
	                                .if al == ";"
	                                    freg_push xcx
	                                    mov xcx, pos
	                                    mov number1[xcx], 0
	                                    freg_pop xcx    
	                                    inc xdx
	                                    inc pos
	                                    mov pos2, 0
	                                    jmp @loop2
	                                .else
	                                    freg_push xcx
	                                    mov xcx, pos
	                                    mov number1[xcx], al
	                                    freg_pop xcx    
	                                .endif
	                                inc xdx
	                                inc pos
	                                jmp @loop1
	                            @loop2:                    
	                                mov al, [xdx]
	                                .if al == ";"
	                                .else
	                                    freg_push xcx
	                                    mov xcx, pos2
	                                    mov number2[xcx], al
	                                    freg_pop xcx    
	                                .endif
	                                inc xdx
	                                inc pos
	                                inc pos2
	                                cmp xcx, pos
	                                jne @loop2
	                                
	                                freg_push xcx
	                                mov xcx, pos2
	                                mov number2[xcx], 0
	                                freg_pop xcx    
	                                
	                                invoke StrToSt0A, addr number1
	                                fstp x 
	                                invoke StrToSt0A, addr number2
	                                fstp y 
	                                New DataPoint
	                                fSlv8 [xax].$Obj(DataPoint).X =  x
	                                fSlv8 [xax].$Obj(DataPoint).Y = y
	                                OCall xsi.Insert, xax
	                                .if final == 1
	                                    jmp termina
	                                .endif
	                            mov xdx, stored
	                            mov xcx, 0
	                        .elseif al == 0Ah
	                        .else
	                            mov line[xcx], al
	                            inc xcx    
	                        .endif
	                    .endif
	                    inc xdx
	                    inc xdi
	                .until xdi == xbx
	                mov final, 1
	                jmp @linea 
	                ;-----------------------
	    termina:    
	                MemFree pMemBlock
	            .endif
	        .endif    
        .endif
        Destroy DskStream
    MethodEnd

    Method DataCluster.Init, uses xsi, pOwner:POINTER, nc : XWORD
        SetObject xsi
        m2m [xsi].Cluster, nc, @if64bits(xax)
        ACall xsi.Init, xsi, 100, 50, COL_MAX_CAPACITY
    MethodEnd

    Method DataCluster.CalculateCentroid, uses xsi
        local count: XWORD
        
        SetObject xsi

        fSlv8 [xsi].avgx = 0
        fSlv8 [xsi].avgy = 0
        mov ecx, [xsi].dCount
        .while xcx > 0
            lea xax, [sizeof(POINTER)*xcx - sizeof(POINTER)]
            add xax, [xsi].pItems
            mov xax, POINTER ptr [xax]
        
            fSlv8 [xsi].avgx = [xsi].avgx + [xax].$Obj(DataPoint).X
            fSlv8 [xsi].avgy = [xsi].avgy + [xax].$Obj(DataPoint).Y
            dec xcx
        .endw
        fSlv8 [xsi].avgx = [xsi].avgx / [xsi].dCount
        fSlv8 [xsi].avgy = [xsi].avgy / [xsi].dCount
        fSlv8 [xsi].centroid.X = [xsi].avgx
        fSlv8 [xsi].centroid.Y = [xsi].avgy
    MethodEnd

    Method DataCluster.GetCentroid, uses xsi
        SetObject xsi
        lea xax, [xsi].centroid
    MethodEnd

    Method DataCluster.Equals, uses xsi, other : $ObjPtr(DataCluster)
        SetObject xsi
        mov xax, 0
        .if other != NULL
            mov xcx, other
            mov xcx, [xcx].$Obj(DataCluster).Cluster
            .if [xsi].Cluster == xcx
                mov xax, 1
            .endif
        .endif
    MethodEnd

    Method DataCluster.GetHashCode, uses xsi
        SetObject xsi
        mov xax, [xsi].Cluster
    MethodEnd
endif