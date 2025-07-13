#include<iostream>
#include<fstream>
#include<pthread.h>
#include<time.h>

using namespace std;
using namespace std::chrono;


//declaring sudoku Globally so no need to pass as an argument everytime
int** sudoku;
//declaring N (dimension of sudoku) and no of threads K globally
int N,K,taskInc;
int n;  //n will be square root oF N , useful for subbox operation 


bool  isSequentialvalid = true; //initializing with true if any method fails it becomes false


int main(){
    //PART 1

    freopen("output.txt", "w", stdout); // Redirect stdout to output.txt

    // Reading from input file by read_object of ifstream class
    ifstream read_object("inp.txt");

    // Check whether it file opens or not
    if (!read_object) {
        cerr << "Some Error happened while opening input.txt\n";
        return 1;
    }

    // Read N and K from the input.txt first line
    read_object >> K >> N>>taskInc;
    n=sqrt(N);  // n will be square root of N

    //Just checking whether N is perfect square beacaue if it is not we cannot validate for sudoku box chack operation
    if(N!=n*n){
        cout<<"N is not a perfect square, please enter a perfect square\n";
        return 0;
    }

    //making of 2d sudoku matrix dynamically
    sudoku = new int*[N];

    for (int i = 0; i < N; i++)sudoku[i] = new int[N];

    //Taking values from input.txt and storing in sudoku matrix
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            read_object >> sudoku[i][j];
        }
    }

    //closing the input.txt file
    read_object.close();

    bool rowvalidseq=true,colvalidseq=true,blockvalidseq=true;

    //PART2 computation tasks

    auto starting_time = high_resolution_clock::now();

    //validating all rows
    for(int row=0;row<N;row++){

        if(isSequentialvalid==false)break;     //ensures early termination

        rowvalidseq=1;
        int* visited = new int[N+1];
        for(int i=0;i<=N;i++)visited[i]=0;  //will strore frequency if i get duplicate element(freq!=1) means sudoku is invalid

        for(int col=0;col<N;col++){
            if(visited[sudoku[row][col]]==1){
                isSequentialvalid = false;
                rowvalidseq=0;
                break;
            }
            visited[sudoku[row][col]]++;
        }
        cout<<"row "<<row+1<<" is checked at time "<<duration_cast<microseconds>(high_resolution_clock::now()-starting_time).count() <<" microseconds and finds it as "<<(rowvalidseq==1 ? "valid\n" : "invalid\n");

        if(isSequentialvalid==false)break;
    }
    
    //checking for all coloumns
    for(int col=0;col<N;col++){

        if(isSequentialvalid==false)break;

        colvalidseq=1;
        int* visited = new int[N+1];
        for(int i=0;i<=N;i++)visited[i]=0;

        for(int row=0;row<N;row++){
            if(visited[sudoku[row][col]]==1){
                isSequentialvalid = false;
                colvalidseq=0;
                break;
            }
            visited[sudoku[row][col]]++;
        }
        cout<<"coloumn "<<col+1<<" is checked at time "<<duration_cast<microseconds>(high_resolution_clock::now()-starting_time).count() <<" microseconds and finds it as "<<(colvalidseq==1 ? "valid\n" : "invalid\n");

        if(isSequentialvalid==false)break;
    }

    //checking for all subgrids , i am using term boxes for n*n subgrids as there index number
    for(int box=0;box<N;box++){

        if(isSequentialvalid==false)break;

        blockvalidseq=1;  //tell each time whether particular box is valid or not, initally assume valid and then if not make false
        int row = (box/n)*n; //formula come from observation , as each row number increase after n boxes so division and coloumn vary between 0 to n-1 so moduloding by n
        int col = (box%n)*n;  //multiplying by n for both give grid 1st cell index number
        int* visited = new int[N+1];

        for(int i=0;i<=N;i++)visited[i]=0;

        for(int nrow=row;nrow<row+n;nrow++){  //+n for onesubgrid only

            for(int ncol=col;ncol<col+n;ncol++){

                if(visited[sudoku[nrow][ncol]]==1){
                    isSequentialvalid = false;
                    blockvalidseq=0;
                    break;
                }
                visited[sudoku[nrow][ncol]]++;
            }
        } 
        cout<<"subgrid "<<box+1<<" is checked at time "<<duration_cast<microseconds>(high_resolution_clock::now()-starting_time).count() <<" microseconds and finds it as "<<(blockvalidseq==1 ? "valid\n" : "invalid\n");

        if(isSequentialvalid==false)break;
    }

    auto ending_time = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(ending_time - starting_time);  
    //we can use clock also but chrono is little precise that's why i use high_resolution_clock


    //PART 2 ends

    isSequentialvalid == 1 ? cout<<"Sudoku is valid\n" : cout<<"Sudoku is not valid\n";
    printf("The total time taken is %lld microseconds\n", duration.count());

    fclose(stdout);

    // FILE *writeobject;
    // writeobject = fopen("output1.txt", "a");

    // fprintf(writeobject,"Total time taken is: %f microseconds\n", (double)duration.count());

    // fclose(writeobject);


    for(int i=0;i<N;i++){
        delete[] sudoku[i];
    }
    delete[] sudoku;

}