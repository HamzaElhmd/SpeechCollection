#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mongoc.h>
#include "../include/code.h"

char *clidb_uri = "mongodb://127.0.0.1:27017";

// Allocating memory for entry
entry_t* entry_alloc() {
    entry_t *entry = (entry_t *) malloc(sizeof(entry_t));
    if (entry == NULL) {
        perror("ERROR : Memory allocation failed for emailer entry.\n");
        return NULL;
    }
    return entry;
}

// Freeing memory for entry
void entry_destroy(entry_t *entry) {
    if (entry == NULL) {
        perror("ERROR : Invalid value for entry => null.\n");
        return;
    }
    free(entry);
}

// Setting values in entry
int8_t entry_set(entry_t *entry, const char *code, const char *email, const int8_t code_s) {
    if (entry == NULL || code == NULL || email == NULL) {
        fprintf(stderr, "ERROR : Invalid arguments to set entry.\n");
        return 1;
    }

    size_t code_size = strlen(code);
    size_t email_size = strlen(email);
    if (code_size > MAX_CODE_LEN || email_size > EMAIL_SIZE) {
        fprintf(stderr, "ERROR : Code or email overflow.\n");
        return 1;
    }

    strncpy(entry->code, code, MAX_CODE_LEN);
    //entry->code[MAX_CODE_LEN - 1] = '\0';
    strncpy(entry->email, email, email_size + 1);
    //entry->email[EMAIL_SIZE - 1] = '\0';

    if (code_s == ACCESS_S || code_s == PENDING_S) {
        entry->code_s = code_s;
    } else {
        fprintf(stderr, "ERROR : Invalid status code.\n");
        return 1;
    }

    return 0;
}

// Function to insert an entry into MongoDB
int8_t insert_code(const char *email_addr, const char *consent_code) {
    mongoc_client_t *auth_cli;
    mongoc_database_t *sc_db;
    mongoc_collection_t *code_coll;
    bson_t *doc = bson_new(); // Initialize doc
    bson_t *response = bson_new(); // Initialize response
    bson_error_t error; // Initialize error
    int8_t rc = 0;
    char json_str[1024];

    auth_cli = mongoc_client_new(clidb_uri);
    if (auth_cli == NULL) {
        fprintf(stderr, "ERROR : Invalid mongoDB client uri .\n");
        return 1;
    }

    sc_db = mongoc_client_get_database(auth_cli, "speechCollector");
    if (sc_db == NULL) {
        fprintf(stderr, "ERROR : Failed to get speechCollector database.\n");
        rc = 1;
        goto cleanup;
    }

    code_coll = mongoc_database_get_collection(sc_db, "consentCode");
    if (code_coll == NULL) {
        fprintf(stderr, "ERROR : Failed to get consentCode collection.\n");
        rc = 1;
        goto cleanup;
    }

    // Prepare the JSON string for MongoDB insertion
    sprintf(json_str, "{\"consentCode\": \"%s\", \"email address\": \"%s\"}", consent_code, email_addr);

    // Initialize doc from JSON string
    if (!bson_init_from_json(doc, json_str, strlen(json_str), &error)) {
        fprintf(stderr, "ERROR : Parsing string to JSON: %s\n", error.message);
        rc = 1;
        goto cleanup;
    }

    // Insert document into MongoDB
    if (!mongoc_collection_insert_one(code_coll, doc, NULL, response, &error)) {
        fprintf(stderr, "Error : Can't insert BSON for the following reason: %s\n", error.message);
        rc = 1;
        goto cleanup;
    }
	
    fprintf(stdout, "INFO : %s\n", bson_as_canonical_extended_json(response, NULL));

cleanup:
    // Cleanup resources
    mongoc_database_destroy(sc_db);
    bson_destroy(doc);
    bson_destroy(response);
    mongoc_collection_destroy(code_coll);
    mongoc_client_destroy(auth_cli);

    return rc;
}

// Generate a random consent code
consent_code_t* generate_random_code() {
    entry_t *entry;
    consent_code_t *code = (consent_code_t *) malloc(MAX_CODE_LEN + 1);
    uint8_t *numerical_code = (uint8_t *) calloc(MAX_CODE_LEN, sizeof(uint8_t));

    if (numerical_code == NULL || code == NULL) {
        fprintf(stderr, "Error allocating memory for numerical code or consent code.\n");
        free(numerical_code);
        free(code);
        return NULL;
    }

    memset(numerical_code, 0, MAX_CODE_LEN);
    srand(time(NULL));

    for (int i = 0; i < MAX_CODE_LEN; i++) {
        numerical_code[i] = (rand() % 100) / 10;
        code[i] = '0' + numerical_code[i];
    }

    code[MAX_CODE_LEN] = '\0';

    fprintf(stdout, "The randomly generated code is: %s\n", code);

    entry = entry_alloc();
    if (entry == NULL) {
        free(code);
        free(numerical_code);
        return NULL;
    }

    entry_set(entry, code, " ", 4);

    fprintf(stdout, "Successfully generated consent code.\n");

    entry_destroy(entry);
    free(numerical_code);

    return code;
}

// Placeholder for validating the consent code
int8_t validate_consent_code(const consent_code_t *code, const char *email_addr) {
    mongoc_client_t *auth_cli;
    mongoc_database_t *sc_db;
    mongoc_collection_t *code_coll;
    mongoc_cursor_t *cursor = NULL;
    const bson_t *response = NULL;
    bson_t *doc = bson_new(); // Initialize doc
    bson_error_t error; // Initialize error
    int8_t rc = 0;
    char json_str[1024];

    auth_cli = mongoc_client_new(clidb_uri);
    if (auth_cli == NULL) {
        fprintf(stderr, "ERROR : Invalid mongoDB client uri .\n");
        return 1;
    }

    sc_db = mongoc_client_get_database(auth_cli, "speechCollector");
    if (sc_db == NULL) {
        fprintf(stderr, "ERROR : Failed to get speechCollector database.\n");
        rc = 1;
        goto cleanup;
    }

    code_coll = mongoc_database_get_collection(sc_db, "consentCode");
    if (code_coll == NULL) {
        fprintf(stderr, "ERROR : Failed to get consentCode collection.\n");
        rc = 1;
        goto cleanup;
    }

    // Prepare the JSON string for MongoDB insertion
    sprintf(json_str, "{\"consentCode\": \"%s\", \"email address\": \"%s\"}", code, email_addr);

    // Initialize doc from JSON string
    if (!bson_init_from_json(doc, json_str, strlen(json_str), &error)) {
        fprintf(stderr, "Error parsing string to JSON: %s\n", error.message);
        rc = 1;
        goto cleanup;
    }

    // Insert document into MongoDB
    cursor = mongoc_collection_find_with_opts(code_coll, doc, NULL, NULL);

    if (!cursor) {
	fprintf(stderr, "ERROR: Failed to create cursor.\n");
	rc = 1;
	goto cleanup;
    }

    if (!mongoc_cursor_next(cursor, &response)) {
    	fprintf(stderr, "WARNING : Collection is empty or query error.\n");
	rc = 2;
	goto cleanup;
    }

    char *str = bson_as_canonical_extended_json(doc, NULL);
    fprintf(stdout, "INFO : Consent code %s from %s exists!\nINFO : %s\n", code, email_addr, str);

cleanup:
    // Cleanup resources
   if (cursor) mongoc_cursor_destroy(cursor);
    mongoc_database_destroy(sc_db);
    bson_destroy(doc);
    mongoc_collection_destroy(code_coll);
    mongoc_client_destroy(auth_cli);

    return rc;
}

