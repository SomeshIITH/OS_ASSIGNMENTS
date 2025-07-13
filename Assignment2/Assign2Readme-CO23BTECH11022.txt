Readme file : 

I have written my code in C++. 

To create sudoku and assigning Number of thread , TA should use this
Create_matrix.cpp file which takes k (number of threads) and N(dimension of sudoku) and TaskInc as input from user
and create inp.txt file with 1st row having K AND N taskInc value and then sudoku(N*N) is written on next line.

For matrix creation run following command in compiler
	g++ Create_matrix.cpp -o matrix 
	./matrix

This will create random numbers sudoku which is invalid mostly

After input file is created by above method run whatever method you want

Or TA can give valid sudoku as inp.txt as per Asssignment format 


For TAS method run following command in compiler
	g++ Assgn2Src-CO23BTECH11022-TAS.cpp -o TAS -std=c++17 ; ./TAS

For CAS method run following command in compiler
	g++ Assgn2Src-CO23BTECH11022-CAS.cpp -o CAS -std=c++17; ./CAS

For BOUNDED CAS method run following command in compiler
	g++ Assgn2Src-CO23BTECH11022-BOUNDEDCAS.cpp -o BCAS -std=c++17 ;./BCAS

For sequential method run following command in compiler
	g++ Assgn2Src-CO23BTECH11022-sequential.cpp -o seq -std=c++17; ./seq


	-std=c++17 used to handle auto, Because auto is in above or in c++11 version



All of above method create output.txt file , if it's not present , else clear and write on it

For seguential as no thread is used it will print like
row/coloumn/subgrid is valid/invalid  -->3*N TIMES
sudoku is valid/invalid.   -->in second last row
The total time taken is ___ microseconds  ->in last row 


For TAS, CAS, BOUNDEDCAS 
print whatever operation got performed first according to given syntax in asssignment but in microseconds(To check what actually going on)

Then at last lines it print 
    Sudoku is valid or not
    Total execution time: __ microseconds
    Average time taken by a thread to enter the CS is __ microseconds
    Average time taken by a thread to exit the CS is __ microseconds
    Worst-case time taken by a thread to enter the CS is __ microseconds
    Worst-case time taken by a thread to exit the CS is __ microseconds

