#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define INPUTMAX 10

int globalHistogram[10] = {0};
int flag = 0;
int threadCount = 1; 
pthread_mutex_t mutex;
sem_t* semaphores;

void *BusyWaitingThread(void* input){
	int* getData = (int*) input;
	int threadID = getData[0];
	int dataSize= getData[1];
	int localHistogram[10] = {0};
		
	printf("TH%d Strat. Size => %d\n", threadID, dataSize);	
	
	int k = 0;
	for(k = 0; k < dataSize; k++){
		int currentData = getData[2 + k];		
		int section = currentData / 10;
		localHistogram[section] += 1; 
	}
	
	while(flag != threadID);
	//===============critical section=====================	
	for(k = 0; k < 10; k++){
		globalHistogram[k] += localHistogram[k];
	} 
	//====================================================
	flag = (flag+1) % threadCount;
	
	return NULL;
}

void *MutexesThread(void* input){
	int* getData = (int*) input;
	int threadID = getData[0];
	int dataSize= getData[1];
	int localHistogram[10] = {0};
		
	printf("TH%d Strat. \nSize => %d\n", threadID, dataSize);	
	
	int k = 0;
	for(k = 0; k < dataSize; k++){
		int currentData = getData[2 + k];		
		int section = currentData / 10;
		localHistogram[section] += 1; 
	}
	
	pthread_mutex_lock(&mutex);
	//===============critical section=====================
	for(k = 0; k < 10; k++){
		globalHistogram[k] += localHistogram[k];
	} 
	//====================================================
	pthread_mutex_unlock(&mutex);
	
	return NULL;
}

void *SemaphoreThread(void* input){
	int* getData = (int*) input;
	int threadID = getData[0];
	int dataSize= getData[1];
	int localHistogram[10] = {0};
		
	printf("TH%d Strat. \nSize => %d\n", threadID, dataSize);	
	
	int k = 0;
	for(k = 0; k < dataSize; k++){
		int currentData = getData[2 + k];		
		int section = currentData / 10;
		localHistogram[section] += 1; 
	}
	
	sem_wait(&semaphores[threadID]);
	//===============critical section=====================
	for(k = 0; k < 10; k++){
		globalHistogram[k] += localHistogram[k];
	} 
	//====================================================
	sem_post(&semaphores[(threadID+1) % threadCount]);
	
	return NULL;
}


int main(int argc, char *argv[]) {
	
	int inputData[INPUTMAX] = {0};
	int correctAns[10] = {0};
	pthread_t* thread_handles;
	int i;
	
	printf("Use threads: ");
	scanf("%d", &threadCount);
	thread_handles = malloc(threadCount * sizeof(pthread_t));
	
	semaphores = malloc(threadCount * sizeof(sem_t));
	
	sem_init(&semaphores[0], 0, 1);
	for (i = 1; i < threadCount; i++)
        sem_init(&semaphores[i], 0, 0);
	
	unsigned seed;
    seed = (unsigned)time(NULL); 
    srand(seed);
    
    for(i=0; i<INPUTMAX; i++){ 
        inputData[i] = rand()%100;
        correctAns[inputData[i]/10]++;
        printf("%d\n", inputData[i]);
    } 
       
    int averageData = INPUTMAX / threadCount;
    
    int sendDataStart = 0, sendDataEnd = -1;
    
    for(i=0; i<threadCount; i++){
    	int* sendData;	
    	int pastDataCount = 0;
    	
    	sendDataStart = sendDataEnd + 1;
		if(i+1 == threadCount){
    		sendDataEnd = INPUTMAX - 1;	
		}
		else{
			sendDataEnd = sendDataEnd + averageData;
		}
		
		pastDataCount = (sendDataEnd - sendDataStart + 1);
    	sendData = malloc( (pastDataCount + 2) * sizeof(int));
    	
    	sendData[0] = i;
    	sendData[1] = pastDataCount;
    	
    	memcpy(&sendData[2], &inputData[sendDataStart], pastDataCount * sizeof(int));
    	
    	//printf("%d %d %d\n", i, sendDataStart, sendDataEnd);	
		//pthread_create(&thread_handles[i], NULL , BusyWaitingThread , (void*) &sendData[0]);
		//pthread_create(&thread_handles[i], NULL , MutexesThread , (void*) &sendData[0]);
		pthread_create(&thread_handles[i], NULL , SemaphoreThread , (void*) &sendData[0]);
	}
	
	for(i=0; i<threadCount; i++){
		pthread_join(thread_handles[i], NULL);
	}
	
	printf("\n\n");
	for(i = 0; i < 10; i++){
		printf("ANS: %d => %d\n", i, correctAns[i]);
		printf("OUT: %d => %d\n", i, globalHistogram[i]);
		printf("%s\n", correctAns[i] == globalHistogram[i] ? "correct" : "error");
	}
	
	free(semaphores);
	
	return 0;
}
