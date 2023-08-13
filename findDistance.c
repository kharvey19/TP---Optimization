#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>

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
	4. createMatrix -> creates a square matrix of 0s based on the inputted 
	   length (max attraction)
	5. writeMatrixToJsonFile ->  writes the matrix to the json file */

long getSize(char *filename) {
    struct stat file_status;
    if (stat(filename, &file_status) < 0) {
        return RESULT_ERROR;
    }
    return file_status.st_size;
}

int getMax(int *arr, int length) {
    if (length <= 0) {
        return RESULT_ERROR;
    }

    int max = arr[0];
    for (int i = 1; i < length; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

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
		// printf("\n");
	
}

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

void printDash() {
	printf("\n\n");
	for (int i = 0; i < 50; i++) {
		printf("-");
	}
	printf("\n\n");
}

int getCost(unsigned long** adjustedMatrix, int* attractionLabels, int rows) {
	int count = 0;

	for (int i = 0; i < rows - 1; i++) {
		count = count + adjustedMatrix[attractionLabels[i]][attractionLabels[i + 1]];
	}

	return count;
}

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

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void shuffleArray(int arr[], int size) {
    for (int i = size - 2; i > 1; i--) {
        int j = rand() % (i - 1) + 1; 
        swap(&arr[i], &arr[j]);  
    }
}

/*............................................................................*/

int main() {

	// reading data from json file 
    char *filename = "useCase.json";
	FILE *fp = fopen("useCase.json", "r");
	if (fp == NULL) {
		printf("Error: Unable to open the file.\n");
		return 1;
	}

	char buffer[getSize(filename)];
	fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);

	cJSON *json = cJSON_Parse(buffer);
	if (json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			printf("Error: %s\n", error_ptr);
		}
		cJSON_Delete(json);
		return 1;
	}

/*............................................................................*/

	// defining variables we will use 
	cJSON *attractions = cJSON_GetObjectItemCaseSensitive(json, "Evaluate535Only");
	int rows = cJSON_GetArraySize(attractions);
	int cols = cJSON_GetArraySize(attractions);
	cJSON *distanceData = cJSON_GetObjectItem(json, "DistanceMatrix");

/*............................................................................*/


	// copying the data from the distance matrix into a C array 
	int row_idx = 0;
	cJSON *rowArray, *cellValue;
	int distanceMatrix[MAX_ROWS][MAX_COLS];

	cJSON_ArrayForEach(rowArray, distanceData) {
		if (row_idx >= MAX_ROWS) {
			printf("Too many rows in the JSON data.\n");
			cJSON_Delete(json);
			return 1;
		}

		int col_idx = 0;
		cJSON_ArrayForEach(cellValue, rowArray) {
			if (col_idx >= MAX_COLS) {
				printf("Too many columns in the JSON data.\n");
				cJSON_Delete(json);
				return 1;
			}

			distanceMatrix[row_idx][col_idx] = cellValue->valueint;
			col_idx++;
		}

		if (col_idx != cols) {
			printf("Invalid number of columns in row %d.\n", row_idx);
			cJSON_Delete(json);
			return 1;
		}

		row_idx++;
	}

	if (row_idx != rows) {
		printf("Invalid number of rows in the JSON data.\n");
		cJSON_Delete(json);
		return 1;
	}

/*............................................................................*/

	// creating an array for the attractions + determining max attraction
    if (cJSON_IsArray(attractions)) {
		cJSON *i;
		int *attractionLabels = malloc(rows * sizeof(int));
		int index = 0;

        cJSON_ArrayForEach(i, attractions) {
			*(attractionLabels + index) = i-> valueint;
			index++;
        }
	
		int maxAttraction = getMax(attractionLabels, rows);

/*............................................................................*/

		// printing findings

		printf("\n");

		printf("Original array: ");

        for (int i = 0; i < rows; i++) {
            printf("%d ", attractionLabels[i]);
        }        
		printf("\n\n");

		printf("Max Attraction: %d\n\n", maxAttraction);

		printf("Original Distance Matrix: \n\n");

		printMatrix(rows, cols, distanceMatrix);

		printf("\n");

		unsigned long** adjustedMatrix = createMatrix(maxAttraction, maxAttraction, distanceMatrix, attractionLabels, rows);

		printDash();

		printf("Distance of manually entered attractions: %lu minutes", adjustedMatrix[95][113]);

		printDash();

		printf("Snippet of Combos Row by Row (edit limit manually): \n\n");

		// change upper bound to show the amount of combos you want to see 
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				printf("Distance between %d and %d is %lu minutes \n", attractionLabels[i], attractionLabels[j], adjustedMatrix[attractionLabels[i]][attractionLabels[j]]);
			}
		}

		printDash();
		
		int original_cost = getCost(adjustedMatrix, attractionLabels, rows);

		printf("Total walking time of entered sequence: %d minutes - %0.2f hours\n", original_cost, ((float)original_cost) / 60);

		int *original_tour = attractionLabels;
		int best_cost = original_cost;
		int *best_tour = original_tour;
		int max_loops = 100;
		int new_tour[MAX_TOUR], new_cost;

		printf("\nOrig. Array: ");
		for (int i = 0; i < rows; i++) {
			printf("%d ", best_tour[i]);
		}
		printf("\n\n");

		while (max_loops-- > 0) {
			srand(time(NULL)); 

			int p1 = (rand() % (rows - 2)) + 1;
			int p2 = p1;
			while (p2 == p1) {
				p2 = (rand() % (rows - 2)) + 1;
			}

			flip(original_tour, new_tour, p1, p2, rows);
			new_cost = getCost(adjustedMatrix, new_tour, rows);

			int gain = original_cost - new_cost; 

			if (gain > 0) {
				clock_t start_time, end_time;
    			start_time = clock(); 

				if (new_cost < best_cost) {
					best_cost = new_cost;
					memcpy(best_tour, new_tour, rows * sizeof(int));
				}

				memcpy(original_tour, best_tour, rows * sizeof(int));
				shuffleArray(original_tour, rows);

				end_time = clock();
				// not sure if this is right
				double cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

				printf("Optimized Array: ");
				for (int i = 0; i < rows; i++) {
					printf("%d ", new_tour[i]);
				}
				printf("| Cost: %d | ", new_cost);
				printf("CPU: %f seconds\n", cpu_time_used);

			}
		}

		printf("\nTotal walking time after Lin-Kernighan: %d minutes - %0.2f hours\n", best_cost, ((float)best_cost) / 60);


		printDash();

		printf("Individual Ride-Combo Calculations: \n\n");

		for (int i = 0; i < rows - 1; i++) {
			printf("Distance from %d to %d is %lu minutes \n", attractionLabels[i], attractionLabels[i + 1], adjustedMatrix[attractionLabels[i]][attractionLabels[i + 1]]);
		}

		printf("\n");

		writeMatrixToJsonFile(adjustedMatrix, maxAttraction, maxAttraction, "adjustedMatrix.json");

		for (int i = 0; i < maxAttraction + 1; i++) {
			free(adjustedMatrix[i]);
		}
		free(attractionLabels);
		free(adjustedMatrix);


	}   

/*............................................................................*/

	cJSON_Delete(json);

	return 0;
}
