Readme file : 

I have written my code in C++. 

To create sudoku and assigning Number of thread , i used
Create_matrix.cpp file which takes k (number of threads) and N(dimension of sudoku) as input from user
and create inp.txt file with 1st row having K AND N value and then sudoku(N*N) is written on next line.

For matrix creation run following command in compiler
	g++ Create_matrix.cpp -o matrix 
	./matrix

After input file is created by above method run whatever method you want


For chunk method run following command in compiler
	g++ Assign1Src-CO23BTECH11022-chunk.cpp -o chunk -std=c++17
	./chunk

For mixed method run following command in compiler
	g++ Assign1Src-CO23BTECH11022-mixed.cpp -o mixed -std=c++17
	./mixed

For sequential method run following command in compiler
	g++ Assign1Src-CO23BTECH11022-sequential.cpp -o sequential -std=c++17
	./sequential

For chunk early termination run
	g++ Assign1Src-CO23BTECH11022-earlytermination-chunk.cpp -o early -std=c++17
	./early


	-std=c++17 used to handle auto



In output.txt file

As you run chunk or mixed method as above , they will create output.txt file automatically in which it tell
Thread ( number ) checks row/coloum/subgrid (number or there index+1) and it is valid/invalid. -->this will print 3* N TIMES
sudoku is valid/invalid.   -->in second last row
Methodname method , Total time taken is ___ microseconds  ->in last row 


For seguential as no thread is used it will print like
row/coloumn/subgrid is valid/invalid  -->3*N TIMES
sudoku is valid/invalid.   -->in second last row
Methodname method , Total time taken is ___ microseconds  ->in last row 