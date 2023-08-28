// source.c

#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "functions.h"

/*............................................................................*/

#define RESULT_ERROR (-1)

/* Max rows for matrices */
#define MAX_ROWS 150
#define MAX_COLS 150

#define MAX_TOUR 15

/*............................................................................*/

/* Auxillary Functions: 
	1. getSize -> Determines size of file 
	2. getMax -> gets the max element in an array 
	3. printMatrix -> used to print original distance matrix 
	4. createMatrix -> creates a square matrix with the correct values and 
	   indexes for walking times 
	5. writeMatrixToJsonFile ->  writes the matrix to the json file
	6. printDash -> prints a line of dashes to separate printed outputs 
	7. getCost -> gets the cost (walking time) for a tour
	8. flip -> given 2 indices (and other info) will flip/ reverse a substring
	   in the tour 
	9. swap -> swaps to elements in an array 
	10. shuffleArray -> shuffles an array, omitting the first and last element 
	11. clockTime -> given the minutes since midnight, returns string for actual
		time in the day
	12. generateNewIndices -> returns an array of the original indices of the 
		shuffled array
	13. calculateSegments -> returns the number of segments that have passed 
		since the start time 
	14. printArray -> prints arrays 
	15. calculateWait -> calculate the average of the segments the time is in 
		between */
	
	
// Determines size of file 
long getSize(char *filename) {
    struct stat file_status;
    if (stat(filename, &file_status) < 0) {
        return RESULT_ERROR;
    }
    return file_status.st_size;
}

// gets the max element in an array
int getMax(int *arr, int length) {
    if (length <= 0) {
        return RESULT_ERROR;
    }

	/* makes the first element of the array the max then checks every element
	   and replaces max with larger values until it reaches the end of the array */
    int max = arr[0];
    for (int i = 1; i < length; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

// literally prints the matrix in the terminal 
void printMatrix(int rows, int cols, int distanceMatrix[MAX_ROWS][MAX_COLS]) {

	int maxDigits = 2;
	/* determine the number of spaces by: 
	   	1. determining how many digits the current number has
		2. determining how many spaces needed in front of the number 
			(2 digit = 0 spaces, 1 digit = 1 space) 
				> every number has a space behind it */
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			int num = distanceMatrix[i][j];
			int digits = 0;
			while (num != 0) {
				num /= 10;
				digits++;
			}

			int numSpaces = maxDigits - digits;
			for (int k = 0; k < numSpaces; k++) {
				printf(" ");
			}
			printf("%d ", distanceMatrix[i][j]);
		}
		printf("\n");
	}
	
}

/* creates a square matrix with the correct values and indexes for walking 
   times using the distance matrix  */
unsigned long** createMatrix(int rows, int cols, int distanceMatrix[MAX_ROWS][MAX_COLS], int* attractionLabels, int labelLength) {
	
	// update rows and cols, so that they are + 1
	cols = cols + 1;
	rows = rows + 1;

	// allocate memory for matrix, type-casted to unsigned long**
    unsigned long** matrix = (unsigned long**)malloc(rows * sizeof(unsigned long*));

    for (int i = 0; i < rows; i++) {
        matrix[i] = (unsigned long*)malloc(cols * sizeof(unsigned long));
        memset(matrix[i], 0, cols * sizeof(unsigned long));
    }
	
    for (int i = 0; i < labelLength; i++) {
		
        int rowIndex = attractionLabels[i];

        if (rowIndex >= 0 && rowIndex < rows) {

            for (int j = 0; j < labelLength; j++) {

                int colIndex = attractionLabels[j];

                if (colIndex >= 0 && colIndex < cols) {

					matrix[attractionLabels[i]][attractionLabels[j]] = distanceMatrix[i][j];                
				}
            }
        }
    }
    return matrix;
}



// function to write a matrix to a JSON file
void writeMatrixToJsonFile(unsigned long** matrix, int rows, int cols, const char* filename) {
	
	cols = cols + 1;
	rows = rows + 1;
    // cJSON object that holds the matrix
    cJSON* root = cJSON_CreateObject();

    cJSON* matrixArray = cJSON_CreateArray();
    
    for (int i = 0; i < rows; i++) {
        // creates a cJSON array for each row
        cJSON* rowArray = cJSON_CreateArray();

		// fills in each element in the row
        for (int j = 0; j < cols; j++) {
            cJSON* cellValue = cJSON_CreateNumber(matrix[i][j]);
            cJSON_AddItemToArray(rowArray, cellValue);
        }

        // adds the row array to the main matrix array
        cJSON_AddItemToArray(matrixArray, rowArray);
    }

    // adds the matrix array as a property to the root object
    cJSON_AddItemToObject(root, "DistanceMatrix", matrixArray);

    char* json_str = cJSON_PrintUnformatted(root);

    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Error: Unable to open the file.\n");
        return;
    }

	// included below to format JSON better 

    // writes each character of the JSON string manually
    for (unsigned long i = 0; i < strlen(json_str); i++) {
        fputc(json_str[i], fp);
        // adds a newline after each row array
		if (json_str[i] == '{') {
            fputc(' ', fp);
        }
		if (json_str[i - 1] == ':' && json_str[i] == '[') {
            fputc('\n', fp);
        }
        if (json_str[i] == ']') {
            fputc('\n', fp);
        }
    }

    fclose(fp);

    cJSON_free(json_str);
    cJSON_Delete(root);
}

// prints a line of dashes to separate printed outputs 
void printDash() {
	printf("\n\n");
	for (int i = 0; i < 50; i++) {
		printf("-");
	}
	printf("\n\n");
}

// gets the cost (walking time) for a tour, by checking the distance matrix 
int getCost(unsigned long** adjustedMatrix, int* attractionLabels, int rows) {
	int count = 0;

	for (int i = 0; i < rows - 1; i++) {
		count = count + adjustedMatrix[attractionLabels[i]][attractionLabels[i + 1]];
	}

	return count;
}

// given 2 indices (and other info) will flip/ reverse a substring in the tour 
void flip(int *tour, int *newTour, int a, int b, int tourLength) {
    int frontFillIndex, backFillIndex;
    if (a < b) {
        frontFillIndex = a;
        backFillIndex = b;
    } else {
        frontFillIndex = b;
        backFillIndex = a;
    }

    // copy elements before the flip section
    for (int i = 0; i < frontFillIndex; i++) {
        newTour[i] = tour[i];
    }

    // copy elements in reverse order for the flip section
    for (int i = 0; i <= backFillIndex - frontFillIndex; i++) {
        newTour[frontFillIndex + i] = tour[backFillIndex - i];
    }

    // copy elements after the flip section
    for (int i = backFillIndex + 1; i < tourLength; i++) {
        newTour[i] = tour[i];
    }
}

// swap
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// shuffles an array, omitting the first and last element 
void shuffleArray(int arr[], int size) {
    for (int i = size - 2; i > 1; i--) {
        int j = rand() % (i - 1) + 1; 
        swap(&arr[i], &arr[j]);  
    }
}

// given the minutes since midnight, returns string for actual time in the day
char* clockTime(int minutes) {
    char* timeStr = (char*)malloc(13 * sizeof(char));
    if (timeStr == NULL) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    int hours = minutes / 60;
    int mins = minutes % 60;

    char amPm[3]; // For "AM" or "PM"
    if (hours >= 12) {
        strcpy(amPm, "PM");
        if (hours > 12) {
            hours -= 12;
        }
    } else {
        strcpy(amPm, "AM");
        if (hours == 0) {
            hours = 12;
        }
    }

    sprintf(timeStr, "%02d:%02d %s", hours, mins, amPm);

    return timeStr;
}

// returns an array of the original indices of the shuffled array
void generateNewIndices(const int shuffled[], int size, const int original[], int newIndices[]) {
    for (int i = 0; i < size; i++) {
        int currentElement = shuffled[i];
        for (int j = 0; j < size; j++) {
            if (original[j] == currentElement) {
                newIndices[i] = j;
                break;
            }
        }
    }
}

// returns the number of segments that have passed since the start time 
int calculateSegments(int startMinutes, int currentMinutes, int segment) {
    int minutesPassed = currentMinutes - startMinutes;
    int segmentsPassed = minutesPassed / segment;
    
    return segmentsPassed;
}

// prints arrays 
void printArray(const int arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
}


/* returns the wait time by calculating the average of the "tme neighbors" in 
   the wait matrix */
int calculateWait(int matrix[MAX_ROWS][MAX_COLS], int index, int segments) {
	int a = matrix[index][segments];
	int b = matrix[index][segments + 1];

	return (a + b) / 2; 
}