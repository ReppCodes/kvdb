#define KEY_MAX_LENGTH 100
#define VALUE_MAX_LENGTH 10000

typedef struct entry {
    char key[KEY_MAX_LENGTH];
    char value[VALUE_MAX_LENGTH];
} entry;

extern void lock_db();
extern void unlock_db();
extern int set_key(char* key, char* value);
extern int get_key(char* key);
extern int del_key(char* key);
extern int ts_key(char* key);

// error values
enum {
    NOERR,
    ERR_BADARGS,
    ERR_BADCMD
};
