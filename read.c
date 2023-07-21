#include <stdio.h>
#include <cjson/cJSON.h>

/*
    In C, read the JSON object into memory.  Use the cJSON library to validate 
    and parse the JSON (https://github.com/DaveGamble/cJSON). 
    
    There's an array element in the JSON called "AttractionsToInclude", and it 
    looks like this: "AttractionsToInclude": ["HS37", "HS104", "HS111", "HS112", 
    "HS07", "HS103", "HS15", "HS113", "HS20", "HS22", "HS95", "HS37"].  Have your 
    C program print out just the numbers in that array, in the order shown in 
    AttractionsToInclude.  So for "HS37", print the integer 37, as an example.

    Use Valgrind to ensure you're not leaking memory or doing anything outside 
    of array bounds.

    (And create a makefile for compilation, create your own repository for this 
    in github, etc.)

*/

int main() {

    FILE *fp = fopen("useCase.json", "r");
    if (fp == NULL) {
        printf("Error: Unable to open the file.\n");
        return 1;
    }

    char buffer[1024];
    int len = fread(buffer, 1, sizeof(buffer), fp);
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

    cJSON *AttractionsToInclude = cJSON_GetObjectItemCaseSensitive(json, "AttractionsToInclude");
    if (cJSON_IsString(AttractionsToInclude) && (AttractionsToInclude->valuestring != NULL)) {
        printf("AttractionsToInclude: %s\n", AttractionsToInclude->valuestring);
    }

    cJSON_Delete(json);
    return 0;

}