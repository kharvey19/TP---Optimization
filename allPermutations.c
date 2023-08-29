#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "functions.h"



/* How to compile and use code:
    1. gcc -c allPermutations.c -o allPermutations.o
    2. gcc source.o allPermutations.o -o program -L../cJSON -lcjson
    3. ./program
*/

/* struct that holds the total time for each permutation (number) and the count 
   of how many times that time appeared in the array */ 
struct pairs {
    int number;
    int count;
};

/* struct that holds an array with the total time for each permutation and the 
   count as the total permutations */ 
struct StorageResult {
    int* storageArray;
    int count;
};

// represents an individual element in the hash table
struct HashNode {
    // value stored in the hash table 
    int key;
    /* used to keep track of how many times a specific key appears in
       the input list */
    int count;
    /* used to point to the next node in the linked list at a 
       particular hash index */ 
    struct HashNode* next;
};

// holds an array of pointers to HashNode instances and a size (for storing data)
struct HashTable {
    /* represents the number of slots in the hash table.
       Each slot can store a linked list of "HashNode" instances */
    int size;
    /* This is a dynamic array (array of pointers to HashNode pointers) 
       that will hold the buckets or slots. Each element of this array 
       points to the head of a linked list of HashNode instances. The 
       actual linked list handles collisions by chaining nodes that hash 
       to the same index. */
    struct HashNode** table;
};

// factorial function used for finding total number of permutations 
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// used to insert a new key into the hash table 
void insert(struct HashTable* hashTable, int key) {
    
    /* calculate the index by taking the remainder of the key when divided
       by the size of the hash table */
    int index = key % hashTable->size;

    // check if the key is already present
    struct HashNode* current = hashTable->table[index];
    while (current != NULL) {
        if (current->key == key) {
            // key is already present, increase the count
            current->count++;
            return;
        }
        current = current->next;
    }

    /* if not present, create a new node for the key and insert it at
       the beginning of the linked list */
    struct HashNode* newNode = (struct HashNode*)malloc(sizeof(struct HashNode));
    if (newNode == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }

    // create new node for the key 
    newNode->key = key;
    newNode->count = 1;
    newNode->next = hashTable->table[index];
    hashTable->table[index] = newNode;
}

// print keys and their counts from the hash table
void printHash(struct HashTable* hashTable) {
    for (int i = 0; i < hashTable->size; i++) {
        struct HashNode* current = hashTable->table[i];
        while (current != NULL) {
            printf("Total Time: %d | Count: %d\n", current->key, current->count);
            current = current->next;
        }
    }
}

/*  creates a hash table by taking a size as input, allocating memory for a hash table,
    allocating memory for the array of buckets, initializing each bucket to NULL, and then 
    returning the created hash table.*/
struct HashTable* createHashTable(int size) {

    struct HashTable* hashTable = (struct HashTable*)malloc(sizeof(struct HashTable));
    if (hashTable == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }

    hashTable->size = size;
    hashTable->table = (struct HashNode**)malloc(size * sizeof(struct HashNode*));
    if (hashTable->table == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }

    // Initialize table entries to NULL
    for (int i = 0; i < size; i++) {
        hashTable->table[i] = NULL;
    }

    return hashTable;
}

// Lin-Kernighan Algorithm 
int lin(int* attractionLabels, int rows, int* key, int startTime, unsigned long** adjustedMatrix, int waitMatrix[MAX_ROWS][MAX_COLS], int *rideMatrixArray){
    
    // copying attraction array to a new array 
    int *orig = (int*)malloc(rows * sizeof(int));
        for (int i = 0; i < rows; i++) {
            orig[i] = attractionLabels[i];
        }

    int original_cost = getCost(adjustedMatrix, attractionLabels, rows);

    int *original_tour = attractionLabels;
    int best_cost = original_cost;
    int *best_tour = original_tour;
    int max_loops = 1000;
    int new_tour[MAX_TOUR], new_cost;

    srand(time(NULL));

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
            }

            memcpy(original_tour, best_tour, rows * sizeof(int));
            shuffleArray(original_tour, rows);

        }
    }

    // finds the original index of each attraction 
    int newIndices[rows];
    generateNewIndices(best_tour, rows, key, newIndices);

    /* the loop to figure out the arrival time, mount time, and dismount time
        for each attraction */
    int off_time = startTime;
    for (int i = 1; i < rows - 1; i++) {
        int arrival_time = off_time + (int)adjustedMatrix[best_tour[i - 1]][best_tour[i]];
        int mount_time = arrival_time + calculateWait(waitMatrix, newIndices[i], calculateSegments(startTime, arrival_time, 15));
        off_time = mount_time + rideMatrixArray[newIndices[i]];
    }

    // calculate the time it takes to walk back to 37 (the entrance)
    int end_time = off_time + (int)adjustedMatrix[best_tour[rows - 2]][best_tour[rows - 1]];

    return end_time - startTime;

}

// function that finds all possible permutations of the list
struct StorageResult brheap_nonrecur(int* arr, int length, int* key, int startTime, unsigned long** adjustedMatrix, int waitMatrix[MAX_ROWS][MAX_COLS], int *rideMatrixArray) {

    // finds number of permutations for the length of the array (without the first and last element)
    int numPermutations = factorial(length - 2); 
    
    // dynamic array that will hold the calculated times for each array
    int* storage = (int*)malloc(numPermutations * sizeof(int));

    int count = 0;

    // I is used to keep track of the current index used during the permutation generation 

    // s is used as a counter move through the array and control permutation generation 
    int I[length - 3], s;

    for (s = 0; s < length - 3; s++) { 
        I[s] = 0;
    }  
    s = length - 4; 

    printf("\nAttractions: ");
    printArray(arr, length);
    printf("\n");

    // calculate and store the total time for the initial arrangement (without the first and last elements)
    storage[count] = lin(arr, length, key, startTime, adjustedMatrix, waitMatrix, rideMatrixArray);
    // printf("Index: %d, Time: %d\n", count, storage[count]);
    count++;

    int index = 1;
    while (s >= 0) {

        // swaps within the valid range  
        if (I[s] < length - s - 3) {

            // increments the current index to progress through the array 
            I[s]++;
            
            // checks whether the number of elements being swapped is odd
            if ((length - s - 2) % 2 != 0) { 

                // swaps while excluding first and last element
                swap(&arr[s + 1], &arr[length - 2]); 
            } else {
                // if even swap the ith element and the last element 
                swap(&arr[s + 1], &arr[length - I[s] - 1]); 

                // ^ repeat this until i is less than n
            }

            // Calculate and store the total time for the new arrangement (without the first and last elements)
            storage[index] = lin(arr, length, key, startTime, adjustedMatrix, waitMatrix, rideMatrixArray);
            // printf("Index: %d, Time: %d\n", index, storage[index]);
            index++;

            // Increment count
            count++;

            // reset s 
            s = length - 4;
        } else {

            // if the current index has reached its maximum value, reset it and move to the previous index 
            I[s] = 0;
            s = s - 1;
        }
    
    }

    // print number of permutations 
    printf("\nPermutations: %d\n\n", count);


    // packages the array of times and the number of permutations, so we can use both later
    struct StorageResult result;
    result.storageArray = storage;
    result.count = count;

    return result;
}


// finds the max element in the array 
int max(int arr[], int length) {
    int largest = arr[0]; 

    for (int i = 1; i < length; i++) {
        if (arr[i] > largest) {
            largest = arr[i];
        }
    }

    return largest;
}

// finds the min element in the array 
int min(int arr[], int length) {
    int smallest = arr[0]; 

    for (int i = 1; i < length; i++) {
        if (arr[i] < smallest) {
            smallest = arr[i];
        }
    }

    return smallest;
}

// function used for quicksort 
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

int main() {
    int arr[] = {37,103,104,20,15,95,111,22,7,113,112,37};
    int length = sizeof(arr) / sizeof(arr[0]);

    // reads json file so we can use it 
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

        unsigned long** adjustedMatrix = createMatrix(maxAttraction, maxAttraction, distanceMatrix, key, rows);
        
        struct StorageResult result = brheap_nonrecur(attractionLabels, rows, key, startTime, adjustedMatrix, waitMatrix, rideMatrixArray);

        int* storage_list = result.storageArray;
        int count = result.count;

        int largest = max(storage_list, count);
        int smallest = min(storage_list, count);

        struct HashTable* hashTable = createHashTable(count);

        // Traverse the storage list and update the hash table
        for (int i = 0; i < count; i++) {
            int number = storage_list[i];
            insert(hashTable, number);
        }

        // Print unique numbers and their counts from the hash table
        printHash(hashTable);

        free(hashTable->table);
        free(hashTable);



        // struct pairs* storage = (struct pairs*)malloc(count * sizeof(struct pairs));

        // // printf("Storage List (total time per permutation): ");
        // // for (int i = 0; i < count; i++) {
        // //     printf("%d ", *(storage_list + i));
        // // }
        // // printf("\n\n");
        
        // // puts the storage list in ascending order 
        // qsort(storage_list, count, sizeof(int), cmpfunc); 

        // // printf("\nStorage List Sorted: ");
        // // for (int i = 0; i < count; i++) {
        // //     printf("%d ", *(storage_list + i));
        // // }

        // int index_str = 0;
        // int count_str = 0;

        // for (int i = 0; i < count; i++) {
        //     for (int j = 0; j < count; j++) {
        //         if (*(storage_list + index_str) == storage_list[j]) {
        //             count_str += 1;
        //         }
        //     }
        //     storage[i].number = *(storage_list + index_str);
        //     storage[i].count = count_str;

        //     count_str = 0; 
        //     index_str++;
        // }

        // // for (int i = 0; i < count; i++) {
        // //     printf("%d ", storage[i].number);
        // // }
        // // printf("\n"); 

        // struct pairs* unique = (struct pairs*)malloc(count * sizeof(struct pairs));
        // int uniqueCount = 0; // Use a separate variable to keep track of unique elements count

        // for (int i = 0; i < count; i++) {
        //     int matches = 0;

        //     for (int j = 0; j < uniqueCount; j++) {
        //         if (storage[i].number == unique[j].number && storage[i].count == unique[j].count) {
        //             matches += 1; 
        //         }
        //     }

        //     if (matches == 0) {
        //         unique[uniqueCount].number = storage[i].number;
        //         unique[uniqueCount].count = storage[i].count;
        //         uniqueCount++; 
        //     }
        // }


        // // printf("Unique List: ");
        // // for (int i = 0; i < uniqueCount; i++) {
        // //     printf("%d ", unique[i].number);
        // // }
        // // printf("\n");

        // // printf("%d\n", uniqueCount);

        // for (int i = 0; i < uniqueCount; i++) {
        //     // if ((unique[i].number <= largest) && (unique[i].number >= smallest) && (unique[i].number != 0)) {
        //         printf("%d, %d\n", unique[i].number, unique[i].count);
        //     // }
        // }
        // printf("\n");

        // free(storage_list);
        // free(unique);
    
    }
    
    return 0;
}