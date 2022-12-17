// #include <iostream>
// #include <stdio.h>
// #include <omp.h>
// #include <queue>
// #include <cmath>
// #include <fstream>


// using namespace std;

// #define NUM_THREADS 32
// #pragma warning(disable:4996)

// int d = 151;
// int q = 997;

// omp_lock_t lock;

// typedef struct {
// 	char* content;
// 	int length;
// } Text;

// typedef struct {
// 	int patternHash;
// 	int patternLength;
// 	int startIndex;
// 	int textLength;
// } SearchPart;


// struct Comma final : std::numpunct<char>
// {
// 	char do_decimal_point() const override { return ','; }
// };


// Text readFileToText(char* name);
// char* generateString(int len);
// int hashText(char* text, int len);
// void printQueue(std::queue<int> queue);
// char* charSubstr(char* text, int offset, int count);
// std::queue<int> rabinKarpOMP(char* chain, char* pattern, int chain_len, int pattern_len);


// int main(int argc, char** argv) {
// 	omp_set_num_threads(NUM_THREADS);
// 	omp_init_lock(&lock);
// 	srand(time(NULL));

//     //std::queue<int> result;

// 	int rank, size, r;
// 	int src, dst, tag, i;
// 	MPI_Status status;
// 	MPI_Datatype searchpart_type;
// 	MPI_Datatype types[4] = { MPI_INT, MPI_INT, MPI_INT, MPI_INT };
// 	int blockLengths[4] = { 1, 1, 1, 1 };
// 	MPI_Aint displacements[4];
// 	MPI_Aint base, addr;
// 	SearchPart dataToSend, receivedData;

// 	double start = 0, end = 0;
// 	MPI_Init(&argc, &argv);
// 	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
// 	MPI_Comm_size(MPI_COMM_WORLD, &size);

// 	MPI_Get_address(&dataToSend, &base);
// 	MPI_Get_address(&(dataToSend.patternHash), &addr);
// 	displacements[0] = addr - base;
// 	MPI_Get_address(&(dataToSend.patternLength), &addr);
// 	displacements[1] = addr - base;
// 	MPI_Get_address(&(dataToSend.startIndex), &addr);
// 	displacements[2] = addr - base;
// 	MPI_Get_address(&(dataToSend.textLength), &addr);
// 	displacements[3] = addr - base;

// 	MPI_Type_create_struct(4, blockLengths, displacements, types, &searchpart_type);
// 	MPI_Type_commit(&searchpart_type);

// 	int tSize = 2000000;
// 	int basicInfoTag = 1;
// 	int textTag = 2;
// 	int patternTag = 3;
// 	int resultSizeTag = 4;
// 	int resultTag = 5;


// 	if (rank == 0) {


//         int chain_len, pattern_len;

//         // std::cin >> chain_len;
//         // std::cin >> pattern_len;

//         // char* chain = generateString(chain_len);
//         // char* pattern = generateString(pattern_len);

//         //	//for tests
//         chain_len = 445;
//         pattern_len = 2;
    
//         char* chain = (char*)"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
//         char* pattern = (char*)"in";

// 		printf("Original text:\n");
// 		printf(chain);
// 		fflush(stdout);
        
// 		printf("Pattern to find:\n");
// 		printf(pattern);
//         fflush(stdout);

//         std::queue<int> resBasic, resOMP, resMPI, resOMP_MPI;
//         double start, end;	

//         start = omp_get_wtime();

//         size_t pattern_hash = hashText(pattern, pattern_len);
// 	    size_t chain_hash;
// 	    const int loops_amount = chain_len - pattern_len;
        
// 		int numberOfPortions = size - 1;
// 		int portionSize = chain_len / numberOfPortions;
// 		int lastPortionSize = chain_len - ((numberOfPortions - 1) * portionSize);


// 		int portionLength = portionSize + (pattern_len - 1);
// 		int lastPortionLength = lastPortionSize;

// 		for (size_t i = 1; i <= numberOfPortions; i++) {
			
// 			// Send basic info in struct
// 			dataToSend.textLength = portionLength + 1;
// 			dataToSend.startIndex = (i - 1) * portionSize;
// 			dataToSend.patternLength = pattern_len + 1;
// 			dataToSend.patternHash = pattern_hash;
			
// 			if (i == size - 1) {
// 				dataToSend.textLength = lastPortionLength + 1;
// 			}

// 			MPI_Send(&dataToSend, 1, searchpart_type, i, basicInfoTag, MPI_COMM_WORLD);
// 		}

// 		MPI_Bcast(pattern, dataToSend.patternLength, MPI_CHAR, 0, MPI_COMM_WORLD);

// 		for (size_t i = 1; i <= numberOfPortions; i++) {

// 			// Send text portion
// 			if (i == size - 1) {
// 				printf("\nText to send for thread %d -> \t%s", i, charSubstr(chain, ((i - 1) * portionSize), lastPortionLength));
// 				fflush(stdout);
// 				MPI_Send(charSubstr(chain, ((i - 1) * portionSize), lastPortionLength), lastPortionLength + 1, MPI_CHAR, i, textTag, MPI_COMM_WORLD);
// 			}
// 			else {
// 				printf("\nText to send for thread %d -> \t%s", i, charSubstr(chain, ((i - 1) * portionSize), portionLength));
// 				fflush(stdout);
// 				MPI_Send(charSubstr(chain, ((i - 1) * portionSize), portionLength), portionLength + 1, MPI_CHAR, i, textTag, MPI_COMM_WORLD);
// 			}
			
// 		}

// 		std::queue<int> grouppedResults;
// 		for (size_t i = 1; i <= numberOfPortions; i++) {
			
// 			// Receive results
// 			int resultSize;
// 			MPI_Recv(&resultSize, 1, MPI_INT, i, resultSizeTag, MPI_COMM_WORLD, &status);
// 			printf("\nReceived %d results from thread %d", resultSize, status.MPI_SOURCE);
// 			fflush(stdout);

// 			if (resultSize > 0) {
// 				int* resultArray = new int[resultSize];
// 				MPI_Recv(resultArray, resultSize, MPI_INT, i, resultTag, MPI_COMM_WORLD, &status);

// 				for (size_t j = 0; j < resultSize; j++)
// 				{
// 					grouppedResults.push(resultArray[j]);
// 				}
// 			}
// 		}

//         end = omp_get_wtime();
//         std::cout << "\nExecution time for algorithm with MPI parallelization: " << (end - start) << ".\n";
//         printQueue(grouppedResults);
// 	}
// 	else {
// 		MPI_Recv(&receivedData, 1, searchpart_type, 0, basicInfoTag, MPI_COMM_WORLD, &status);
		
// 		char* text = new char[receivedData.textLength];
// 		char* pattern = new char[receivedData.patternLength];

// 		MPI_Bcast(pattern, receivedData.patternLength, MPI_CHAR, 0, MPI_COMM_WORLD);
// 		MPI_Recv(text, receivedData.textLength, MPI_CHAR, 0, textTag, MPI_COMM_WORLD, &status);

// 		printf("\nProces %d: Text Length: %d\tPattern Length: %d\t Start Index: %d", rank, receivedData.textLength, receivedData.patternLength, receivedData.startIndex);
// 		fflush(stdout);
// 		printf("\nProces %d received text:\t%s", rank, text);
// 		fflush(stdout);
// 		printf("\nProces %d received pattern:\t%s", rank, pattern);
// 		fflush(stdout);

// 		std::queue<int> result;
// 		result = rabinKarpOMP(text, pattern, receivedData.textLength-1, receivedData.patternLength-1);

// 		int resultSize = (int)result.size();
// 		if (resultSize > 0) {
// 			int* resultArray = new int[resultSize];
// 			for (size_t i = 0; i < resultSize; i++)
// 			{
// 				resultArray[i] = result.front() + receivedData.startIndex;
// 				result.pop();
// 			}
// 			MPI_Send(&resultSize, 1, MPI_INT, 0, resultSizeTag, MPI_COMM_WORLD);
// 			MPI_Send(resultArray, resultSize, MPI_INT, 0, resultTag, MPI_COMM_WORLD);
// 		}
// 		else {
// 			resultSize = 0;
// 			MPI_Send(&resultSize, 1, MPI_INT, 0, resultSizeTag, MPI_COMM_WORLD);
// 		}

// 	}

// 	MPI_Finalize();

// 	//release dynamic allocated mem
// 	return 0;
// }

// char* generateString(int len) {
// 	char* text = new char[len+1];

// 	for (int i = 0; i < len; i++) {
// 		text[i] = (char)((rand() % ('z' - ' ')) + ' ');
// 	}
// 	text[len] = '\0';
// 	return text;
// }

// void printQueue(std::queue<int> queue) {
// 	while(!queue.empty()) {
// 		std::cout << queue.front() << " ";
// 		queue.pop();
// 	}
// }

// char* charSubstr(char* text, int offset, int count) {
// 	char* substring = new char[count+1];
// 	strncpy(substring, &text[offset], count);
// 	substring[count] = '\0';
// 	return substring;
// }

// int hashText(char* text, int len) {
// 	int h = 1;
// 	for (int i = 0; i < len; i++) {
// 		h += int(text[i]) * pow(151, len - i - 1);
// 	}
// 	return h;
// }

// std::queue<int> rabinKarpOMP(char* chain, char* pattern, int chain_len, int pattern_len) {
// 	std::queue<int> result;
// 	size_t pattern_hash = hashText(pattern, pattern_len);
// 	const int loops_amount = chain_len - pattern_len + 1;

// #pragma omp parallel for schedule(static) shared(result) firstprivate(pattern_hash)
// 	for (int i = 0; i < loops_amount; i++) {

// 		size_t chain_hash = 1;
// #pragma omp parallel for schedule(static) // reduction (+ : chain_hash) 
// 		for (int j = 0; j < pattern_len; j++) {
// 			chain_hash += int(chain[i + j]) * pow(151, pattern_len - j - 1); //pomyœleæ nad sta³¹; aktualnie = 151
// 		}

// 		if (chain_hash == pattern_hash) {
// 			for (int j = 0; j < pattern_len; j++) {
// 				if (pattern[j] != chain[i + j]) break;
// 				else if (j == pattern_len - 1) {
// 					omp_set_lock(&lock);
// 					result.push(i);
// 					omp_unset_lock(&lock);
// 				}
// 			}
// 		}
// 	}

// 	return result;
// }
