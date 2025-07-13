#include<iostream>
#include<fstream>
#include<chrono>
#include<pthread.h>
#include<time.h>

using namespace std;
using namespace std::chrono;

typedef struct{
    int tid;                //thread id useful while we are printing in output file
    bool innervalid;        //for storing validity that whether particular row,col,grid is valid or not
    int* line;              //store all  elements of particular row,col,grid to tell from where to start and end
    int size;               //size of line array
    high_resolution_clock::time_point starttime;   ////tell the starting time of execution, act as reference to check speed
}Array;


//for printing output in desired format as mentioned i use this datatype
typedef struct{
    int tid;                    //thread id useful while we are printing in output file
    int index;                  //index of row,col,grid
    bool innervalid;            //for storing validity that whether particular row,col,grid is valid or not
    microseconds totaltime;     //for storing total execution time(final - reference starting time)
    string whichthing;              //for storing method name
}Result;

//my explicit comparator which will sort result in ascending order according to execution time
static bool cmp(Result& a,  Result& b){
    return a.totaltime < b.totaltime;
}

//declaring global variable
int** sudoku;              //declaring sudoku Globally so no need to pass as an argument everytime
int N,K;                   //declaring N (dimension of sudoku) and no of threads K globally
int n;                     //n will be square root oF N , useful for subbox operation 
bool isMixedvalid =true;   //initializing with true if any method fails it becomes false and tell finally suoku is invalid
Result * results;          //to store validity of each row,col,grid like log




//mixed checking methods
void* rowcheckmixed(void *arguement){
    Array *var=(Array *)arguement;  //typecasting from void* to Array*
    int thread_id = var->tid;       //thread id for printing
    int* line = var->line;          //store all  elements of particular row,col,grid to tell from where to start and end
    int size = var->size;           //size of line array

    for(int i=0;i<size;i++){

        int* visited = new int[N+1];    //will strore frequency if i get duplicate element(freq!=1) means sudoku is invalid

        for(int i=0;i<=N;i++)visited[i]=0;  //initializing with zero

        var->innervalid = 1;
        int row = line[i];

        for(int col=0;col<N;col++){
            if(visited[sudoku[row][col]]==1){
                isMixedvalid = false;  //making global variable false
                var->innervalid = false;
                break;      //if found invalid no need to check further
            }
            visited[sudoku[row][col]]++;
        }
        delete[] visited;
        Result dummy = {thread_id + 1, row + 1, var->innervalid,chrono::duration_cast<microseconds>(high_resolution_clock::now() - var->starttime),"row"};
        results[row] = dummy;      //storing for ouputs
    }
    // delete[] line;
    pthread_exit(NULL);
}

void* colcheckmixed(void *arguement){
    Array *var=(Array *)arguement;  //typecasting from void* to Array*
    int thread_id = var->tid;
    int* line = var->line;   //has all elements of particular row,col,grid
    int size = var->size;

    for(int i=0;i<size;i++){

        int* visited = new int[N+1];   //will strore frequency if i get duplicate element(freq!=1) means sudoku is invalid

        for(int i=0;i<=N;i++)visited[i]=0;

        var->innervalid = 1;
        int col = line[i];

        for(int row=0;row<N;row++){
            if(visited[sudoku[row][col]]==1){
                isMixedvalid = false;     //making global variable false
                var->innervalid = false;
                break;      //if found invalid no need to check further , optimization
            }
            visited[sudoku[row][col]]++;
        }
        delete[] visited;
        Result a = {thread_id + 1, col + 1, var->innervalid,chrono::duration_cast<microseconds>(high_resolution_clock::now() - var->starttime),"coloumn"};
        results[col+N] = a;     //col+N because we are storing it like first all N rows so there index is 0 to N-1 , now for coloumn starts from N to 2N-1
    }
    // delete[] line;
    pthread_exit(NULL);
}

void* blockcheckmixed(void *arguement){
     //i am using block term instead of grid because it is easy to understand that i am just thinking grids as 0 to N-1 blocks
    Array *var=(Array *)arguement;  //give me start block and end block, typecasting from void* to pair<int,int>*
    int thread_id = var->tid;
    int* line = var->line;
    int size = var->size;

    for(int i=0;i<size;i++){
        int block = line[i];
        int blockrow =( block/n)*n; 
        int blockcol =( block%n)*n;
        /* above formula came from observation suppose 9*9 sudoku so we have 9 blocks of 3*3 each ,index then 0,1,2,3,4,5,6,7,8
            after 3 row changes and increment by 1 some remainder after 3 intution so % to find remainder 
            after 0,1,2,col again become 0,1,2 so it is like
            [0,0],[0,1],[0,2],[1,0],[1,1],[1,2],[2,0],[2,1],[2,2] if i multiply by 3 (or n) then it becomes
            [0,0],[0,3],[0,6],[3,0],[3,3],[3,6],[6,0],[6,3],[6,6] which are actually index of sudoku telling from where block starts
        */ 

        int* visited = new int[N+1];

        for(int i=0;i<=N;i++)visited[i]=0;
        var->innervalid = 1;

        for(int row=blockrow ;row<blockrow+n; row++){ 

            for(int col=blockcol;col<blockcol+n;col++){
                if(visited[sudoku[row][col]]==1){
                    isMixedvalid = false;
                    var->innervalid = false;
                    break;
                }
                visited[sudoku[row][col]]++;
            }
        }
        delete[] visited;
        Result a = {thread_id + 1, block + 1, var->innervalid,chrono::duration_cast<microseconds>(high_resolution_clock::now() - var->starttime),"subgrid"};
        results[block+2*N] = a;
    }
    // delete[] line;
    pthread_exit(NULL);
}


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
    read_object >> K >> N;

    n=sqrt(N);  // n will be square root of N needed for subgrid checking
    results = (Result *) malloc(3*N*sizeof(Result));      //dynamically allocating memory, 3N because we print result for each row(N) + each coloumn(N) + each subgrid/box(N) so total 3N

    //checking whether N is perfect square beacaue if it is not we cannot validate for sudoku box chack operation
    if(N!=n*n){
        cout<<"N is not a perfect square, please enter a perfect square\n";
        return 0;
    }
    if(K<3){
        cerr<<"K must be at least 3 to divide into 3 groups"<<endl;
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


    int K1,K2,K3;  //dividing K int 3 groups

    //PART2  compuations tasks

    pthread_t threads[K];       //Taking K number of threads


    //We have to options either starts time from main to end of main or consider time for computation of each thread , i choose 2nd option because file reading and printing is not what our focus is
    auto start_time_mixed = high_resolution_clock::now();


    //dividing K into 3 parts k1,k2,k3
    K1 = ceil(K / 3.0), K2 = ceil((K - K1) / 2.0), K3 = K - K1 - K2;
    /*Ex - k=8 k1=ceil(8/3)->3 k2=ceil(5/2)->3 k3=ceil(2/1)->2*/

    // k1 threads will perform row operation from starting index to ending index which i am storing in rowsplit
    Array rowsplit[K1],colsplit[K2],boxsplit[K3];  

    //initialising all of them
    for (int i =0; i < K1; i++){
        rowsplit[i].tid = i;   //storing thread id like indexing 0 to k ,not actual id
        rowsplit[i].line = new int[N/K1+1];  //each is of chunk size and adding +1 if it is not divisible
        rowsplit[i].size =0;   //iniitially size is 0 means no cell data
        rowsplit[i].innervalid=1;   //initally true if any false come it will change
    }
    for (int i =0; i < K2; i++){
        colsplit[i].tid = i + K1;   //it is 0 base indexing so adding K1 because first K1 threads are for row
        colsplit[i].line = new int[N/K2+1];
        colsplit[i].size =0;
        colsplit[i].innervalid=1;
    }
    for (int i =0; i < K3; i++){
        boxsplit[i].tid = i + K1 + K2;  //it is 0 base indexing so adding K1+K2 because first K1+K2 threads are for row and col
        boxsplit[i].line = new int[N/K3+1];
        boxsplit[i].size =0;
        boxsplit[i].innervalid=1;
    }

    // for mixed we see general pattern due to cyclicity , all indexes of same group have same remainder , we are using this proprty
    //ex - k=8,N=9 k1=3,k2=3,k3=2 for k1 =3 [0,3,6]has rem=0,[1,4,7]has rem=1,[2,5,8] has rem=2 for k2=3 [0,3,6]has rem=0,[1,4,7]has rem=1,[2,5,8] has rem=2 for k3=2 [0,2,4,6,8 ]has rem=0,[1,3,5,7]has rem=1
    for(int idx=0;idx<N;idx++){
        rowsplit[idx%K1].line[rowsplit[idx%K1].size++]=idx;   //this thing rowsplit[idx%K1].size++ is just assigning index to rowsplit and then increasing size by 1 ,doing in one line
        colsplit[idx%K2].line[colsplit[idx%K2].size++]=idx;
        boxsplit[idx%K3].line[boxsplit[idx%K3].size++]=idx;
    }

    for(int id=0;id<K1;id++){
        pthread_create(&threads[id],NULL,rowcheckmixed,(void *)(&rowsplit[id]));  
    }

    for(int id=0;id<K2;id++){
        pthread_create(&threads[id+K1],NULL,colcheckmixed,(void *)(&colsplit[id])); 
        //for colsplit id because colsplit is of size k2 so it's index go from 0 to k2-1 
        //for thread id go from k1 to k1+k2-1 beacause 0 to K1-1 are already created
    }

    for(int id=0;id<K3;id++){
        pthread_create(&threads[id+K1+K2],NULL,blockcheckmixed,(void *)(&boxsplit[id])); 
        //for for blocksplit id because blocksplit is of size k3 so it's index go from 0 to k3-1 
        //for thread id go from k1+k2 to k1+k2+k3-1 beacause 0 to K1+k2-1 are already created
    }

    //Joining them all , it will wait for all threads to finish
    for(int id=0;id<K;id++)pthread_join(threads[id],NULL);

    
    // clock_t end_time_mixed=clock();
    auto end_time_mixed = high_resolution_clock::now();

    auto total_time = duration_cast<std::chrono::microseconds>(end_time_mixed - start_time_mixed).count();  //give final-start time in microseconds
    //we can use clock also but chrono is little precise that's why i use high_resolution_clock

    //PART 2  finished (all computation of threads are done)

    //PART 3 printing output


    //sorting it according to execution time in ascending order by comparator
    sort(results,results+3*N,cmp);

    for(int i=0;i<3*N;i++){
        cout<<"Thread " <<results[i].tid<<" "<<"checks  "<<results[i].whichthing<<" "<<results[i].index<<" and it is "<<(results[i].innervalid == 1 ? "valid" : "invalid") << " time taken is "<<" and wrt to starting time "<<results[i].totaltime.count()<<" passed"<<endl;
    }
    isMixedvalid== true ? cout<<"Sudoku is valid\n" : cout<<"Sudoku is not valid\n";

    cout<<"Mixed method , Total time taken is "<<total_time<<" microseconds"<<endl;

    // cout<<"Total time taken is "<<total_time<<" microseconds"<<endl;

    //deleting dynamically allocated arrays
    for (int i = 0; i < K1; i++) delete[] rowsplit[i].line;
    for (int i = 0; i < K2; i++) delete[] colsplit[i].line;
    for (int i = 0; i < K3; i++) delete[] boxsplit[i].line;

    for(int i=0;i<N;i++){
        delete[] sudoku[i];
    }
    delete[] sudoku;

    return 0;

}