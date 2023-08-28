// functions.h

#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#define MAX_ROWS 150
#define MAX_COLS 150

#define MAX_TOUR 15

long getSize(char *filename);
int getMax(int *arr, int length);
void printMatrix(int rows, int cols, int distanceMatrix[MAX_ROWS][MAX_COLS]);
unsigned long** createMatrix(int rows, int cols, int distanceMatrix[MAX_ROWS][MAX_COLS], int* attractionLabels, int labelLength);
void writeMatrixToJsonFile(unsigned long** matrix, int rows, int cols, const char* filename);
void printDash();
int getCost(unsigned long** adjustedMatrix, int* attractionLabels, int rows);
void flip(int *tour, int *newTour, int a, int b, int tourLength);
void swap(int *a, int *b);
void shuffleArray(int arr[], int size);
char* clockTime(int minutes);
void generateNewIndices(const int shuffled[], int size, const int original[], int newIndices[]);
int calculateSegments(int startMinutes, int currentMinutes, int segment);
void printArray(const int arr[], int size);
int calculateWait(int matrix[MAX_ROWS][MAX_COLS], int index, int segments);


#endif // FUNCTIONS_H