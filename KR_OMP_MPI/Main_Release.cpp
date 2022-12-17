#include <iostream>
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
void printQueue(std::queue<int> queue);
char* charSubstr(char* text, int offset, int count);
std::queue<int> rabinKarpBasic(char* chain, char* pattern, int chain_len, int pattern_len);
std::queue<int> rabinKarpOMP(char* chain, char* pattern, int chain_len, int pattern_len);


int main(int argc, char** argv) {
	omp_set_num_threads(NUM_THREADS);
	omp_init_lock(&lock);

	srand(time(NULL));

    unsigned long jobSizes[] = { 100000000, 10000000, 1000000, 100000, 10000, 1000, 100 };

	file.imbue(std::locale(std::locale::classic(), new Comma));
	file.open("wyniki.csv");

	std::queue<int> resBasic, resOMP, resMPI, resOMP_MPI;
	double start, end;	

    file << "D³ugoœæ tekstu" << ";" << "Czas bez zrównoleglenia" << ";" << "Czas z OpenMP" << endl;

	for (size_t js = 0; js < 7; js++) 
	{

        int chain_len = jobSizes[js];
        int pattern_len = 10;

        char* chain = generateString(chain_len);
	    char* pattern = generateString(pattern_len);

        start = omp_get_wtime();
        resBasic = rabinKarpBasic(chain, pattern, chain_len, pattern_len);
        end = omp_get_wtime();

        double timeBasic = end - start;
        printf("\nExecution time for text size %d and algorithm without parallelization: %f", chain_len, timeBasic);

        start = omp_get_wtime();
        resOMP = rabinKarpOMP(chain, pattern, chain_len, pattern_len);
        end = omp_get_wtime();

        double timeOMP = end - start;
        printf("\nExecution time for text size %d and algorithm with OpenMP parallelization: %f", chain_len, timeOMP);

        file << chain_len << ";" << timeBasic << ";" << timeOMP << endl;

        delete chain;
        delete pattern;
    }

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
		h += int(text[i]) * pow(151, len - i - 1);		//pomyœleæ nad sta³¹; aktualnie = 151
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

std::queue<int> rabinKarpOMP(char* chain, char* pattern, int chain_len, int pattern_len) {
	std::queue<int> result;
	size_t pattern_hash = hashText(pattern, pattern_len);
	const int loops_amount = chain_len - pattern_len + 1;

	#pragma omp parallel for schedule(static) shared(result) firstprivate(pattern_hash)
	for (int i = 0; i < loops_amount; i++) {

		size_t chain_hash = 1;
		#pragma omp parallel for schedule(static)
		for (int j = 0; j < pattern_len; j++) {
			chain_hash += int(chain[i + j]) * pow(151, pattern_len - j - 1); //pomyœleæ nad sta³¹; aktualnie = 151
		}

		if (chain_hash == pattern_hash) {
			for (int j = 0; j < pattern_len; j++) {
				if (pattern[j] != chain[i + j]) break;
				else if (j == pattern_len-1) {
					omp_set_lock(&lock);
					result.push(i);
					omp_unset_lock(&lock);
				}
			}
		}
	}

	return result;
}