#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
#include <string.h>

//輸入的數字量的大小 
#define INPUTMAX 100000

int inputData[INPUTMAX] = {0}; //儲存輸入的數字 
int globalHistogram[10] = {0}; //儲存輸出的直方圖長度 
int threadCount = 1; //thread的數量 
int flag = 0; //BusyWaiting管理critical section的變數 
pthread_mutex_t mutex; //Mutex管理critical section的變數  
sem_t* semaphores; //Semaphore管理critical section的變數  


//這是使用 BusyWaiting 的 Thread Function 
void *BusyWaitingThread(void* input){
	int* getData = (int*) input; //取得傳入的資料 
	int threadID = getData[0]; //thread的ID 
	int dataSize = getData[1]; //thread要處理的資料長度 
	int dataHead = getData[2]; //thread要處理的資料開頭位置 
	int localHistogram[10] = {0}; //儲存local的直方圖長度 
	int currentData = 0; //當前處裡的資料 
	int section = 0; //處裡的資料屬於哪一個區域 
	//印出 thread 開始執行 
	printf("Thread[%d] Strat. Size => %d\n", threadID, dataSize);	
	
	//將被分配給這thread的資料統計到local的直方圖 
	int dataPtr = 0;
	for(dataPtr = 0; dataPtr < dataSize; dataPtr++){
		currentData = inputData[dataHead + dataPtr];		
		section = currentData / 10;
		localHistogram[section] += 1;
	}
	
	//請求全域變數的存取權限 
	while(flag != threadID);
	//===============critical section=====================	
	for(dataPtr = 0; dataPtr < 10; dataPtr++){
		globalHistogram[dataPtr] += localHistogram[dataPtr];
	} 
	//====================================================
	flag = (flag+1) % threadCount;
	
	return NULL;
}

//這是使用 Mutex 的 Thread Function
void *MutexesThread(void* input){
	int* getData = (int*) input; //取得傳入的資料 
	int threadID = getData[0]; //thread的ID 
	int dataSize = getData[1]; //thread要處理的資料長度 
	int dataHead = getData[2]; //thread要處理的資料開頭位置 
	int localHistogram[10] = {0}; //儲存local的直方圖長度 
	int currentData = 0; //當前處裡的資料 
	int section = 0; //處裡的資料屬於哪一個區域 
	//印出 thread 開始執行 
	printf("Thread[%d] Strat. Size => %d\n", threadID, dataSize);	
	
	//將被分配給這thread的資料統計到local的直方圖 
	int dataPtr = 0;
	for(dataPtr = 0; dataPtr < dataSize; dataPtr++){
		currentData = inputData[dataHead + dataPtr];		
		section = currentData / 10;
		localHistogram[section] += 1;
	}
	
	//請求全域變數的存取權限 
	pthread_mutex_lock(&mutex);
	//===============critical section=====================
	for(dataPtr = 0; dataPtr < 10; dataPtr++){
		globalHistogram[dataPtr] += localHistogram[dataPtr];
	} 
	//====================================================
	pthread_mutex_unlock(&mutex);
	
	return NULL;
}

//這是使用 Semaphore 的 Thread Function
void *SemaphoreThread(void* input){
	int* getData = (int*) input; //取得傳入的資料 
	int threadID = getData[0]; //thread的ID 
	int dataSize = getData[1]; //thread要處理的資料長度 
	int dataHead = getData[2]; //thread要處理的資料開頭位置 
	int localHistogram[10] = {0}; //儲存local的直方圖長度 
	int currentData = 0; //當前處裡的資料 
	int section = 0; //處裡的資料屬於哪一個區域 
	//印出 thread 開始執行 
	printf("Thread[%d] Strat. Size => %d\n", threadID, dataSize);	
	
	//將被分配給這thread的資料統計到local的直方圖 
	int dataPtr = 0;
	for(dataPtr = 0; dataPtr < dataSize; dataPtr++){
		currentData = inputData[dataHead + dataPtr];		
		section = currentData / 10;
		localHistogram[section] += 1;
	}
	
	//請求全域變數的存取權限 
	sem_wait(&semaphores[threadID]);
	//===============critical section=====================
	for(dataPtr = 0; dataPtr < 10; dataPtr++){
		globalHistogram[dataPtr] += localHistogram[dataPtr];
	} 
	//====================================================
	sem_post(&semaphores[(threadID+1) % threadCount]);
	
	return NULL;
}


int main(int argc, char *argv[]) {	
	int correctAns[10] = {0}; //這裡紀錄正確答案 
	pthread_t* thread_handles; //Thread控制 
	int thread, numIndex, histogramIndex; //for迴圈的變數名 
	
	printf("Use threads: "); //決定要使用幾個thread 
	scanf("%d", &threadCount);  
	thread_handles = malloc(threadCount * sizeof(pthread_t)); //配置thread記憶體空間 
	
	semaphores = malloc(threadCount * sizeof(sem_t)); //配置semaphores記憶體空間 
	
	//初始化semaphores參數，只讓一個為1其他為0 
	sem_init(&semaphores[0], 0, 1);
	for (thread = 1; thread < threadCount; thread++)
        sem_init(&semaphores[thread], 0, 0);
	
	//亂數生成 
	unsigned seed;
    seed = (unsigned)time(NULL); 
    srand(seed);
    
    for(numIndex = 0; numIndex < INPUTMAX; numIndex++){ 
        inputData[numIndex] = rand()%100;
        correctAns[inputData[numIndex] / 10]++; //統計正確解答 
    } 
       
    int averageData = INPUTMAX / threadCount; //計算每個thread的工作量 
    
    int sendDataStart = 0, sendDataEnd = -1; //每個thread取資料開始與結束範圍 
    
    for(thread = 0; thread < threadCount; thread++){
    	int* sendData; //要傳送給thread的訊息 
    	int pastDataCount = 0; //thread要工作的資料數量 
    	
    	//設定thread取資料開始與結束的範圍 
    	sendDataStart = sendDataEnd + 1;
		if(thread + 1 == threadCount){
    		sendDataEnd = INPUTMAX - 1;	
		}
		else{
			sendDataEnd = sendDataEnd + averageData;
		}
		
		pastDataCount = (sendDataEnd - sendDataStart + 1);
    	
    	//配置sendData記憶體，並放入threadID、工作量及工作資料的開始位置 
		sendData = malloc(3 * sizeof(int)); 	
    	sendData[0] = thread;
    	sendData[1] = pastDataCount;
    	sendData[2] = sendDataStart;
    	
    	//建立以 BusyWaiting來運作的thread
		//pthread_create(&thread_handles[thread], NULL, BusyWaitingThread, (void*) &sendData[0]);
		//建立以 Mutex來運作的thread 
		pthread_create(&thread_handles[thread], NULL, MutexesThread, (void*) &sendData[0]); 
		//建立以 Semaphore來運作的thread 
		//pthread_create(&thread_handles[thread], NULL, SemaphoreThread, (void*) &sendData[0]);
	}
	
	//等待所有thread執行結束 
	for(thread = 0;  thread < threadCount; thread++){
		pthread_join(thread_handles[thread], NULL);
	}
	
	//對答案，看thread跑出來是不是正確的 
	printf("\n\n");
	for(histogramIndex = 0; histogramIndex < 10; histogramIndex++){
		printf("CORRECT  ANS: %d => %d\n", histogramIndex, correctAns[histogramIndex]);
		printf("MY OUTPUTANS: %d => %d\n", histogramIndex, globalHistogram[histogramIndex]);
		printf("%s\n", correctAns[histogramIndex] == globalHistogram[histogramIndex] ? "correct" : "error");
	}
	
	//釋放記憶體 
	free(thread_handles);
	free(semaphores);
	
	return 0;
}
