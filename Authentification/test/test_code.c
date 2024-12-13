#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/code.h"

consent_code_t* test_generate_random_code()
{
    	consent_code_t *code = (consent_code_t *) malloc(MAX_CODE_LEN + 1);  // Allocate memory dynamically for the code
    	uint8_t *numerical_code = (uint8_t *) calloc(MAX_CODE_LEN, sizeof(uint8_t));

    	if (numerical_code == NULL)
    	{
        	fprintf(stderr, "Error allocating memory for numerical code.\n");
    		free(numerical_code);
		return NULL;
    	}
    	if (code == NULL)
    	{
		fprintf(stderr, "Error allocating memory for code.\n");
		free(code);
		return NULL;
    	}

    	memset(numerical_code, 0, MAX_CODE_LEN);
    	srand(time(NULL));

    	for (int i = 0; i < MAX_CODE_LEN; i++)
    	{
        	numerical_code[i] = (rand() % 100) / 10;
		code[i] = '0' + numerical_code[i];
	}

	code[MAX_CODE_LEN] = '\0';

	fprintf(stdout, "The randomly generated code is : %s\n", code);

	free(numerical_code);

	return code;
}

int main (int argc, char * argv[])
{
    	consent_code_t* generated_code = test_generate_random_code();
    	if (generated_code != NULL)
    	{
        	free(generated_code);
    	}

    	return 0;
}
