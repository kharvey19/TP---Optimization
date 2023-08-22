#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

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

	// reading in pointers to lists we need 
	cJSON *attractions = cJSON_GetObjectItemCaseSensitive(json, "Evaluate535Only");
	cJSON *rideMatrix = cJSON_GetObjectItem(json, "RideMatrix");

	// rows can be used to represent length 
	int rows = cJSON_GetArraySize(attractions);
	int cols = cJSON_GetArraySize(attractions);
	
	// saving the location of matrices to later make them into a 2D array 
	cJSON *distanceData = cJSON_GetObjectItem(json, "DistanceMatrix");
	cJSON *waitData = cJSON_GetObjectItem(json, "WaitMatrix");

	// reading in vars
	cJSON *start = cJSON_GetObjectItemCaseSensitive(json, "Start");
	int startTime = cJSON_GetNumberValue(start);
	cJSON *sliceLength = cJSON_GetObjectItemCaseSensitive(json, "TimesliceLength");
	int segment = cJSON_GetNumberValue(sliceLength);


	cJSON *attractionKey = cJSON_GetObjectItemCaseSensitive(json, "AttractionsToInclude");
	int *key = malloc(rows * sizeof(int));
	cJSON *l;
	int j = 0;
	cJSON_ArrayForEach(l, attractionKey) {
		*(key + j) = atoi(strtok(l-> valuestring, "HS"));
		j++;
	}
        
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

	// copying the data from the wait matrix into a C array 

	int wait_row_idx = 0;
	cJSON *waitRowArray, *waitCellValue;
	int waitMatrix[MAX_ROWS][MAX_COLS];
	int wait_rows = 12;
	int wait_cols = 57;

	cJSON_ArrayForEach(waitRowArray, waitData) {
		if (wait_row_idx >= MAX_ROWS) {
			printf("Too many rows in the JSON data.\n");
			cJSON_Delete(json);
			return 1;
		}

		int wait_col_idx = 0;
		cJSON_ArrayForEach(waitCellValue, waitRowArray) {
			if (wait_col_idx >= MAX_COLS) {
				printf("Too many columns in the JSON data.\n");
				cJSON_Delete(json);
				return 1;
			}

			waitMatrix[wait_row_idx][wait_col_idx] = waitCellValue->valueint;
			wait_col_idx++;
		}

		if (wait_col_idx != wait_cols) {
			printf("Invalid number of columns in row %d.\n", wait_row_idx);
			cJSON_Delete(json);
			return 1;
		}

		wait_row_idx++;
	}

	if (wait_row_idx != wait_rows) {
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

		int rideMatrixTotal = 0;
		
		cJSON* rideValue;
		int *rideMatrixArray = malloc(rows * sizeof(int));
		int ride = 0;

		cJSON_ArrayForEach(rideValue, rideMatrix) {
			rideMatrixTotal += rideValue -> valueint;
			*(rideMatrixArray + ride) = rideValue -> valueint;  // Store the value in the allocated memory
			ride++;
		}

/*............................................................................*/

		// printing findings

		printf("\n");

		printf("Original array: ");
		printArray(attractionLabels, rows);
		printf("\n\n");

		printf("Max Attraction: %d\n\n", maxAttraction);

		printf("Original Distance Matrix: \n\n");
		printMatrix(rows, cols, distanceMatrix);
		printf("\n");

		unsigned long** adjustedMatrix = createMatrix(maxAttraction, maxAttraction, distanceMatrix, key, rows);

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

		int *orig = (int*)malloc(rows * sizeof(int));
		    for (int i = 0; i < rows; i++) {
        		orig[i] = attractionLabels[i];
    		}

		// implementing Lin-Kernighan 
		int *original_tour = attractionLabels;
		int best_cost = original_cost;
		int *best_tour = original_tour;
		int max_loops = 1000;
		int new_tour[MAX_TOUR], new_cost;

		printf("\nOrig. Array: ");
		printArray(orig, rows);
		printf("\n\n");

		srand(time(NULL));

		// clocking CPU
		clock_t start_time, end_time;
		start_time = clock(); 

		// algorithm implementation
		while (max_loops-- > 0) {
			int p1 = (rand() % (rows - 2)) + 1;
			int p2 = p1;
			while (p2 == p1) {
				p2 = (rand() % (rows - 2)) + 1;
			}

			flip(original_tour, new_tour, p1, p2, rows);
			new_cost = getCost(adjustedMatrix, new_tour, rows);

			int gain = original_cost - new_cost; 

			if (gain > 0) {

				if (new_cost < best_cost) {
					best_cost = new_cost;
					memcpy(best_tour, new_tour, rows * sizeof(int));
					printf("Found new best tour of %d cost\n", best_cost);
				}

				memcpy(original_tour, best_tour, rows * sizeof(int));
				shuffleArray(original_tour, rows);

				end_time = clock();
				double cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

				printf("Current Array: ");
				printArray(new_tour, rows);
				printf("| Cost: %d | ", new_cost);
				printf("CPU: %f seconds\n", cpu_time_used);

			}
		}

		printf("\nTotal walking time after Lin-Kernighan: %d minutes - %0.2f hours\n", best_cost, ((float)best_cost) / 60);
		printf("Total time including ride matrix: %d\n\n", rideMatrixTotal + best_cost);
		
		printf("Most Optimal Tour: ");
		printArray(best_tour, rows);

		printf("\nKey: ");
		printArray(key, rows);

		// finding what row each value in best tour is 
		int newIndices[rows];
		generateNewIndices(best_tour, rows, key, newIndices);
		printf("\n\nNew Indices: ");
		printArray(newIndices, rows);

		// printing start time in clock format
		char* startTimeStr = clockTime(startTime);
		printf("\n\n\nStart Time: %s\n", startTimeStr);
		free(startTimeStr);

		/* the loop to figure out the arrival time, mount time, and dismount time
		   for each attraction */
		int off_time = startTime;
		for (int i = 1; i < rows - 1; i++) {
			int arrival_time = off_time + (int)adjustedMatrix[best_tour[i - 1]][best_tour[i]];
			char* arrivalTimeStr = clockTime(arrival_time);
			printf("\nArrives at %d: %s\n", best_tour[i], arrivalTimeStr);
			free(arrivalTimeStr); // Free the memory allocated by clockTime

			int mount_time = arrival_time + calculateWait(waitMatrix, newIndices[i], calculateSegments(startTime, arrival_time, segment));
			char* mountTimeStr = clockTime(mount_time);
			printf("Mounts attraction at %s\n", mountTimeStr);
			free(mountTimeStr); // Free the memory allocated by clockTime

			off_time = mount_time + rideMatrixArray[newIndices[i]];
			char* getTimeStr = clockTime(off_time);
			printf("Gets off attraction at %s\n", getTimeStr);
			free(getTimeStr); // Free the memory allocated by clockTime
		}

		// calculate the time it takes to walk back to 37 (the entrance)
		end_time = off_time + (int)adjustedMatrix[best_tour[rows - 2]][best_tour[rows - 1]];
		char* finish_time = clockTime(end_time);
		printf("\nArrives back to park entrance at %s\n", finish_time);
		free(finish_time);

		printDash();

		// useful for checking if the above loop is correct 
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
		free(orig);
		free(rideMatrixArray);
		free(key);


	}   

/*............................................................................*/

	cJSON_Delete(json);

	return 0;
}
