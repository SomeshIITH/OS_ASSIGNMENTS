#include <iostream>
#include<fstream>
#include <ctime>
using namespace std;

int main(){
    srand(time(0)); // Seed the random number generator

    int N ,K ;
    cout<<"Enter the number of threads (K): ";
    cin>>K;
    cout<< "Enter the size of the Sudoku grid (N): ";
    cin>>N;

    //start generating matrix
    
    ofstream write;
    write.open("inp.txt");
    write<< K << " " << N<<endl;

    // Fill the grid with random numbers
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            write<<( (rand() % N) + 1); // Random number between 1 and N
            write<< " ";

        }
        write << "\n";
    }
    write.close();
    return 0;
    
}