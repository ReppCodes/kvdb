#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <strings.h>
#include <time.h>

#include "kvdb.h"

// caller must free resulting buffer
// TODO -- swap this out for an approach that'll give extra 3 decimal points.
char* get_curr_ts()
{
    char* buffer = (char*)malloc(TIMESTAMP_LENGTH);
    time_t timer;
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, TIMESTAMP_LENGTH, "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}

static char* get_db_ts(char* key, char* dbpath)
{
    char* buffer = (char*)malloc(TIMESTAMP_LENGTH);
    record rec;
    scan_db_record(key, dbpath, &rec);
    memcpy(buffer, &rec.init_ts, 26);
    return buffer;
}

void init_db_record(record *rec)
{
    // null initialize all fields of record
    rec->is_alive = false;
    strcpy(rec->init_ts, NULL_TIMESTAMP);
    strcpy(rec->curr_ts, NULL_TIMESTAMP);
    rec->key_length = 0;
    memset(rec->key, 0, KEY_MAX_LENGTH);
    rec->value_length = 0;
    memset(rec->value, 0, VALUE_MAX_LENGTH);
}

void scan_db_record(char* key, char* dbpath, record* rec)
{
    int err;
    FILE* db;

    // null initialize db record before looking for actual data
    // we rely on safe null values for access and for error handling
    init_db_record(rec);


    db = fopen(dbpath, "r");
    if (db == NULL) {
        printf("Error writing key %s to database\n", key);
        err = ERR_FILESYS;
        goto cleanup;
    }

    while (true) {
        
    }

cleanup:
    fclose(db);
}

void lock_db();
void unlock_db();
int get_key(char* key, char* dbpath);
int del_key(char* key, char* dbpath);
int ts_key(char* key, char* dbpath);

// as a simplifying workaround, timestamps are default to all 0s, and we
// just check the leading 0 for validity.
bool is_valid_ts(char* timestamp)
{
    if (timestamp[0] == '0') {
        return false;
    } else {
        return true;
    }
}

int set_key(char* key, char* value, char* dbpath)
{
    int err = ERR_NOERR;
    bool del_old_record;

    // bounds checks
    if (strlen(key) > KEY_MAX_LENGTH || strlen(key) == 0) {
        printf("Key kength must be between 1 and 25, inclusive\n");
        return ERR_BADKEY;
    } else if (strlen(value) > VALUE_MAX_LENGTH || strlen(value) == 0) {
        printf("Value kength must be between 1 and 1000, inclusive\n");
        return ERR_BADVALUE;
    }
  
    
    char* init_ts = get_db_ts(key, dbpath);
    char* curr_ts = get_curr_ts();
    if (is_valid_ts(init_ts)) {
        del_old_record = true;
    } else {
        free(init_ts);
        init_ts = curr_ts;
        del_old_record = false;
    }

    record* rec = (record*)malloc(sizeof(record));
    init_db_record(rec);
    rec->is_alive = true;
    strcpy(rec->init_ts, init_ts);
    strcpy(rec->curr_ts, curr_ts);
    rec->key_length = strlen(key);
    strcpy(rec->key, key);
    rec->value_length = strlen(value);
    strcpy(rec->value, value);

    FILE* db = fopen(dbpath, "a");
    if (db != NULL) {
        if (fwrite(rec, sizeof(record), 1, db)) {
            printf("Error writing key %s to database\n", key);
            err = ERR_FILESYS;
            goto cleanup;
        }
        if (fclose(db)) {
            printf("Error closing database\n");
            err = ERR_FILESYS;
            goto cleanup;
        }
    } else {
        printf("Error opening database\n");
        err = ERR_FILESYS;
        goto cleanup;
    }

    // do this at the end, data loss is worse than duplicate keys because later
    // key is always the valid one so we can always recover.
    if (del_old_record) {
      del_key(key, dbpath);
    }

cleanup:
    free(curr_ts);
    curr_ts = NULL;
    if (init_ts) {
        free(init_ts);
    }
    free(rec);
    return err;
}

int main(int argc, char* argv[])
{
    int err;
    // for POC, just use single db-wide lock to be safe for multiprocessing
    sem_t* db_sem; 

    if (argc < 3) {
        printf("Requires at least 3 arguments, only %d provided\n", argc);
        return ERR_BADARGS;
    } else if (argc > 4) {
        printf("Accepts at most 4 arguments, %d provided\n", argc);
        return ERR_BADARGS;
    }

    char* cmd = argv[1];
    char* key = argv[2];
    char* value;
    if (argc == 4) {
        value = argv[3];
    }

    // keep this separate and passed in for better modularity in future work
    char* dbpath = "kv.db";

    db_sem = sem_open(dbpath, O_CREAT, SEM_PERMS, 1);
    if (db_sem == SEM_FAILED) {
        printf("Unable to acquire lock on database\n");
        return ERR_SEMAPHORE;
    }

    // TODO -- add timeout loop here
    if (sem_wait(db_sem) < 0) {
        printf("Unable to acquire lock on database\n");
        return ERR_SEMAPHORE;
    }


    // invoke correct handler function
    if (strcasecmp(cmd, "get") == 0) {
        err = get_key(key, dbpath);
    } else if (strcasecmp(cmd, "set") == 0) {
        err = set_key(key, value, dbpath);
    } else if (strcasecmp(cmd, "del") == 0) {
        err = del_key(key, dbpath);
    } else if (strcasecmp(cmd, "ts") == 0) {
        err = ts_key(key, dbpath);
    } else {
        printf("Unknown command. Accepted commands are: get, set, del, ts.\n");
        err = ERR_BADCMD;
    }

    if (sem_post(db_sem) < 0) {
        printf("Error releasing lock on database\n");
        err = ERR_SEMAPHORE;
    }
    return err;
}
