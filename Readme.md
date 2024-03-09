# Comparing K-Means and Others Algorithms for Data Clustering in Assembly.

[![](https://img.shields.io/badge/Assembler-UASM%20v2.52-green.svg?style=flat-square&logo=visual-studio-code&logoColor=white&colorB=1CC887)](http://www.terraspace.co.uk/uasm.html) 

Framework : ObjAsm C.2  - masters : [Main Developer](https://github.com/ObjAsm/ObjAsm-C.2) , [Fork](https://github.com/ASMHSE/ObjAsm-C.2/tree/master)
                        - working version [6 march 2024](https://github.com/ASMHSE/ObjAsm-C.2/tree/patch-2)   

Some weeks ago I found the article: [Comparing K-Means and Others Algorithms for Data Clustering](https://www.codeproject.com/Articles/5375470/Comparing-K-Means-and-Others-Algorithms-for-Data-C) by Nicolás Descartes. With C# source code.

Look very interesting to translate that to Assembly. Still is a work in progress but look well.

There is some kind of abuse of Collections, because is very easy to write that in C#. I removed most obvious exagerations (perhaps author try to make more clear the algorithm, not sure).

- K-Means strategy need initial randomness, then probably you have to run several times before to find the better solution.

- Hierarchical strategy need to define termination, and in these cases are number of clusters.

- Density-based spatial clustering of applications with noise really benefit from Collections, and also need a couple of Sorted Vectors.

Anyway is a good challenge, and still I have to collect some leaks

Original discussion can be found in [Masm32 Forum](https://masm32.com/board/index.php?topic=11649.0)

Any sugestion or improvement is welcome!
