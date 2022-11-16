#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <omp.h>
#include <queue>

#define NUM_THREADS 32

int d = 100;
int q = 151;

omp_lock_t lock;


char* generateString(int len);
void printQueue(std::queue<int> queue);
char* charSubstr(char* text, int offset, int count);
std::queue<int> rabinKarpBasic(char* chain, char* pattern, int chain_len, int pattern_len);
std::queue<int> rabinKarpOMP(char* chain, char* pattern, int chain_len, int pattern_len);


int main() {
	omp_set_num_threads(NUM_THREADS);
	omp_init_lock(&lock);

	srand(time(NULL));

	int chain_len, pattern_len;

	std::cin >> chain_len;
	std::cin >> pattern_len;

	char* chain = generateString(chain_len);
	char* pattern = generateString(pattern_len);

	std::queue<int> resBasic, resOMP, resMPI, resOMP_MPI;
	double start, end;
	
	//start = omp_get_wtime();
	//resBasic = rabinKarpBasic(chain, pattern, chain_len, pattern_len);
	//end = omp_get_wtime();

	//std::cout << "\nExecution time for algorithm without parallelization: " << (end - start) << ".\n";
	//printQueue(resBasic);

	start = omp_get_wtime();
	resOMP = rabinKarpOMP(chain, pattern, chain_len, pattern_len);
	end = omp_get_wtime();

	std::cout << "\nExecution time for algorithm with OpenMP parallelization: " << (end - start) << ".\n";
	//printQueue(resOMP);

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
	for (int i = 0; i < queue.size(); i++) {
		std::cout << queue.front() << " ";
		queue.pop();
	}
}

char* charSubstr(char* text, int offset, int count) {
	char* substring = new char[count];

	for (int i = 0; i < count; i++) {
		substring[i] = text[i + offset];
	}
	return substring;
}

std::queue<int> rabinKarpBasic(char* chain, char* pattern, int chain_len, int pattern_len) {
	std::queue<int> result;

	size_t pattern_hash = std::hash<char*>{}(pattern);
	size_t chain_hash;
	const int loops_amount = chain_len - pattern_len;

	for (int i = 0; i < loops_amount; i++) {
		chain_hash = std::hash<char*>{}(charSubstr(chain, i, pattern_len));
		if (chain_hash == pattern_hash) {
			if (pattern == charSubstr(chain, i, pattern_len)) {
				result.push(i);
			}
		}
	}

	return result;
}

std::queue<int> rabinKarpOMP(char* chain, char* pattern, int chain_len, int pattern_len) {
	std::queue<int> result;

	size_t pattern_hash = std::hash<char*>{}(pattern);
	size_t chain_hash;
	const int loops_amount = chain_len - pattern_len;

	//#pragma omp parallel for schedule(static, 8) shared(result) 
#pragma omp parallel for schedule(dynamic, 100) shared(result)
	for (int i = 0; i < loops_amount; i++) {
		chain_hash = std::hash<char*>{}(charSubstr(chain, i, pattern_len));
		if (chain_hash == pattern_hash) {
			if (pattern == charSubstr(chain, i, pattern_len)) {
				omp_set_lock(&lock);
				result.push(i);
				omp_unset_lock(&lock);
			}
		}
	}

	return result;
}