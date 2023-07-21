#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>

#define RESULT_ERROR (-1)

long getSize(char *filename) {
    struct stat file_status;
    if (stat(filename, &file_status) < 0) {
        return RESULT_ERROR;
    }
    return file_status.st_size;
}

int main() {
    char *filename = "useCase.json";
	// open the file
	FILE *fp = fopen("useCase.json", "r");
	if (fp == NULL) {
		printf("Error: Unable to open the file.\n");
		return 1;
	}

	// read the file contents into a string
	char buffer[getSize(filename)];
	fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);

	// parse the JSON data
	cJSON *json = cJSON_Parse(buffer);
	if (json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			printf("Error: %s\n", error_ptr);
		}
		cJSON_Delete(json);
		return 1;
	}

	// access the JSON data
	cJSON *attractions = cJSON_GetObjectItemCaseSensitive(json, "AttractionsToInclude");

    if (cJSON_IsArray(attractions)) {
        cJSON *i;
        cJSON_ArrayForEach(i, attractions) {
            printf("%s\n", strtok(i-> valuestring, "HS"));
        }
        
    }

	// delete the JSON object
	cJSON_Delete(json);
	return 0;
}
