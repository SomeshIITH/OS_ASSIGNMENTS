#include<iostream>
#include<fstream>
#include<chrono>
#include<pthread.h>
#include<time.h>

using namespace std;   
using namespace std::chrono;  //for calculating 

typedef struct{
    int start;          //tell thread from where to start, it can be startrow,startcol,and startgrid
    int end;            //tell thread from where to end, it can be endrow,endcol,and endgrid
    bool innervalid;    //for storing validity that whether particular row,col,grid is valid or not
    int tid;            //thread id useful while we are printing in output file
    high_resolution_clock::time_point starttime;  //tell the starting time of execution, act as reference to check speed
}Data;


//for printing output in desired format as mentioned i use this datatype
typedef struct{
    int tid;                    //thread id useful while we are printing in output file
    int index;                  //index of row,col,grid
    bool innervalid;            //for storing validity that whether particular row,col,grid is valid or not
    microseconds totaltime;     //for storing total execution time(final - reference starting time)
    string whichthing;              //for storing method name
}Result;

void signal_handler(int signum){
    pthread_exit(NULL);
}

//my explicit comparator which will sort result in ascending order according to execution time
static bool cmp( Result& a,  Result& b){
    return a.totaltime < b.totaltime;
}


//global variables
int** sudoku;              //declaring sudoku Globally so no need to pass as an argument everytime
int N,K;                   //declaring N (dimension of sudoku) and no of threads K globally
int n;                     //n will be square root oF N , useful for subbox operation 
bool isChunkvalid =true;   //initializing with true if any method fails it becomes false and tell finally suoku is invalid
Result * results;          //to store validity of each row,col,grid like log



void* rowcheckchunk(void *arguement){
    if(isChunkvalid==false){
         pthread_exit(NULL);
    }
    Data* var=(Data *)arguement;    //typecasting from void* to Data*
    int startrow=var->start;        //startrow of sudoku
    int endrow=var->end;            //endrow of sudoku
    int thread_id = var->tid;       //thread id for printing

    for(int row= startrow;row <= endrow ;row++){

        int* visited = new int[N+1];  //will strore frequency if i get duplicate element(freq!=1) means sudoku is invalid

        for(int i=0;i<=N;i++)visited[i]=0;   //initializing with zero

        var->innervalid = 1;

        for(int col=0;col<N;col++){
            if(visited[sudoku[row][col]]==1){
                var->innervalid = false;
                isChunkvalid = false;   //making global variable false
                break;  //if found invalid no need to check further
            }
            visited[sudoku[row][col]]++;
        }
        delete[] visited;
        Result dummy = {thread_id + 1, row + 1, var->innervalid,chrono::duration_cast<microseconds>(high_resolution_clock::now() - var->starttime),"row"};
        results[row]=dummy;  //for storing for ouputs
        if(isChunkvalid==false){
            pthread_kill(pthread_self(), SIGUSR1);  // Signal other threads
        }
    }

    pthread_exit(NULL);
}

void* colcheckchunk(void *arguement){
        if(isChunkvalid==false){
         pthread_exit(NULL);
        }
    Data* var=(Data*)arguement;  // typecasting from void* to Data*
    int startcol=var->start;
    int endcol=var->end;  
    int thread_id = var->tid; 

    for(int col = startcol ;col <= endcol ;col++){

        var->innervalid = 1;
        int* visited = new int[N+1];

        for(int i=0;i<=N;i++)visited[i]=0;

        for(int row=0;row<N;row++){
            if(visited[sudoku[row][col]]==1){
                isChunkvalid = false;        //making global variable false
                var->innervalid = false;
                break;          //if found invalid no need to check further
            }
            visited[sudoku[row][col]]++;
        }
        delete[] visited;
        Result a = {thread_id + 1, col + 1, var->innervalid,chrono::duration_cast<microseconds>(high_resolution_clock::now() - var->starttime),"coloumn"};
        results[col+N] = a;    //col+N because we are storing it like first all N rows so there index is 0 to N-1 , now for coloumn starts from N to 2N-1
        if(isChunkvalid==false){
            pthread_kill(pthread_self(), SIGUSR1);  // Signal other threads
        }
    }
    pthread_exit(NULL);
}

void* blockcheckchunk(void *arguement){
    if(isChunkvalid==false){
        pthread_exit(NULL);
        //break;
    }
    //i am using block term instead of grid because it is easy to understand that i am just thinking grids as 0 to N-1 blocks
    Data *var=(Data *)arguement;  //typecasting from void* to Data*
    int startblock=var->start;    //starting block 
    int endblock=var->end;  
    int thread_id = var->tid;

    for(int block = startblock ;block <= endblock ;block++){  
        int blockrow =( block/n)*n; 
        int blockcol =( block%n)*n;
        /* above formula came from observation suppose 9*9 sudoku so we have 9 blocks of 3*3 each ,index then 0,1,2,3,4,5,6,7,8
            after 3 row changes and increment by 1 some remainder after 3 intution so % to find remainder 
            after 0,1,2,col again become 0,1,2 so it is like
            [0,0] ,[0,1],[0,2],[1,0],[1,1],[1,2],[2,0],[2,1],[2,2] if i multiply by 3 (or n) then it becomes
            [0,0] ,[0,3],[0,6],[3,0],[3,3],[3,6],[6,0],[6,3],[6,6] which are actually index of sudoku telling from where block starts
        */ 

        int* visited = new int[N+1];

        for(int i=0;i<=N;i++)visited[i]=0;
        var->innervalid = 1;

        for(int row=blockrow ;row<blockrow+n ;row++){ 

            for(int col=blockcol;col<blockcol+n;col++){
                if(visited[sudoku[row][col]]==1){
                    isChunkvalid = false;
                    var->innervalid = false;
                    break;
                }
                visited[sudoku[row][col]]++;
            }
        }
        delete[] visited;
        Result a = {thread_id + 1, block + 1, var->innervalid,chrono::duration_cast<microseconds>(high_resolution_clock::now() - var->starttime),"grid"};
        results[block+2*N] = a;
        if(isChunkvalid==false){
            pthread_kill(pthread_self(), SIGUSR1);  // Signal other threads
        }
    }
    pthread_exit(NULL);
}


int main(){

    //PART 1

    freopen("output.txt", "w", stdout);   // Redirect stdout to output.txt

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
    results = (Result *) malloc(3*N*sizeof(Result));  //dynamically allocating memory, 3N because we print result for each row(N) + each coloumn(N) + each subgrid/box(N) so total 3N
    for(int i=0;i<3*N;i++)results[i].whichthing="NOTHING";

    //J checking whether N is perfect square beacaue if it is not we cannot validate for sudoku box chack operation
    if(N!=n*n){
        cerr<<"N is not a perfect square, please enter a perfect square\n";
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


    //PART 2

    int P = N / K;         // Defining chunk size

    int K1,K2,K3,P1,P2,P3;  //dividing K into 3 groups k1,k2,k3 and P1,P2,P3 which tell chunk size
    int rem1,rem2,rem3;

    pthread_t threads[K]; //Taking K number of threads

    //We have to options either starts time from main to end of main or consider time for computation of each thread , i choose 2nd option because file reading and printing is not what our focus is
    auto starting_time = high_resolution_clock::now();

    // Install signal handler for early exit
    signal(SIGUSR1, signal_handler);

    K1 = ceil(K / 3.0), K2 = ceil((K - K1) / 2.0), K3 = K - K1 - K2;
    P1 = N/K1, P2 = N/K2, P3 = N/K3;  
    /* Ex- k=8 k1=ceil(8/3)->3 k2=ceil(5/2)->3 k3=ceil(2/1)->2 */
    rem1 = N%K1 ,rem2 = N%K2 ,rem3 = N%K3;
    /*suppose N=1024 , k=342 then K1 = 114,k2 =114,k3=114 but rem1 = 112,rem2 = 112,rem3 = 112 which means for group1 K1*p1 = 912 SO
    we have to assign all left rows to last thread which means non uniformity in distribution so , we can give all thread 1 extra operation if remainder>0*/

    Data rowsplit[K1],colsplit[K2],blocksplit[K3];  

    // k1 threads will perform row operation from starting index to ending index which i am storing in rowsplit
    int startidx=0,endidx=0;
    for(int i=0;i<K1;i++){  //i is  helping in alloting N rows into K1 parts
        rowsplit[i].start=startidx;
        endidx = startidx + P1 -1 + (rem1>0 ? 1 :0);    //if(remainder>0)means last one has to do all leftover operations , so we are dividing its works to all others by giving 1 extra  
        if (endidx >= N) endidx = N - 1;
        rowsplit[i].end = endidx;
        rem1--;
        startidx = endidx+1;

        rowsplit[i].starttime = starting_time;
        rowsplit[i].tid=i;
    }

    // k2 threads will perform col operation from starting index to ending index which i am storing in colsplit
    startidx=0,endidx=0;
    for(int i=0;i<K2;i++){  //i is again helps in alloting N coloumns into K2 parts
        colsplit[i].start=startidx;
        endidx = startidx + P2 -1 + (rem2>0 ? 1 :0);
        if (endidx >= N) endidx = N - 1;
        colsplit[i].end = endidx;
        rem2--;
        startidx = endidx+1;

        colsplit[i].starttime = starting_time;
        colsplit[i].tid=i+K1;
    }

    // k3 threads will perform block operation from starting index to ending index which i am storing in blocksplit
    startidx=0,endidx=0;
    for(int i=0;i<K3;i++){  //i is just variable which again helps in alloting N blocks into K3 parts
        blocksplit[i].start=startidx;
        endidx = startidx + P3 -1 + (rem3>0 ? 1 :0);
        if (endidx >= N) endidx = N - 1;
        blocksplit[i].end = endidx;
        rem3--;
        startidx = endidx+1; 

        blocksplit[i].tid=i+K1+K2;
        blocksplit[i].starttime = starting_time;
    }

    //creating k1 threads in chunk
    for(int id=0;id<K1;id++){
        pthread_create(&threads[id],NULL,rowcheckchunk,(void *)(&rowsplit[id]));
    }

    //creating k2 threads in chunk
    for(int id=K1;id<(K1+K2);id++){
        pthread_create(&threads[id],NULL,colcheckchunk,(void *)(&colsplit[id-K1]));
        //for colsplit id-k1 because colsplit is of size k2 so it's index go from 0 to k2-1 
        //for thread id go from k1 to k1+k2-1 beacause 0 to K1-1 are already created
    }
    //creating k3 threads in chunk
    for(int id=K1+K2;id<(K1+K2+K3);id++){
        pthread_create(&threads[id],NULL,blockcheckchunk,(void *)(&blocksplit[id-(K1+K2)]));
        //for for blocksplit id-k1-k2 because blocksplit is of size k3 so it's index go from 0 to k3-1 
        //for thread id go from k1+k2 to k1+k2+k3-1 beacause 0 to K1+k2-1 are already created

    }
    //Joining them all , it will wait for all threads to finish
    for(int id =0;id<K;id++){
        pthread_join(threads[id],NULL);
    }


    
    auto ending_time = high_resolution_clock::now();
    //we can use clock also but chrono is little precise that's why i use high_resolution_clock
    auto duration = duration_cast<microseconds>(ending_time - starting_time);   //give final-start time in microseconds

    //PART 2 finished (all computation of threads are done)

    //PART 3 prining results

    //sorting it according to execution time in ascending order by comparator
    sort(results,results+3*N,cmp);

    //N rows N coloumns N blocks TOTAL 3N
    for(int i=0;i<3*N;i++){
        if(results[i].whichthing=="NOTHING")continue;
        cout<<"Thread " <<results[i].tid<<" "<<"checks  "<<results[i].whichthing<<" "<<results[i].index<<" and it is "<<(results[i].innervalid == 1 ? "valid" : "invalid") << " time taken is "<<" and wrt to starting time "<<results[i].totaltime.count()<<" passed"<<endl;
    }
    isChunkvalid==true ? cout<<"Sudoku is valid\n" : cout<<"Sudoku is not valid"<<endl;

    cout<<"Chunk method , Total time taken is "<<duration.count()<<" microseconds"<<endl;
    


    for(int i=0;i<N;i++){
        delete[] sudoku[i];
    }
    delete[] sudoku;

    return 0;

}
