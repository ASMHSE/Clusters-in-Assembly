; ==================================================================================================
; Title:      StrategyForClusters.inc
; Author:     Héctor S. Enrique
; Version:    1.0.0
; Purpose:    Clusters Application Strategies.
; Notes:      Version 1.0.0, January 2024
;               - First release.
; ==================================================================================================
Object ClusteringStrategy,,Primer
    VirtualAbstract Cluster, $ObjPtr(DataSet), XWORD
    RedefineMethod Init
    RedefineMethod Done
    Embed results, Collection; = new List<DataCluster>();
    Embed selectedPoints, Collection; = new List<DataPoint>();

ObjectEnd

Object KMeansClusteringStrategy,,ClusteringStrategy
    RedefineMethod Init
    RedefineMethod Done
    RedefineMethod Cluster, $ObjPtr(DataSet), XWORD
    VirtualMethod Initialize, $ObjPtr(DataSet), XWORD
    Embed remainingPoints, Collection
ObjectEnd

Object HierarchicalClusteringStrategy,, ClusteringStrategy
    RedefineMethod Init
    RedefineMethod Done
    RedefineMethod Cluster, $ObjPtr(DataSet), XWORD
    VirtualMethod ComputeDistanceBetweenClusters, XWORD, XWORD
    Embed remainingClusters, Collection
ObjectEnd

Object DBSCANClusteringStrategy,, ClusteringStrategy
    RedefineMethod Cluster, $ObjPtr(DataSet), XWORD
    VirtualMethod  GetNeighbors, $ObjPtr(DataSet), POINTER

    VirtualMethod  ComputeEpsilon, $ObjPtr(DataSet)
    VirtualMethod  ComputeMinPoints, $ObjPtr(DataSet)
    DefineVariable eps, real8, 0.0
ObjectEnd

if IMPLEMENT

	Tiny_Random proto :DWORD
	.data
	    TinyRNG_x   dd  0
	    TinyRNG_a   dd  100711433
	.code
;	invoke Tiny_Random, 10				;Returns a random number from 0-9 in eax
	Tiny_Random proc uses xdx iRange
	   rdtsc
	   adc eax, edx
	   adc eax, TinyRNG_x
	   mul TinyRNG_a
	   adc eax, edx
	   mov TinyRNG_x, eax   
	   mul iRange
	   mov eax, edx
	   ret
	Tiny_Random endp
	
    Method ClusteringStrategy.Init, uses xsi
        SetObject xsi
        OCall [xsi].results::Collection.Init, xsi, 100, 50, COL_MAX_CAPACITY
        OCall [xsi].selectedPoints::Collection.Init, xsi, 100, 50, COL_MAX_CAPACITY
    MethodEnd
    Method ClusteringStrategy.Done, uses xsi
        SetObject xsi
        OCall [xsi].results::Collection.Done
        OCall [xsi].selectedPoints::Collection.Done
    MethodEnd

    Method KMeansClusteringStrategy.Init, uses xsi
        SetObject xsi
        
        ACall xsi.Init
        OCall [xsi].remainingPoints::Collection.Init, xsi, 100, 50, COL_MAX_CAPACITY
    MethodEnd
    Method KMeansClusteringStrategy.Done, uses xsi
        SetObject xsi
        ACall xsi.Done 
        OCall [xsi].remainingPoints::Collection.Done
    MethodEnd

    Method KMeansClusteringStrategy.Cluster, uses xsi, set : $ObjPtr(DataSet), clusters :XWORD
        local changed : XWORD
        local currentCluster : $ObjPtr(DataCluster)
        local selectedCluster : $ObjPtr(DataCluster)
        local grupo : $ObjPtr(DataCluster)
        local point : $ObjPtr(DataPoint)
        local d : REAL8, distance : REAL8, counter : XWORD
        local fSlvTLS()
        local fregTLS()
        
        SetObject xsi

        OCall xsi.Initialize, set, clusters
        mov counter, 0
        loop1i:
           
            lea xbx, [xsi].results
            .ColForEach [xbx]
                OCall xax::DataCluster.CalculateCentroid
            .ColNext
            
            mov changed, FALSE
            mov xbx, set
            .ColForEach [xbx]
                mov point, xax
                
                mov currentCluster, NULL
                freg_push xbx
                lea xbx, [xsi].results
                .ColForEach [xbx]
                    mov grupo, xax
                    OCall grupo::Collection.IndexOf, point
                    .if eax != -1
                        m2m currentCluster, grupo, @if64bits(xax)
                    .endif    
                .ColNext
                freg_pop xbx
                .if (currentCluster == NULL) 
                    mov changed, TRUE
                .endif

                fSlv8 d = 1.0e300 
                mov selectedCluster, NULL

                freg_push xbx
                lea xbx, [xsi].results
                .ColForEach [xbx]
                    mov grupo, xax
                    OCall grupo::DataCluster.GetCentroid
                    OCall point::DataPoint.ComputeDistance, xax
                    fstp distance
                    .if fGT(d, distance)
                        m2m selectedCluster, grupo, @if64bits(xax)
                        fSlv8 d = distance
                    .endif
                .ColNext    
                freg_pop xbx
                mov xdx, selectedCluster
                .if xdx != currentCluster
                    .if selectedCluster != NULL
                        OCall selectedCluster::DataCluster.Insert, point
                    .endif
                    .if currentCluster != NULL 
                        OCall currentCluster::DataCluster.Delete, point
                    .endif
                    mov changed, TRUE                  
                .endif                    
            .ColNext
            inc counter 
            .if counter > 200
                jmp loop1f
            .endif
            cmp changed, 0
            jz loop1f
        jmp loop1i
      loop1f:

        lea xax, [xsi].results
    MethodEnd

    Method KMeansClusteringStrategy.Initialize, uses xsi, set : $ObjPtr(DataSet), clusters : XWORD
        local nc : XWORD
        local cluster : $ObjPtr(DataCluster)
        local other : $ObjPtr(DataPoint)
        local point : $ObjPtr(DataPoint)
        local p : $ObjPtr(DataPoint)
        local k : DWORD
        local maximum : REAL8, minimum : REAL8
        local distance : REAL8, lastdistance : REAL8
        local fregTLS()
        local fSlvTLS()
        
        SetObject xsi
        
        OCall [xsi].results::Collection.DisposeAll
        OCall [xsi].selectedPoints::Collection.DisposeAll
        OCall [xsi].remainingPoints::Collection.DisposeAll

        mov xax, set
        mov eax, [xax].$Obj(DataSet).dCount
        dec eax
 
    	invoke Tiny_Random, eax 
    	mov k, eax 
        OCall set::DataSet.ItemAt, k
        mov p, xax 
        OCall [xsi].selectedPoints::Collection.Insert, p

        mov nc, 1
        
        New DataCluster
        mov cluster, xax
        OCall cluster::DataCluster.Init, xsi, nc        
        OCall cluster::DataCluster.Insert, p 

        OCall [xsi].results::Collection.Insert, cluster
        
        mov xbx, set 
        .ColForEach [xbx]
            OCall [xsi].remainingPoints::Collection.Insert, xax 
        .ColNext
        OCall [xsi].remainingPoints::Collection.Delete, p 
        mov xax, nc
 
        .while (xax < clusters)
            fSlv8 maximum = -1.0e300 
            mov other, NULL
            lea xbx, [xsi].remainingPoints
            .ColForEach [xbx]
                mov point, xax
                fSlv8 distance = 1.0e300
                fSlv8 minimum = 1.0e300
                
                freg_push xbx
                lea xbx, [xsi].selectedPoints
                .ColForEach [xbx]
                    OCall xax::DataPoint.ComputeDistance, point
                    .if fLT(lastdistance, minimum)
                        fSlv8 minimum = lastdistance
                    .endif
                .ColNext
                .if fGT(distance, maximum)
                    fSlv8 maximum = distance
                    m2m other, point, @if64bits(xax)
                .endif
                freg_push xbx
            .ColNext

            OCall [xsi].remainingPoints::Collection.Delete, other 
            OCall [xsi].selectedPoints::Collection.Insert, other

            inc nc
            New DataCluster
            mov cluster, xax 
       
            OCall cluster::DataCluster.Init, xsi, nc        
            OCall cluster::DataCluster.Insert, other
            
            OCall [xsi].results::Collection.Insert, cluster
            mov xax, nc
            
        .endw
    MethodEnd
    
    ;------------------------------------------------------------------    
    
    Method HierarchicalClusteringStrategy.Init, uses xsi
        SetObject xsi
        ACall xsi.Init
        OCall [xsi].remainingClusters::Collection.Init, xsi, 100, 50, COL_MAX_CAPACITY
    MethodEnd
 
    Method HierarchicalClusteringStrategy.Done, uses xsi
        SetObject xsi
        ACall xsi.Done 
        OCall [xsi].remainingClusters::Collection.Done
    MethodEnd
    
    Method HierarchicalClusteringStrategy.Cluster, uses xsi, set : $ObjPtr(DataSet), clusters : XWORD
        local nc : XWORD
        local point : $ObjPtr(DataPoint)
        local cluster : POINTER
        local remainingCluster : POINTER;$Obj(DataCluster)
        local clusterToMerge : POINTER;$Obj(DataCluster)
        local remainingToMerge : POINTER;$Obj(DataCluster)
        local minorCluster : $Obj(DataCluster)
        local otherCluster : POINTER;$Obj(DataCluster)
        local minimum : REAL8
        local distance : REAL8
        local fregTLS()
        local fSlvTLS()
        
        SetObject xsi

        mov nc, 1
        
        mov xbx, set
        .ColForEach [xbx]
            mov point, xax
            New DataCluster
            mov cluster, xax
            OCall cluster::DataCluster.Init, xsi, nc
            OCall cluster::DataCluster.Insert, point
            OCall [xsi].results::Collection.Insert, cluster
            inc nc
        .ColNext

        lea xax, [xsi].results
        mov eax, [xax].$Obj(Collection).dCount
        .while (xax > clusters)
            fSlv8 minimum = 1.0e300
            lea xbx, [xsi].results 
            .ColForEach [xbx]
                mov cluster, xax
                freg_push xbx

                lea xbx, [xsi].results
                .ColForEach [xbx]
                    mov remainingCluster, xax
                    .if xax != cluster
	                    mov _ecx, cluster
	                    mov _edx, remainingCluster
	                    OCall xsi.ComputeDistanceBetweenClusters, _ecx, _edx;remainingCluster
	                    fstp distance
	                    .if fLT(distance, minimum)
	                        fSlv8 minimum = distance
	                        m2m clusterToMerge, cluster, xax
	                        m2m remainingToMerge, remainingCluster, xax
	                    .endif
                    .endif
                .ColNext
                freg_pop xbx
            .ColNext
            
            mov xax, clusterToMerge
            mov xdx, [xax].$Obj(DataCluster).Cluster
            mov xcx, remainingToMerge
            .if xdx < [xcx].$Obj(DataCluster).Cluster
                mov xbx, xcx
                mov minorCluster, xax
                mov otherCluster, xcx
            .else    
                mov xbx, xax
                mov minorCluster, xcx
                mov otherCluster, xax
            .endif
            .ColForEach [xbx]
                mov _ecx, minorCluster
                OCall _ecx::Collection.Insert, xax     
            .ColNext
            mov _ecx, otherCluster     
            OCall [xsi].results::Collection.Delete, _ecx
            mov _ecx, otherCluster     
            OCall _ecx::Collection.DeleteAll
            mov _ecx, otherCluster     
            Destroy _ecx
            
            lea xax, [xsi].results
            mov eax, [xax].$Obj(Collection).dCount
        .endw
         lea xax, [xsi].results
  
    MethodEnd

    Method HierarchicalClusteringStrategy.ComputeDistanceBetweenClusters, uses xbx xsi, dataCluster1:XWORD, dataCluster2:XWORD
        local point1:$Obj(DataPoint)
        local minimum : REAL8
        local distance : REAL8
        local fSlvTLS()
        local fregTLS()
        
        SetObject xsi
        
        fSlv8 minimum = 1.0e300

        mov xbx, dataCluster1
        .ColForEach [xbx]
            mov point1, xax
            freg_push xbx
	        mov xbx, dataCluster2
	        .ColForEach [xbx]
                mov _ecx, point1
                OCall _ecx::DataPoint.ComputeDistance,xax
                fstp distance
                .if fLT(distance, minimum)
                    fSlv8 minimum = distance
                .endif
            .ColNext
            freg_pop xbx
        .ColNext

        fld minimum
    MethodEnd
    
 	Method DBSCANClusteringStrategy.Cluster, uses xsi xbx, set : $ObjPtr(DataSet), clusters : XWORD
        local nc : XWORD
        local index : DWORD
        local minPts : DWORD
        local visitedPoints : POINTER
        local noisePoints : POINTER
        local neighbors : POINTER
        local npointNeighbors : POINTER
        local npointNeighbor : POINTER
        local npoint : POINTER
        local point : POINTER
        local cluster : POINTER
        local fregTLS()
        
	    SetObject xsi

        OCall xsi.ComputeEpsilon, set
        fstp [xsi].eps 
        DbgFloat [xsi].eps
        
        OCall xsi.ComputeMinPoints, set
        mov minPts, eax
        
        DbgDec minPts
    
	    mov nc, 1
	    
	    New Collection
	    mov visitedPoints, xax
        OCall xax::Collection.Init, xsi, 100, 50, COL_MAX_CAPACITY
        
	    New Collection
	    mov noisePoints, xax
        OCall xax::Collection.Init, xsi, 100, 50, COL_MAX_CAPACITY

	    mov xbx, set
	    .ColForEach [xbx]
	        mov point, xax

	        mov _ecx, visitedPoints
	        OCall _ecx::Collection.IndexOf, point       
            .if eax != -1 
                jmp continue
            .endif
            
	        mov _ecx, visitedPoints
            OCall _ecx::Collection.Insert, point

            OCall xsi.GetNeighbors, set, point
            mov neighbors, xax
            mov eax, [xax].$Obj(Collection).dCount 
            .if eax < minPts
                OCall noisePoints::Collection.Insert, point
                jmp continue
            .endif

            inc nc
            New DataCluster 
            mov cluster, xax
            OCall cluster::DataCluster.Init, xsi, nc
            OCall cluster::DataCluster.Insert, point
            
            mov index , 0
            mov xax, neighbors            
            mov eax, [xax].$Obj(Collection).dCount 
            .while (index < eax)
                OCall neighbors::Collection.ItemAt, index
                mov npoint, xax 
                OCall noisePoints::Collection.IndexOf, npoint
                .if eax != -1 
                    OCall noisePoints::Collection.Delete, npoint
                .endif
                OCall visitedPoints::Collection.IndexOf, npoint
                .if eax != -1
                    jmp continue2
                .endif
                OCall visitedPoints::Collection.Insert, npoint

                OCall xsi.GetNeighbors, set, npoint
                mov npointNeighbors, xax
                mov eax, [xax].$Obj(Collection).dCount 
                .if eax >= minPts
                    freg_push xbx
                    mov xbx, npointNeighbors
                    .ColForEach [xbx]
                        mov npointNeighbor, xax
                        mov _ecx, neighbors         
                        OCall _ecx::Collection.IndexOf, npointNeighbor
                        .if eax == -1
                            mov _ecx, neighbors
                            OCall _ecx::Collection.Insert, npointNeighbor
                        .endif
                    .ColNext
                    freg_pop xbx
                .endif
                mov _ecx, cluster
                OCall _ecx::Collection.Insert, npoint
                mov eax, [_ecx].$Obj(Collection).dCount
                continue2:
                inc index
                mov xax, neighbors            
                mov eax, [xax].$Obj(Collection).dCount 
            .endw
	        OCall [xsi].results::Collection.Insert, cluster
	        continue:
        .ColNext

	    mov xax, noisePoints
	    .if [xax].$Obj(DataCluster).dCount > 0
	        OCall [xsi].results::Collection.Insert, xax
	    .endif    
        lea xax, [xsi].results

	MethodEnd
	
	Method DBSCANClusteringStrategy.GetNeighbors, uses xsi xbx, set : $ObjPtr(DataSet), point : POINTER
	    local distance : real8, p: POINTER
	    local neighbors : POINTER
	    local fSlvTLS()

	    SetObject xsi
	    
	    New Collection
	    mov neighbors, xax
        OCall neighbors::Collection.Init, xsi, 100, 50, COL_MAX_CAPACITY
	    
        mov xbx, set
        .ColForEach [xbx]
	        .if xax != point
	            mov p, xax
                OCall xax::DataPoint.ComputeDistance, point
                fstp distance
                .if fLE(distance, [xsi].eps)
                    OCall neighbors::Collection.Insert, p
                .endif    
	        .endif
        .ColNext
        mov xax, neighbors
	MethodEnd
	
	Method DBSCANClusteringStrategy.ComputeEpsilon, uses xsi xbx, set : $ObjPtr(DataSet)
	    local distance : real8, d : real8
	    local distances : POINTER, point : POINTER
	    local nb : DWORD
	    local fregTLS()
 	    local fSlvTLS()

      local i : xword
      SetObject xsi
      
      New Real8VectorS    
      mov distances, xax
      OCall xax::Real8VectorS.Init, xsi, 100, 50, COL_MAX_CAPACITY

      mov xbx, set
      .ColForEach [xbx]
        mov point, xax
        fSlv8 distance = 1.0e300
        freg_push xbx
        mov xbx, set
        .ColForEach [xbx]
          .if xax != point
            mov _ecx, point
            OCall _ecx::DataPoint.ComputeDistance, xax
            fstp d
            .if fLT(d, distance)
              fSlv8 distance = d
            .endif
          .endif
        .ColNext
        freg_pop xbx
        mov _ecx, distances 
        OCall _ecx::Real8VectorS.Insert, distance
      .ColNext

       mov xbx, distances 
	   fSlv nb = trunc(ceil([xbx].$Obj(Real8VectorS).dCount * 0.9))

      .VecForEach [xbx]::Real8VectorS
        DbgFloat real8 ptr [xax]
      .VecNext

      OCall xbx::Real8VectorS.ItemAt, nb
      freg_push xax
      Destroy distances
      freg_pop xax	            
      fld real8 ptr [xax]
	MethodEnd
	
	Method DBSCANClusteringStrategy.ComputeMinPoints, uses xsi xbx, set : $ObjPtr(DataSet)
	    local nb : DWORD
	    local numberPoints : POINTER
	    local value1 : DWORD
	    	
	    SetObject xsi
	    
	    New DwordVectorS
	    mov numberPoints, xax
        OCall xax::DwordVectorS.Init, xsi, 100, 50, COL_MAX_CAPACITY
   
        mov xbx, set
        .ColForEach [xbx]
            OCall xsi.GetNeighbors, set, xax
            mov eax, [xax].$Obj(Collection).dCount
            mov _ecx, numberPoints
            OCall _ecx::DwordVectorS.Insert, eax 
        .ColNext
        
        DbgLine
        mov xbx, numberPoints
        .VecForEach [xbx]::DwordVectorS
          DbgDec DWORD ptr [xax]
        .VecNext	
        DbgLine

	    mov xbx, numberPoints
	    fSlv nb = trunc([xbx].$Obj(DwordVectorS).dCount * 0.9)

	    OCall xbx::DwordVectorS.ItemAt, nb
	    mov xax, [xax]
	    mov value1, eax
	    Destroy numberPoints
	    mov eax, value1        
	MethodEnd


    
    
endif