#include<iostream>  
#include<fstream>
#include<chrono>  //for calculating time
#include<vector>
#include<pthread.h>  //for threads
#include<time.h>
#include<atomic>  //for atomic operations

using namespace std;   
using namespace std::chrono;  //for calculating accurate time


//global variables
int** sudoku;                      //declaring sudoku Globally so no need to pass as an argument everytime
int N,K,taskInc;                   //declaring N (dimension of sudoku) , no of threads K , taskInc is task increment globally
int n;                             //n will be square root oF N , useful for subgrid operation 
bool Sudoku_Is_Valid =true;        //initializing with true if any method fails it becomes false and tell finally suoku is invalid

int Total_CS_entry =0 ;             //total number of times all threads entered Critical Section
double Total_Time_To_Enter_CS=0;    //total time taken by all threads to enter Critical Section
double Total_Time_To_Exit_CS =0;    //total time taken by all threads to exit Critical Section
double Worst_CS_Entry_time =0;      //worst time taken by any thread to enter Critical Section
double Worst_CS_Exit_time =0;       //worst time taken by any thread to exit Critical Section
int C_value = 0;             


high_resolution_clock::time_point BASE_START_TIME;     //After reading input file , we will start the time and take as base time for program


atomic<int> C(0);                                      // Shared counter for dynamic task allocation initially 0
atomic<int> BCAS_LOCK(0);
vector<bool> waiting;

int expected=0;
int new_val=1;

// BCAS Lock Implementation
void lock_BCAS(int thread_id) {
    if(Sudoku_Is_Valid==false)return;
    waiting[thread_id] = true;
    int expected = 0; // Initialize expected value to 0

    while (waiting[thread_id] && !BCAS_LOCK.compare_exchange_strong(expected, new_val)) {
        expected = 0; // Reset expected value before next attempt
    }

    waiting[thread_id] = false;
}

void unlock_BCAS(int thread_id) {
    int j = (thread_id + 1) % K;
    while ((j != thread_id) && !waiting[j])
        j = (j + 1) % K;
    if (j == thread_id)
        BCAS_LOCK.store(expected);
    else
        waiting[j] = false;
}

// Row, Column, and Subgrid Checking Functions
void rowcheck(int index,int thread_id){

    int* visited = new int[N+1]; 
    for(int i=0;i<=N;i++)visited[i]=0;               //initializing visited array with 0, to keep track of visited numbers
    bool valid=true;
    
    for(int col=0;col<N;col++){
        if(visited[sudoku[index][col]]==1){
            valid = false;
            Sudoku_Is_Valid = false;                //making global variable false
            break;                                  //if found invalid no need to check further to save some
        }
        visited[sudoku[index][col]]++;
    }
    printf("Thread %d completes checking of row %d at time %lld microseconds and finds it as %s\n",thread_id, index+1, duration_cast<microseconds>(high_resolution_clock::now() - BASE_START_TIME).count(),valid ? "valid" : "invalid");
    delete[] visited;
}

void colcheck(int index,int thread_id){
    
    int* visited = new int[N+1]; 
    for(int i=0;i<=N;i++)visited[i]=0;  
    bool valid=true; 
    
    for(int row=0;row<N;row++){
        if(visited[sudoku[row][index]]==1){
            valid = false;
            Sudoku_Is_Valid = false;                //making global variable false
            break;                                  //if found invalid no need to check further
        }
        visited[sudoku[row][index]]++;
    }
    printf("Thread %d completes checking of column %d at time %lld microseconds and finds it as %s\n",thread_id, index+1,duration_cast<microseconds>(high_resolution_clock::now() - BASE_START_TIME).count(),valid ? "valid" : "invalid");
    delete[] visited;
}

void subgridcheck(int index,int thread_id){
    
    int* visited = new int[N+1]; 
    for(int i=0;i<=N;i++)visited[i]=0;   
    bool valid=true;
    
    for(int row=0;row<n;row++){
        for(int col=0;col<n;col++){
            if(visited[sudoku[row+(index/n)*n][col+(index%n)*n]]==1){
                valid = false;
                Sudoku_Is_Valid = false;            //making global variable false
                break;                              //if found invalid no need to check further
            }
            visited[sudoku[row+(index/n)*n][col+(index%n)*n]]++;
        }
    }
    printf("Thread %d completes checking of subgrid %d at time %lld microseconds and finds it as %s\n",thread_id, index+1,duration_cast<microseconds>(high_resolution_clock::now() - BASE_START_TIME).count(),valid ? "valid" : "invalid");

    delete[] visited;
}


void* check_sudoku(void *arguement){
    int thread_id = *(int*)arguement;                  //typecasting from void* to int
    int zero_indexed_tid = thread_id-1;

    while((Sudoku_Is_Valid==true) && (C < (3*N))){                                  //Total number of tasks is 3N, so if C is greater than equal to 3N then all tasks are done   

        /* ENTRY SECTION */
        if(Sudoku_Is_Valid==false) break;    

        auto Request_time = high_resolution_clock::now();

        printf("Thread %d request to enter CS at time %lld microseconds\n",thread_id,duration_cast<microseconds>(Request_time - BASE_START_TIME).count());

        /*Lock to provide mutual exclusion and protect critial section*/
        lock_BCAS(zero_indexed_tid); 

        if(Sudoku_Is_Valid==false) {                                              // Another thread already found invalidity
            unlock_BCAS(zero_indexed_tid);                                                    // Release lock immediately
            break;  
        }

        /* CRITICAL SECTION */

        auto CS_entry_time = high_resolution_clock::now();
        Total_CS_entry++;  //incrementing total number of times all threads entered CS

        printf("Thread %d enters CS at time %lld microseconds\n",thread_id, duration_cast<microseconds>(CS_entry_time - BASE_START_TIME).count());


        if(C >= 3*N){                               //Edge Case :if all tasks are done then and some thread entered inside Waiting Section , then just Give them entry in CS and exit them without any validation check
            
            // Total_CS_entry++;  //incrementing total number of times all threads entered CS
            Total_Time_To_Enter_CS += duration_cast<microseconds>(CS_entry_time - Request_time).count();  //total time taken by all threads to enter CS
            Worst_CS_Entry_time = max(Worst_CS_Entry_time,(double)duration_cast<microseconds>(CS_entry_time - Request_time).count());  //worst time taken by any thread to enter CS

            auto CS_exit_time = high_resolution_clock::now();
            Total_Time_To_Exit_CS += duration_cast<microseconds>(CS_exit_time - CS_entry_time).count();  //total time taken by all threads to execute CS
            Worst_CS_Exit_time = max(Worst_CS_Exit_time,(double)duration_cast<microseconds>(CS_exit_time - CS_entry_time).count());  //worst time taken by any thread to execute CS

            printf("Thread %d leaves CS at time %lld microseconds\n",thread_id, duration_cast<microseconds>(CS_exit_time - BASE_START_TIME).count());

            unlock_BCAS(zero_indexed_tid);                            //Deadlock Prevention : Directly breaking Edge case thread acquire the lock and thus other thread waiting in WAITING SECTION got stuck, so unlock it before exiting
            break;                                   //if all tasks are done then break
        }


        // Total_CS_entry++;  //incrementing total number of times all threads entered CS
        Total_Time_To_Enter_CS += duration_cast<microseconds>(CS_entry_time - Request_time).count();  //total time taken by all threads to enter CS
        Worst_CS_Entry_time = max(Worst_CS_Entry_time,(double)duration_cast<microseconds>(CS_entry_time - Request_time).count());  //worst time taken by any thread to enter CS

        C_value = C.fetch_add(taskInc);                                 //fetching the value of C , And then increment it by taskInc  , can be done by inbuilt fetch_add also. EX- C_value = C.fetch_add(taskInc);  //fetching and adding taskInc to C
        // C.fetch_add(taskInc);
        // C+=taskInc;

        auto C_Increment_time = high_resolution_clock::now();

        printf("Thread %d increments C= %d - %d at time %lld microseconds\n",thread_id, C_value, min (C_value + taskInc, 3 * N), duration_cast<microseconds>(C_Increment_time - BASE_START_TIME).count());


        auto CS_exit_time = high_resolution_clock::now();
        Total_Time_To_Exit_CS += duration_cast<microseconds>(CS_exit_time - CS_entry_time).count();  //total time taken by all threads to execute CS
        Worst_CS_Exit_time = max(Worst_CS_Exit_time,(double)duration_cast<microseconds>(CS_exit_time - CS_entry_time).count());  //worst time taken by any thread to execute CS

        printf("Thread %d leaves CS at time %lld microseconds\n",thread_id, duration_cast<microseconds>(CS_exit_time - BASE_START_TIME).count());


        unlock_BCAS(zero_indexed_tid);                                     //Signalling to unlock CS so that other thread can enter
        
        /* REMAINDER SECTION */

        int start = C_value;                        //start: store start oF C_value
        int end =  min (C_value + taskInc, 3*N);    //end: store end of C_value , but Edge case : Final C_value should be less than equal to 3*N                  //vector to store all tasks
        
        for(int i=start;i<end;i++){
            if(!Sudoku_Is_Valid) break;
            
            int taskType = i/N;                    // 0: Row, 1: Column, 2: Subgrid
            int index = i%N;                       //index of row, column or subgrid (0 to N-1)

            if(taskType == 0){
                printf("Thread %d grabs row %d at time %lld microseconds\n",thread_id, index+1, duration_cast<microseconds>(high_resolution_clock::now() - BASE_START_TIME).count());  //cout<<"Thread " <<thread_id<<" grabs row " <<index<<" at time "<<duration_cast<microseconds>(Task_Allotment-BASE_START_TIME).count()<<" microseconds\n";
                rowcheck(index,thread_id);
            }
            else if(taskType == 1){
                printf("Thread %d grabs column %d at time %lld microseconds\n",thread_id, index+1, duration_cast<microseconds>(high_resolution_clock::now() - BASE_START_TIME).count());  //cout<<"Thread " <<thread_id<<" grabs column " <<index<<" at time "<<duration_cast<microseconds>(Task_Allotment-BASE_START_TIME).count()<<" microseconds\n";
                colcheck(index,thread_id);
            }
            else { 
                printf("Thread %d grabs subgrid %d at time %lld microseconds\n",thread_id, index+1, duration_cast<microseconds>(high_resolution_clock::now() - BASE_START_TIME).count());  //cout<<"Thread " <<thread_id<<" grabs subgrid " <<index<<" at time "<<duration_cast<microseconds>(Task_Allotment-BASE_START_TIME).count()<<" microseconds\n";
                subgridcheck(index,thread_id);
            }
            
        }
    }
    pthread_exit(NULL);
}

int main() {
    freopen("output.txt", "w", stdout);
    ifstream read_object("inp.txt");
    if (!read_object) {
        cerr << "Error opening input.txt\n";
        return 1;
    }

    read_object >> K >> N >> taskInc;
    n = sqrt(N);

    if (N != n * n) {
        cerr << "N is not a perfect square\n";
        return 0;
    }

    waiting.resize(K, false); // Fixing out-of-bounds error

    sudoku = new int*[N];
    for (int i = 0; i < N; i++) sudoku[i] = new int[N];

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            read_object >> sudoku[i][j];

    read_object.close();

    pthread_t threads[K];
    int thread_ids[K];

    BASE_START_TIME = high_resolution_clock::now();

    for (int i = 0; i < K; i++) {
        thread_ids[i] = i+1;
        pthread_create(&threads[i], NULL, check_sudoku, (void*)&thread_ids[i]);
    }

    for (int i = 0; i < K; i++)
        pthread_join(threads[i], NULL);

    auto total_time = duration_cast<microseconds>(high_resolution_clock::now() - BASE_START_TIME);

    printf("Sudoku is %s\n", Sudoku_Is_Valid ? "valid" : "invalid");
    printf("Total execution time: %f microseconds\n", (double)total_time.count());
    printf("Average time taken by a thread to enter the CS is %f microseconds\n", Total_Time_To_Enter_CS / Total_CS_entry);
    printf("Average time taken by a thread to exit the CS is %f microseconds\n", Total_Time_To_Exit_CS / Total_CS_entry);
    printf("Worst-case time taken by a thread to enter the CS is %f microseconds\n", Worst_CS_Entry_time);
    printf("Worst-case time taken by a thread to exit the CS is %f microseconds\n", Worst_CS_Exit_time);

    fclose(stdout);

    // FILE *writeobject;
    // writeobject = fopen("output1.txt", "a");

    // fprintf(writeobject,"Total time taken is: %f microseconds\n", (double)total_time.count());
    // fprintf(writeobject,"Average time to enter CS: %f microseconds\n", Total_Time_To_Enter_CS/Total_CS_entry);
    // fprintf(writeobject,"Average time to exit CS: %f microseconds\n", Total_Time_To_Exit_CS/Total_CS_entry);
    // fprintf(writeobject,"Worst time to enter CS: %f microseconds\n", Worst_CS_Entry_time);
    // fprintf(writeobject,"Worst time to exit CS: %f microseconds\n", Worst_CS_Exit_time);
    // fclose(writeobject);

    for (int i = 0; i < N; i++) delete[] sudoku[i];
    delete[] sudoku;

    return 0;
}


/*
FILE *writeobject;
    writeobject = fopen("output1.txt", "a");

    fprintf(writeobject, "Total time taken is: %f microseconds\n", (double)total_time.count());
    fprintf(writeobject,"Average time to enter CS: %f microseconds\n", Total_Time_To_Enter_CS/Total_CS_entry);
    fprintf(writeobject,"Average time to exit CS: %f microseconds\n", Total_Time_To_Exit_CS/Total_CS_entry);
    fprintf(writeobject,"Worst time to enter CS: %f microseconds\n", Worst_CS_Entry_time);
    fprintf(writeobject,"Worst time to exit CS: %f microseconds\n", Worst_CS_Exit_time);
    fclose(writeobject);

*/

