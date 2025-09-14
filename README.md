# Data_Structure_HW06--M-Way-Search-Tree-and-B-Tree
Use C++ to implement a data structure program.  
Write a program with the C++ to implement m-way search tree and B-tree operations including insertion, deletion, and tree printing.  

本專案使用 C++ 實作 m-way 搜尋樹（m=3）與 B-樹，包括整數插入、刪除與樹狀結構列印。程式先輸入 m 值與多個整數建立樹之後開始互動操作。  

**輸入格式**  
輸入 m-way tree 的 m 值：  
m  
輸入整數數量（輸入 -1 停止）：  
N  
val1 val2 ... valN  
之後操作：  
輸入值插入 m-way tree：val  
輸入值刪除 m-way tree：val  
輸入值插入 B-tree：val  
輸入值刪除 B-tree：val  

**輸出範例**  
m-way搜尋樹 (建立):  
輸入：  
3  
10 20 5 6 15 30 25 50 40 35 -1  

輸出：  
m-way Tree:  
[ 20 ]  
[ 10 ] [ 30 50 ]  
[ 5 6 ] [ 15 ] [ 25 ] [ 35 40 ] [ ]  

m-way搜尋樹 (刪除):  
輸入：  
20 30 -1  

輸出：  
m-way Tree (delete **20**):  
[ 10 ]  
[ 6 ] [ 30 50 ]  
[ 5 ] [ 15 ] [ 25 ] [35 40 ] [ ]  

m-way Tree (delete **30**):  
[ 10 ]  
[ 6 ] [ 25 50 ]  
[ 5 ] [ 15 ] [ ] [35 40 ] [ ]  

