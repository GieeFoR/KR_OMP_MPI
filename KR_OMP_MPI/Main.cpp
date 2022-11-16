#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <omp.h>
#include <queue>
#include <cmath>

#define NUM_THREADS 32

int d = 151;
int q = 997;

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

	//std::cin >> chain_len;
	//std::cin >> pattern_len;

	//for tests
	chain_len = 1000000;
	pattern_len = 100;

	char* chain = generateString(chain_len);
	char* pattern = generateString(pattern_len);

	std::queue<int> resBasic, resOMP, resMPI, resOMP_MPI;
	double start, end;
	
	start = omp_get_wtime();
	resBasic = rabinKarpBasic(chain, pattern, chain_len, pattern_len);
	end = omp_get_wtime();

	std::cout << "\nExecution time for algorithm without parallelization: " << (end - start) << ".\n";
	//printQueue(resBasic);

	start = omp_get_wtime();
	resOMP = rabinKarpOMP(chain, pattern, chain_len, pattern_len);
	end = omp_get_wtime();

	std::cout << "\nExecution time for algorithm with OpenMP parallelization: " << (end - start) << ".\n";
	//printQueue(resOMP);


	//release dynamic allocated mem
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

int hashText(char* text, int len) {
	int h = 1;
	for (int i = 0; i < len; i++) {
		h += int(text[i]) * pow(151, len - i - 1);		//pomyœleæ nad sta³¹; aktualnie = 151
	}
	return h;
}

std::queue<int> rabinKarpBasic(char* chain, char* pattern, int chain_len, int pattern_len) {
	std::queue<int> result;

	size_t pattern_hash = hashText(pattern, pattern_len);
	size_t chain_hash;
	const int loops_amount = chain_len - pattern_len;

	for (int i = 0; i < loops_amount; i++) {
		chain_hash = hashText(charSubstr(chain, i, pattern_len), pattern_len);
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
	size_t pattern_hash = hashText(pattern, pattern_len);
	const int loops_amount = chain_len - pattern_len; //+1?

	#pragma omp parallel for schedule(dynamic, 8) shared(result) private(pattern_hash)
	for (int i = 0; i < loops_amount; i++) {

		size_t chain_hash = 1;
		#pragma omp parallel for schedule(static, 5) // reduction (+ : chain_hash) 
		for (int j = 0; j < pattern_len; j++) {
			chain_hash += int(chain[j + pattern_len]) * pow(151, pattern_len - j - 1); //pomyœleæ nad sta³¹; aktualnie = 151
		}

		if (chain_hash == pattern_hash) {
			//best way to compare chain with pattern one by one letter?
			//#pragma omp parallel for schedule(static, 5) private(i) shared(result)
			for (int j = 0; j < pattern_len; j++) {
				if (pattern[j] != chain[i + j]) break;
				else if (j == pattern_len-1) { //do tablicy wpisujemy dopiero po sprawdzeniu wszystkich znaków
					omp_set_lock(&lock);
					result.push(0);
					omp_unset_lock(&lock);
				}
			}
		}
	}

	return result;
}