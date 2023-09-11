#include <stdio.h>
#include <strings.h>
#include <time.h>

#include "kvdb.h"

void lock_db();
void unlock_db();
int set_key(char* key, char* value);
int get_key(char* key);
int del_key(char* key);
int ts_key(char* key);

char* get_timestamp()
{
    // TODO -- swap this out for something that'll give extra 3 decimal points.
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    puts(buffer);
    printf("\n%s\n", buffer);

    return 0;
}

int main(int argc, char *argv[])
{
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

    // invoke correct handler function
    if (strcasecmp(cmd, "get") == 0) {
        int err = get_key(key);
        return err;
    } else if (strcasecmp(cmd, "set") == 0) {
        int err = set_key(key, value);
        return err;
    } else if (strcasecmp(cmd, "del") == 0) {
        int err = del_key(key);
        return err;
    } else if (strcasecmp(cmd, "ts") == 0) {
        int err = ts_key(key);
        return err;
    } else {
        printf("Unknown command. Accepted commands are: get, set, del, ts.\n");
        return  ERR_BADCMD; 
    }
}
