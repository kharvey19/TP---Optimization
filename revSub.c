#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX_TOUR 15

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

int main() {
    int tour[] = {37, 4, 56, 34, 26, 87, 103, 45, 32, 37};
    int tourLength = sizeof(tour) / sizeof(tour[0]);
    int newTour[MAX_TOUR];

    srand(time(NULL)); 

    int p1 = (rand() % (tourLength - 2)) + 1;
    int p2 = p1;
    while (p2 == p1) {
        p2 = (rand() % (tourLength - 2)) + 1;
    }

    printf("\n\nOriginal Tour: ");
    for (int i = 0; i < tourLength; i++) {
        printf("%d ", tour[i]);
    }
    printf("\n\n");

    flip(tour, newTour, p1, p2, tourLength);

    printf("Flipped Tour: ");
    for (int i = 0; i < tourLength; i++) {
        printf("%d ", newTour[i]);
    }
    printf("\n\n");

    shuffleArray(tour, tourLength);

    printf("Shuffled Tour: ");
    for (int i = 0; i < tourLength; i++) {
        printf("%d ", tour[i]);
    }
    printf("\n\n");
    return 0;
}
