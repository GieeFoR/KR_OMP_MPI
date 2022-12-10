//
//#include<iostream>
//#include<string.h>
//#include<mpi.h>
//#include<math.h>
//
//
//#define max_characters 128
//#define ASCII_START 32
//#define ASCII_END 126
//
//
//using std::cout;
//using std::endl;
//using std::cin;
//using std::string;
//
//
//
//void search(char pattern[], char text[])
//{
//	int pattern_lenght = strlen(pattern);
//	int text_lenght = strlen(text);
//	int pattern_hash = 0;
//	int text_hash = 0;
//	for (int i = 0; i < pattern_lenght; i++)
//	{
//		pattern_hash += int(pattern[i]) * pow(max_characters, pattern_lenght - i - 1);
//	}
//	for (int i = 0; i < text_lenght - pattern_lenght; i++)
//	{
//		for (int j = i; j < i + pattern_lenght; j++)
//		{
//			text_hash += int(text[j]) * pow(max_characters, pattern_lenght - j - 1 + i);
//		}
//		if (pattern_hash == text_hash)
//		{
//			std::cout << "string found at position " << i << std::endl;
//		}
//		text_hash = 0;
//	}
//}
//
//char* generateRandomString(int size)
//{
//	int i;
//	char* res = (char*)malloc(size + 1);
//	for (i = 0; i < size; i++) {
//		res[i] = (char)(rand() % (ASCII_END - ASCII_START)) + ASCII_START;
//	}
//	res[i] = '\0';
//	return res;
//}
//
//
//
//void MPI_search(const char pattern[], const char text[], int startIndex, int stopIndex)
//{
//
//	int pattern_lenght = strlen(pattern);
//	int text_lenght = strlen(text);
//	int pattern_hash = 0;
//	int text_hash = 0;
//	if (stopIndex >= text_lenght - pattern_lenght) stopIndex = text_lenght - pattern_lenght + 1;
//	for (int i = 0; i < pattern_lenght; i++)
//	{
//		pattern_hash += int(pattern[i]) * pow(max_characters, pattern_lenght - i - 1);
//	}
//	for (int i = startIndex; i < stopIndex; i++)
//	{
//		for (int j = i; j < i + pattern_lenght; j++)
//		{
//			text_hash += int(text[j]) * pow(max_characters, pattern_lenght - j - 1 + i);
//		}
//		if (pattern_hash == text_hash)
//		{
//			std::cout << "string found at position " << i << std::endl;
//		}
//		text_hash = 0;
//	}
//}
//int main(int argc, char** argv)
//{
//	srand(time(NULL));
//	int rank, size;
//	int indexes[2];
//	MPI_Status status;
//	MPI_Init(&argc, &argv);
//	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//	MPI_Comm_size(MPI_COMM_WORLD, &size);
//	const char* text = generateRandomString(1000000);
//	const char* pattern = generateRandomString(100);
//
//	if (rank == 0) {
//		double start = MPI_Wtime();
//		int pattern_lenght = strlen(pattern);
//		int text_lenght = strlen(text);
//		int step = text_lenght / (size - 1);
//		int bufor;
//		int indexesSend[2];
//		if (pattern_lenght)
//
//			for (int i = 1; i < size; i++) {
//				indexesSend[0] = (i - 1) * step;
//				indexesSend[1] = ((i - 1) * step) + step;
//				MPI_Send(indexesSend, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
//			}
//		for (int i = 1; i < size; i++) {
//			MPI_Recv(&bufor, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
//		}
//
//		double end = MPI_Wtime();
//		std::cout << "The process" << rank << " took " << end - start << " seconds to run." << std::endl;
//
//	}
//	else {
//		MPI_Recv(&indexes, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
//		MPI_search(pattern, text, indexes[0], indexes[1]);
//		int b = 1;
//		MPI_Send(&b, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
//	}
//	MPI_Finalize();
//	//search(pattern, text, max_characters);
//}
//
