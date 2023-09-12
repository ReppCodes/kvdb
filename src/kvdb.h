#include <stdbool.h>
#include <sys/stat.h>

#define KEY_MAX_LENGTH 25
#define TIMESTAMP_LENGTH 26
#define VALUE_MAX_LENGTH 1000
#define NULL_TIMESTAMP "000-00-00 00:00:00"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

typedef struct record {
    bool is_alive;
    char init_ts[TIMESTAMP_LENGTH];
    char curr_ts[TIMESTAMP_LENGTH];
    int key_length;
    char key[KEY_MAX_LENGTH];
    int value_length;
    char value[VALUE_MAX_LENGTH];
} record;

extern bool is_valid_ts(char* timestamp);
extern char* get_curr_ts();
extern int del_key(char* key, char* dbpath);
extern int get_key(char* key, char* dbpath);
extern int set_key(char* key, char* value, char* dbpath);
extern int ts_key(char* key, char* dbpath);
extern void lock_db();
extern void scan_db_record(char* key, char* dbpath, record* rec);
extern void init_db_record(record* rec);
extern void unlock_db();

// error values
enum {
    ERR_NOERR,
    ERR_BADARGS,
    ERR_BADCMD,
    ERR_FILESYS,
    ERR_BADKEY,
    ERR_BADVALUE,
    ERR_SEMAPHORE
};
