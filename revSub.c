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

    // Copy elements before the flip section
    for (int i = 0; i < frontFillIndex; i++) {
        newTour[i] = tour[i];
    }

    // Copy elements in reverse order for the flip section
    for (int i = 0; i <= backFillIndex - frontFillIndex; i++) {
        newTour[frontFillIndex + i] = tour[backFillIndex - i];
    }

    // Copy elements after the flip section
    for (int i = backFillIndex + 1; i < tourLength; i++) {
        newTour[i] = tour[i];
    }
}

int main() {
    int tour[] = {37, 4, 56, 34, 26, 87, 103, 45, 32, 37};
    int tourLength = sizeof(tour) / sizeof(tour[0]);
    int newTour[MAX_TOUR];

    srand(time(NULL)); 

    int p1 = rand() % tourLength;
    int p2 = p1;
    while (p2 == p1) {
        p2 = rand() % tourLength;
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

    return 0;
}
