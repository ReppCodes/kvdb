# Limitations
* This project prioritizes simplicity and stability. This will impose both feature and performance limitations,
  but given a 4 hour budget I chose to prioritize in this way.
* The program places the database file into the current working directory. It would be pretty
  straightforward to modify it to accept a filepath as an argument but that is left for future work. 
* The file is append-only, with "delete" just marking a record dead. A future "gc" command would be
  implemented for compaction.
* The record format is fixed-width, so smaller records still take up full space. This was done to
  simplify the code for scanning through records.
* The maximum key length is 25 characters.
* The maximum value length is 1000 characters.
* Both keys and values are treated as text.

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
* Please provide a short write-up of the limitations of your program. (20 lines max).
• Hand in a tarball containing your `kvdb` root folder and the write-up.

# Design Choices
* Use a liveness field for delete, just mark it to 0 for "delete" op.
    * Garbage collection op could be "future work", would be easy enough as its own op.
* Use a semaphore for locking the db during operations
* Write data to/from file in current directory
* keys and values all treated as ASCII text

# Data format
```
[isalive][initial timestamp][most recent timestamp][key][value]
```

