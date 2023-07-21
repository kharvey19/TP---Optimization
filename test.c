#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>

int main() {
	// open the file
	FILE *fp = fopen("use.json", "r");
	if (fp == NULL) {
		printf("Error: Unable to open the file.\n");
		return 1;
	}

	// read the file contents into a string
	char buffer[1024];
	int len = fread(buffer, 1, sizeof(buffer), fp);
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
