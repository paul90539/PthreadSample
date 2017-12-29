#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
#include <string.h>

//��J���Ʀr�q���j�p 
#define INPUTMAX 100000

int inputData[INPUTMAX] = {0}; //�x�s��J���Ʀr 
int globalHistogram[10] = {0}; //�x�s��X������Ϫ��� 
int threadCount = 1; //thread���ƶq 
int flag = 0; //BusyWaiting�޲zcritical section���ܼ� 
pthread_mutex_t mutex; //Mutex�޲zcritical section���ܼ�  
sem_t* semaphores; //Semaphore�޲zcritical section���ܼ�  


//�o�O�ϥ� BusyWaiting �� Thread Function 
void *BusyWaitingThread(void* input){
	int* getData = (int*) input; //���o�ǤJ����� 
	int threadID = getData[0]; //thread��ID 
	int dataSize = getData[1]; //thread�n�B�z����ƪ��� 
	int dataHead = getData[2]; //thread�n�B�z����ƶ}�Y��m 
	int localHistogram[10] = {0}; //�x�slocal������Ϫ��� 
	int currentData = 0; //��e�B�̪���� 
	int section = 0; //�B�̪�����ݩ���@�Ӱϰ� 
	//�L�X thread �}�l���� 
	printf("Thread[%d] Strat. Size => %d\n", threadID, dataSize);	
	
	//�N�Q���t���othread����Ʋέp��local������� 
	int dataPtr = 0;
	for(dataPtr = 0; dataPtr < dataSize; dataPtr++){
		currentData = inputData[dataHead + dataPtr];		
		section = currentData / 10;
		localHistogram[section] += 1;
	}
	
	//�ШD�����ܼƪ��s���v�� 
	while(flag != threadID);
	//===============critical section=====================	
	for(dataPtr = 0; dataPtr < 10; dataPtr++){
		globalHistogram[dataPtr] += localHistogram[dataPtr];
	} 
	//====================================================
	flag = (flag+1) % threadCount;
	
	return NULL;
}

//�o�O�ϥ� Mutex �� Thread Function
void *MutexesThread(void* input){
	int* getData = (int*) input; //���o�ǤJ����� 
	int threadID = getData[0]; //thread��ID 
	int dataSize = getData[1]; //thread�n�B�z����ƪ��� 
	int dataHead = getData[2]; //thread�n�B�z����ƶ}�Y��m 
	int localHistogram[10] = {0}; //�x�slocal������Ϫ��� 
	int currentData = 0; //��e�B�̪���� 
	int section = 0; //�B�̪�����ݩ���@�Ӱϰ� 
	//�L�X thread �}�l���� 
	printf("Thread[%d] Strat. Size => %d\n", threadID, dataSize);	
	
	//�N�Q���t���othread����Ʋέp��local������� 
	int dataPtr = 0;
	for(dataPtr = 0; dataPtr < dataSize; dataPtr++){
		currentData = inputData[dataHead + dataPtr];		
		section = currentData / 10;
		localHistogram[section] += 1;
	}
	
	//�ШD�����ܼƪ��s���v�� 
	pthread_mutex_lock(&mutex);
	//===============critical section=====================
	for(dataPtr = 0; dataPtr < 10; dataPtr++){
		globalHistogram[dataPtr] += localHistogram[dataPtr];
	} 
	//====================================================
	pthread_mutex_unlock(&mutex);
	
	return NULL;
}

//�o�O�ϥ� Semaphore �� Thread Function
void *SemaphoreThread(void* input){
	int* getData = (int*) input; //���o�ǤJ����� 
	int threadID = getData[0]; //thread��ID 
	int dataSize = getData[1]; //thread�n�B�z����ƪ��� 
	int dataHead = getData[2]; //thread�n�B�z����ƶ}�Y��m 
	int localHistogram[10] = {0}; //�x�slocal������Ϫ��� 
	int currentData = 0; //��e�B�̪���� 
	int section = 0; //�B�̪�����ݩ���@�Ӱϰ� 
	//�L�X thread �}�l���� 
	printf("Thread[%d] Strat. Size => %d\n", threadID, dataSize);	
	
	//�N�Q���t���othread����Ʋέp��local������� 
	int dataPtr = 0;
	for(dataPtr = 0; dataPtr < dataSize; dataPtr++){
		currentData = inputData[dataHead + dataPtr];		
		section = currentData / 10;
		localHistogram[section] += 1;
	}
	
	//�ШD�����ܼƪ��s���v�� 
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
	int correctAns[10] = {0}; //�o�̬������T���� 
	pthread_t* thread_handles; //Thread���� 
	int thread, numIndex, histogramIndex; //for�j�骺�ܼƦW 
	
	printf("Use threads: "); //�M�w�n�ϥδX��thread 
	scanf("%d", &threadCount);  
	thread_handles = malloc(threadCount * sizeof(pthread_t)); //�t�mthread�O����Ŷ� 
	
	semaphores = malloc(threadCount * sizeof(sem_t)); //�t�msemaphores�O����Ŷ� 
	
	//��l��semaphores�ѼơA�u���@�Ӭ�1��L��0 
	sem_init(&semaphores[0], 0, 1);
	for (thread = 1; thread < threadCount; thread++)
        sem_init(&semaphores[thread], 0, 0);
	
	//�üƥͦ� 
	unsigned seed;
    seed = (unsigned)time(NULL); 
    srand(seed);
    
    for(numIndex = 0; numIndex < INPUTMAX; numIndex++){ 
        inputData[numIndex] = rand()%100;
        correctAns[inputData[numIndex] / 10]++; //�έp���T�ѵ� 
    } 
       
    int averageData = INPUTMAX / threadCount; //�p��C��thread���u�@�q 
    
    int sendDataStart = 0, sendDataEnd = -1; //�C��thread����ƶ}�l�P�����d�� 
    
    for(thread = 0; thread < threadCount; thread++){
    	int* sendData; //�n�ǰe��thread���T�� 
    	int pastDataCount = 0; //thread�n�u�@����Ƽƶq 
    	
    	//�]�wthread����ƶ}�l�P�������d�� 
    	sendDataStart = sendDataEnd + 1;
		if(thread + 1 == threadCount){
    		sendDataEnd = INPUTMAX - 1;	
		}
		else{
			sendDataEnd = sendDataEnd + averageData;
		}
		
		pastDataCount = (sendDataEnd - sendDataStart + 1);
    	
    	//�t�msendData�O����A�é�JthreadID�B�u�@�q�Τu�@��ƪ��}�l��m 
		sendData = malloc(3 * sizeof(int)); 	
    	sendData[0] = thread;
    	sendData[1] = pastDataCount;
    	sendData[2] = sendDataStart;
    	
    	//�إߥH BusyWaiting�ӹB�@��thread
		//pthread_create(&thread_handles[thread], NULL, BusyWaitingThread, (void*) &sendData[0]);
		//�إߥH Mutex�ӹB�@��thread 
		pthread_create(&thread_handles[thread], NULL, MutexesThread, (void*) &sendData[0]); 
		//�إߥH Semaphore�ӹB�@��thread 
		//pthread_create(&thread_handles[thread], NULL, SemaphoreThread, (void*) &sendData[0]);
	}
	
	//���ݩҦ�thread���浲�� 
	for(thread = 0;  thread < threadCount; thread++){
		pthread_join(thread_handles[thread], NULL);
	}
	
	//�ﵪ�סA��thread�]�X�ӬO���O���T�� 
	printf("\n\n");
	for(histogramIndex = 0; histogramIndex < 10; histogramIndex++){
		printf("CORRECT  ANS: %d => %d\n", histogramIndex, correctAns[histogramIndex]);
		printf("MY OUTPUTANS: %d => %d\n", histogramIndex, globalHistogram[histogramIndex]);
		printf("%s\n", correctAns[histogramIndex] == globalHistogram[histogramIndex] ? "correct" : "error");
	}
	
	//����O���� 
	free(thread_handles);
	free(semaphores);
	
	return 0;
}
