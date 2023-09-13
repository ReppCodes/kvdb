#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "kvdb.h"

char* get_curr_ts()
{
    struct timeval timestamp;
    time_t timestamp_sec;
    struct tm* timestamp_struct;
    char* buffer = (char*)malloc(TIMESTAMP_LENGTH);

    gettimeofday(&timestamp, NULL);
    timestamp_sec = timestamp.tv_sec;
    timestamp_struct = localtime(&timestamp_sec);

    int offset = strftime(buffer, TIMESTAMP_LENGTH, "%Y-%m-%d %H:%M:%S", timestamp_struct);
    char* microsec_str;
    sprintf(microsec_str, "%ld", timestamp.tv_usec);
    strcat(buffer, ".");
    strncat(buffer, microsec_str, 3);

    return buffer;
}

void init_db_record(record* rec)
{
    // null initialize all fields of record
    strcpy(rec->is_alive, ISDEAD);
    strcpy(rec->init_ts, NULL_TIMESTAMP);
    strcpy(rec->curr_ts, NULL_TIMESTAMP);
    memset(rec->key, 0, KEY_MAX_LENGTH);
    memset(rec->value, 0, VALUE_MAX_LENGTH);
}

int scan_db_record(char* key, char* dbpath, record* rec)
{
    int err;
    FILE* db;
    int offset = 0;

    // null initialize db record before looking for actual data
    // we rely on safe null values for access and for error handling
    init_db_record(rec);

    db = fopen(dbpath, "r");
    if (db == NULL) {
        printf("Error opening database\n");
        err = ERR_FILESYS;
        goto cleanup;
    }

    while (1) {
        size_t scanval = fread(rec, sizeof(record), 1, db);
        if (scanval == 0) {
            break;
        }

        if (strcmp(rec->is_alive, ISALIVE) != 0 || strcmp(key, rec->key) != 0) {
            init_db_record(rec);
            offset += sizeof(record);
            continue;
        }

        // found a live match for our key. leave it populating the struct, and
        // return the offset we found it at.
        break;
    }
cleanup:
    if (db != NULL) {
        fclose(db);
    }
    return offset;
}

int get_key(char* key, char* dbpath)
{
    int err = ERR_NOERR;
    FILE* db;
    record* rec = (record*)malloc(sizeof(record));
    scan_db_record(key, dbpath, rec);
    if (strcmp(rec->is_alive, ISALIVE) == 0) {
        printf("%s\n", rec->value);
    } else {
        printf("Key not found in database\n");
    }
    free(rec);
    return err;
}

int del_key(char* key, char* dbpath)
{
    record* rec = (record*)malloc(sizeof(record));
    int err = ERR_NOERR;
    char* deadval = ISDEAD;
    int offset = scan_db_record(key, dbpath, rec);

    if (strcmp(rec->is_alive, ISALIVE) == 0) {
        FILE* db = fopen(dbpath, "r+");
        fseek(db, offset, SEEK_SET);
        fwrite(deadval, 5, 1, db);
    } else {
        printf("Key not found in databse\n");
        err = ERR_KEYNOTFOUND;
    }
    free(rec);
    return err;
}

int ts_key(char* key, char* dbpath)
{
    int err = ERR_NOERR;
    FILE* db;
    record* rec = (record*)malloc(sizeof(record));
    scan_db_record(key, dbpath, rec);
    if (strcmp(rec->is_alive, ISALIVE) == 0) {
        printf("First timestamp: %s\n", rec->init_ts);
        printf("Last timestamp: %s\n", rec->curr_ts);
    } else {
        printf("Key not found in database\n");
        err = ERR_KEYNOTFOUND;
    }
    free(rec);
    return err;
}

int set_key(char* key, char* value, char* dbpath)
{
    int err = ERR_NOERR;
    int del_old_record;

    // bounds checks
    if (strlen(key) > KEY_MAX_LENGTH || strlen(key) == 0) {
        printf("Key kength must be between 1 and 25, inclusive\n");
        return ERR_BADKEY;
    } else if (strlen(value) > VALUE_MAX_LENGTH || strlen(value) == 0) {
        printf("Value kength must be between 1 and 1000, inclusive\n");
        return ERR_BADVALUE;
    }

    // check for old record, both to delete and for init_ts
    char* init_ts;
    char* curr_ts = get_curr_ts();
    record* old_record = (record*)malloc(sizeof(record));
    scan_db_record(key, dbpath, old_record);
    if (strcmp(old_record->is_alive, ISALIVE) == 0) {
        del_old_record = 1;
        init_ts = old_record->init_ts;
    } else {
        init_ts = curr_ts;
        del_old_record = 0;
    }

    // populate current record and write to end of file
    record* rec = (record*)malloc(sizeof(record));
    init_db_record(rec);
    strcpy(rec->is_alive, ISALIVE);
    strcpy(rec->init_ts, init_ts);
    strcpy(rec->curr_ts, curr_ts);
    strcpy(rec->key, key);
    strcpy(rec->value, value);

    FILE* db = fopen(dbpath, "a");
    if (db != NULL) {
        size_t bytes = fwrite(rec, sizeof(record), 1, db);
        if (bytes < 0) {
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
    if (curr_ts != NULL) {
        free(curr_ts);
        curr_ts = NULL;
    }
    if (old_record != NULL) {
        free(old_record);
    }

    if (rec != NULL) {
        free(rec);
    }
    return err;
}

int main(int argc, char* argv[])
{
    int err;

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
    char cwd[512];
    getcwd(cwd, sizeof(cwd));
    char dbname[7] = "/kv.db\0";
    char dbpath[519];
    strcpy(dbpath, cwd);
    strcat(dbpath, dbname);

    // create the file if it doesn't exist, simplifies downstream fopen calls.
    FILE* db = fopen(dbpath, "a");
    fclose(db);

    // for POC, just use single db-wide lock to be safe for multiprocessing
    sem_t* db_sem;
    db_sem = sem_open("kvdb_sem", O_CREAT, SEM_PERMS, 1);
    if (db_sem == SEM_FAILED) {
        printf("Unable to acquire lock on database\n");
        return ERR_SEMAPHORE;
    }
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

    // close down and clean up semaphore
    if (sem_post(db_sem) < 0) {
        printf("Error posting to semaphore\n");
        err = ERR_SEMAPHORE;
    } else if (sem_close(db_sem) < 0) {
        printf("Error closing semaphore\n");
        err = ERR_SEMAPHORE;
    } 

    // all processes will call unlink, but only needs to be called once. doesn't hurt to call
    // repeatedly.
    sem_unlink("kvdb_sem");
    return err;
}
