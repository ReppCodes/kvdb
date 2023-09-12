# kvdb
A simple key-value database in C. Built as a take-home for Canonical.

# Requirements
Design and develop a simple key-value database. Running `make` in your project's root
folder should produce a binary `kvdb` that at least implements the following commands.
```sh
kvdb set {key} {value} : Associates `key` with `value`.
kvdb get {key} : Fetches the value associated with `key`.
kvdb del {key} : Removes `key` from the database.
kvdb ts {key} : Returns the timestamp when `key` was first set and when it was
last set.
```

Expected timestamp format is "YYYY-MM-DD HH:MM:SS.SSS".
Rules:
* Your program should be written in C.
* Assume that your database can be accessed by multiple processes concurrently.
* Keep performance and storage efficiency in mind.
* You are expected to not spend much more than 4 hours on this project.
* You are allowed to use libraries, but don't just write a wrapper around an existing database.
* We will not install extra dependencies to compile your program.
* Your program will be compiled with gcc 11.2.0 on a x86_64 machine running Ubuntu Jammy Jellyfish.
* Please provide a short write-up of the limitations of your program. (20

# Design Choices
* Prioritize simplicity and stability. This will impose both feature and performance limitations,
  but gotta timebox something out.
* Use a liveness field for delete, just mark it to 0 for "delete" op.
    * Garbage collection op could be "future work", would be easy enough as its own op.
* Use a semaphore for locking the db during operations
* Write data to/from file in current directory
* keys and values all treated as ASCII text

# Data format
```
[isalive][key length][key][value length][value][initial timestamp][most recent timestamp]
```

# Limitations
* Places database file into current working directory
* File is append-only, future "gc" command would be for compaction
* Max key length = 25
* Max value length = 1000
* Both key and value treated as text
