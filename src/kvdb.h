#include <sys/stat.h>

#define KEY_MAX_LENGTH 25
#define TIMESTAMP_LENGTH 24
#define VALUE_MAX_LENGTH 1000
#define NULL_TIMESTAMP "000-00-00 00:00:00.000"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define ISALIVE "ALIV"
#define ISDEAD "DEAD"

typedef struct record {
    char is_alive[5];
    char init_ts[TIMESTAMP_LENGTH];
    char curr_ts[TIMESTAMP_LENGTH];
    char key[KEY_MAX_LENGTH];
    char value[VALUE_MAX_LENGTH];
} record;

extern char* get_curr_ts();
extern int compact_db(char* dbpath);
extern int del_key(char* key, char* dbpath);
extern int get_key(char* key, char* dbpath);
extern int scan_db_record(char* key, char* dbpath, record* rec);
extern int set_key(char* key, char* value, char* dbpath);
extern int ts_key(char* key, char* dbpath);
extern void init_db_record(record* rec);

// error values
enum {
    ERR_NOERR,
    ERR_BADARGS,
    ERR_BADCMD,
    ERR_FILESYS,
    ERR_BADKEY,
    ERR_BADVALUE,
    ERR_SEMAPHORE,
    ERR_KEYNOTFOUND
};
