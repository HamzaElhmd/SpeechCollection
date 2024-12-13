#ifndef CODE_H
#define CODE_H

#include <stdint.h>

extern char *clidb_uri;

/* maximum size of a consent code and email addr */
#define EMAIL_SIZE 255 
#define MAX_CODE_LEN 6

/* status of the code created */
#define ACCESS_S 2
#define PENDING_S 4

/* code structure as an array of characters */
typedef char consent_code_t;
typedef char email_addr_t;

/* database representation of a consent code entry */
typedef struct {
	consent_code_t code[MAX_CODE_LEN];
	email_addr_t email[EMAIL_SIZE];
	uint8_t code_s;
} entry_t;


/********************* METHODS **************************/
entry_t* entry_alloc();

void entry_destroy(entry_t *entry);

int8_t entry_set(entry_t *entry, const char *code, const char *email, const int8_t code_s);



/********************* SERVICES ************************/
/* create and insert randomly generated code to db */
consent_code_t* generate_random_code();

int8_t insert_code(const char *consent_code, const char *email_addr);

/* Validate consent code against the database */
int8_t validate_consent_code(const consent_code_t *code, const char *email_addr);

#endif
