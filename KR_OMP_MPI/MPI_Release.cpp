#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <omp.h>
#include <queue>
#include <cmath>
#include <fstream>


using namespace std;

#define NUM_THREADS 32
#pragma warning(disable:4996)

int d = 151;
int q = 997;

omp_lock_t lock;

typedef struct {
	char* content;
	int length;
} Text;

typedef struct {
	int patternHash;
	int patternLength;
	int startIndex;
	int textLength;
} SearchPart;


struct Comma final : std::numpunct<char>
{
	char do_decimal_point() const override { return ','; }
};

ofstream file;

Text readFileToText(char* name);
char* generateString(int len);
int hashText(char* text, int len);
void printQueue(std::queue<int> queue);
char* charSubstr(char* text, int offset, int count);
std::queue<int> rabinKarpBasic(char* chain, char* pattern, int chain_len, int pattern_len);


int main(int argc, char** argv) {
	omp_set_num_threads(NUM_THREADS);
	omp_init_lock(&lock);
	//unsigned long jobSizes[] = { 1500000000,1000000000,500000000,250000000, 125000000, 31250000, 15625000, 7812500, 3906250, 1953125 };
	unsigned long jobSizes[] = { 10000000, 1000000, 100000, 10000, 1000, 100 };
	srand(time(NULL));

    //std::queue<int> result;
	file.imbue(std::locale(std::locale::classic(), new Comma));
	file.open("wyniki.csv");

	int rank, size, r;
	int src, dst, tag, i;
	MPI_Status status;
	MPI_Datatype searchpart_type;
	MPI_Datatype types[4] = { MPI_INT, MPI_INT, MPI_INT, MPI_INT };
	int blockLengths[4] = { 1, 1, 1, 1 };
	MPI_Aint displacements[4];
	MPI_Aint base, addr;
	SearchPart dataToSend, receivedData;

	double start = 0, end = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Get_address(&dataToSend, &base);
	MPI_Get_address(&(dataToSend.patternHash), &addr);
	displacements[0] = addr - base;
	MPI_Get_address(&(dataToSend.patternLength), &addr);
	displacements[1] = addr - base;
	MPI_Get_address(&(dataToSend.startIndex), &addr);
	displacements[2] = addr - base;
	MPI_Get_address(&(dataToSend.textLength), &addr);
	displacements[3] = addr - base;

	MPI_Type_create_struct(4, blockLengths, displacements, types, &searchpart_type);
	MPI_Type_commit(&searchpart_type);

	int tSize = 2000000;
	int basicInfoTag = 1;
	int textTag = 2;
	int patternTag = 3;
	int resultSizeTag = 4;
	int resultTag = 5;

	file << "D³ugoœæ tekstu" << ";" << "Czas" << endl;

	for (size_t js = 0; js < 6; js++) 
	{
		if (rank == 0) {

			int chain_len = jobSizes[js]; 
			int pattern_len = 10;

			char* chain = generateString(chain_len);
			char* pattern = generateString(pattern_len);

			std::queue<int> resBasic, resOMP, resMPI, resOMP_MPI;
			double start, end;

			start = omp_get_wtime();

			size_t pattern_hash = hashText(pattern, pattern_len);
			size_t chain_hash;
			const int loops_amount = chain_len - pattern_len;

			int numberOfPortions = size - 1;
			int portionSize = chain_len / numberOfPortions;
			int lastPortionSize = chain_len - ((numberOfPortions - 1) * portionSize);


			int portionLength = portionSize + (pattern_len - 1);
			int lastPortionLength = lastPortionSize;

			for (size_t i = 1; i <= numberOfPortions; i++) {

				// Send basic info in struct
				dataToSend.textLength = portionLength + 1;
				dataToSend.startIndex = (i - 1) * portionSize;
				dataToSend.patternLength = pattern_len + 1;
				dataToSend.patternHash = pattern_hash;

				if (i == size - 1) {
					dataToSend.textLength = lastPortionLength + 1;
				}

				MPI_Send(&dataToSend, 1, searchpart_type, i, basicInfoTag, MPI_COMM_WORLD);
			}

			MPI_Bcast(pattern, dataToSend.patternLength, MPI_CHAR, 0, MPI_COMM_WORLD);

			for (size_t i = 1; i <= numberOfPortions; i++) {

				// Send text portion
				if (i == size - 1) {
					MPI_Send(charSubstr(chain, ((i - 1) * portionSize), lastPortionLength), lastPortionLength + 1, MPI_CHAR, i, textTag, MPI_COMM_WORLD);
				}
				else {
					MPI_Send(charSubstr(chain, ((i - 1) * portionSize), portionLength), portionLength + 1, MPI_CHAR, i, textTag, MPI_COMM_WORLD);
				}
			}

			delete chain;
			delete pattern;

			std::queue<int> grouppedResults;
			for (size_t i = 1; i <= numberOfPortions; i++) {

				// Receive results
				int resultSize;
				MPI_Recv(&resultSize, 1, MPI_INT, i, resultSizeTag, MPI_COMM_WORLD, &status);

				if (resultSize > 0) {
					int* resultArray = new int[resultSize];
					MPI_Recv(resultArray, resultSize, MPI_INT, i, resultTag, MPI_COMM_WORLD, &status);

					for (size_t j = 0; j < resultSize; j++)
					{
						grouppedResults.push(resultArray[j]);
					}
				}
			}

			end = omp_get_wtime();
			double time = end - start;
			printf("\nExecution time for text size %d and algorithm with MPI parallelization: %f", chain_len, time);
			fflush(stdout);
			printQueue(grouppedResults);
			file << chain_len << ";" << time << endl;
		}
		else {
			MPI_Recv(&receivedData, 1, searchpart_type, 0, basicInfoTag, MPI_COMM_WORLD, &status);

			char* text = new char[receivedData.textLength];
			char* pattern = new char[receivedData.patternLength];

			MPI_Bcast(pattern, receivedData.patternLength, MPI_CHAR, 0, MPI_COMM_WORLD);
			MPI_Recv(text, receivedData.textLength, MPI_CHAR, 0, textTag, MPI_COMM_WORLD, &status);

			std::queue<int> result;
			result = rabinKarpBasic(text, pattern, receivedData.textLength - 1, receivedData.patternLength - 1);

			delete text;
			delete pattern;

			int resultSize = (int)result.size();
			if (resultSize > 0) {
				int* resultArray = new int[resultSize];
				for (size_t i = 0; i < resultSize; i++)
				{
					resultArray[i] = result.front() + receivedData.startIndex;
					result.pop();
				}
				MPI_Send(&resultSize, 1, MPI_INT, 0, resultSizeTag, MPI_COMM_WORLD);
				MPI_Send(resultArray, resultSize, MPI_INT, 0, resultTag, MPI_COMM_WORLD);
				delete resultArray;
			}
			else {
				resultSize = 0;
				MPI_Send(&resultSize, 1, MPI_INT, 0, resultSizeTag, MPI_COMM_WORLD);
			}

			
		}
	}
	

	MPI_Finalize();

	return 0;
}

char* generateString(int len) {
	char* text = new char[len+1];

	for (int i = 0; i < len; i++) {
		text[i] = (char)((rand() % ('z' - ' ')) + ' ');
	}
	text[len] = '\0';
	return text;
}

void printQueue(std::queue<int> queue) {
	while(!queue.empty()) {
		std::cout << queue.front() << " ";
		queue.pop();
	}
}

char* charSubstr(char* text, int offset, int count) {
	char* substring = new char[count+1];
	strncpy(substring, &text[offset], count);
	substring[count] = '\0';
	return substring;
}

int hashText(char* text, int len) {
	int h = 1;
	for (int i = 0; i < len; i++) {
		h += int(text[i]) * pow(151, len - i - 1);
	}
	return h;
}

std::queue<int> rabinKarpBasic(char* chain, char* pattern, int chain_len, int pattern_len) {
	std::queue<int> result;

	size_t pattern_hash = hashText(pattern, pattern_len);
	size_t chain_hash;
	const int loops_amount = chain_len - pattern_len + 1;

	for (int i = 0; i < loops_amount; i++) {
		chain_hash = hashText(charSubstr(chain, i, pattern_len), pattern_len);
		if (chain_hash == pattern_hash) {
			for (int j = 0; j < pattern_len; j++) {
				if (pattern[j] != chain[i + j]) break;
				else if (j == pattern_len - 1) {
					result.push(i);
				}
			}
		}
	}

	return result;
}
