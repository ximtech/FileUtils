# FileUtils

[![tests](https://github.com/ximtech/FileUtils/actions/workflows/cmake-ci.yml/badge.svg)](https://github.com/ximtech/FileUtils/actions/workflows/cmake-ci.yml)
[![codecov](https://codecov.io/gh/ximtech/FileUtils/branch/main/graph/badge.svg?token=PrwzGetfuh)](https://codecov.io/gh/ximtech/FileUtils)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/605a7cfb6be540d2931b6a69024e05a2)](https://app.codacy.com/gh/ximtech/FileUtils/dashboard)


C library provides method to manipulates files like moving, opening, checking existence, reading of file etc.

### Dependencies

- [CRC](https://github.com/ximtech/CRC)
- [BufferString](https://github.com/ximtech/BufferString)
- [Collections](https://github.com/ximtech/Collections)


### Features

- No static or dynamic memory allocations
- No need to free resources
- Easy and safe file manipulation
- Lightweight design
- Single include
- Easy to use


### Tradeoffs

- External dependencies
- Thread safety

### Add as CPM project dependency

How to add CPM to the project, check the [link](https://github.com/cpm-cmake/CPM.cmake)

```cmake
CPMAddPackage(
        NAME FileUtils
        GITHUB_REPOSITORY ximtech/FileUtils
        GIT_TAG origin/main)

target_link_libraries(${PROJECT_NAME} FileUtils)
```

```cmake
add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})
# For Clion STM32 plugin generated Cmake use 
target_link_libraries(${PROJECT_NAME}.elf FileUtils)
```

## Usage

### Single header include

```c
#include "FileUtils.h"
```

### Base `File` struct creation

***NOTE:*** The file or directory will not be created itself, only struct initialization

```c
File *myFile = NEW_FILE("my_file.txt");   // Creates File type struct with file name
assert(myFile != NULL);

File *rootDir = NEW_FILE("root/");    // Path separators for Unix: '/' or Win: '\\' will be auto resolved
File *subDir = FILE_OF(rootDir, "/sub/dir");   // Creates: /root/sub/dir
File *fileInSubDir = FILE_OF(subDir, myFile->path);     // Creates: /root/sub/dir/my_file.txt
```

### Create File or Directory

Works same with absolute path like: `C:/Users/Usr/Desktop/Project/target`

```c
File *myFile = NEW_FILE("my_file.txt");
assert(createFile(myFile));    // returns 'false' if file creation is failed

File *myDir = NEW_FILE("dir");
assert(MKDIR(myDir->path) == 0);

File *subDirs = NEW_FILE("dir1/dir2/dir3");
assert(createSubDirs(subDirs));    // creates all sub directories: "dir1/dir2 ..."
assert(MKDIR(subDirs->path) == 0);   // create: "/dir3"

File *fileWithDir = NEW_FILE("sub1/sub2/sub3/file.txt");
assert(createFileDirs(fileWithDir));   // create file sub directories: "sub1/sub2/sub3/"
assert(createFile(fileWithDir));       // create file itself: "file.txt"
```

### Check that File or Directory exist

```c
// Check file
File *myFile = NEW_FILE("my_file.txt");
assert(isFileExists(myFile) == false); // file not created yet
assert(createFile(myFile));

assert(isFileExists(myFile));  // file exist
assert(isFile(myFile));

// Check directory
File *myDir = NEW_FILE("dir");
assert(isDirExists(myFile) == false); // directory not created yet
assert(MKDIR(myDir->path) == 0);

assert(isDirExists(myDir));  // directory exist
assert(isDirectory(myDir));
```

### File and parent name
```c
File *fileWithDir = NEW_FILE("sub1/sub2/sub3/file.txt");
BufferString *name = EMPTY_STRING(32);

// Extract file name
getFileName(fileWithDir, name);
printf("[%s]\n", name->value);  // [file.txt]
clearString(name);

// Extract sub directories
getParentName(fileWithDir, name);
printf("[%s]\n", name->value);  // [sub1\sub2\sub3]
```

#### Even shorter
```c
BufferString *name = getFileName(fileWithDir, EMPTY_STRING(32));
BufferString *parentName = getParentName(fileWithDir, EMPTY_STRING(32));
```

### Find files within a given directory and its subdirectories
```c
File *fileWithDir = NEW_FILE("sub1/file.txt");
createFileDirs(fileWithDir);
createFile(fileWithDir);

File *nextFile = NEW_FILE("sub1/file_2.txt");
createFile(nextFile);   // directories already created

File *subDir = NEW_FILE("sub1/sub2");
MKDIR(subDir->path);

fileVector *vec = NEW_VECTOR_16(file);
File *dir = PARENT_FILE(fileWithDir);

// Find only files
listFiles(dir, vec, false);    // set flag to 'true' for recursive walk within directories

for (int i = 0; i < fileVecSize(vec); i++) {
    printf("[%s]\n", fileVecGet(vec, i).path);
}
```

#### Output:
```text
[sub1\file.txt]
[sub1\file_2.txt]
```

```c
// All files and directories
fileVecClear(vec);
listFilesAndDirs(dir, vec, false);

for (int i = 0; i < fileVecSize(vec); i++) {
    printf("[%s]\n", fileVecGet(vec, i).path);
}
```

#### Output:
```text
[sub1\file.txt]
[sub1\file_2.txt]
[sub1\sub2]
```

### Clean directory, remove all contents
```c
File *rootDir = NEW_FILE("/dir"); // create root dir
MKDIR(rootDir->path);

File *subDir = FILE_OF(rootDir, "/sub");
MKDIR(subDir->path);

File *file1 = FILE_OF(rootDir, "/file_1.txt");
File *file2 = FILE_OF(subDir, "/file_2.txt");
createFile(file1);
createFile(file2);

assert(!isEmptyDir(rootDir));    // directory have files and subdirectories
cleanDirectory(rootDir); // remove all from root
assert(isEmptyDir(rootDir)); // now directory is empty
```

### Remove directory with all contents
```c
File *rootDir = NEW_FILE("/dir"); // create root dir
MKDIR(rootDir->path);

File *subDir = FILE_OF(rootDir, "/sub");
MKDIR(subDir->path);

File *file1 = FILE_OF(rootDir, "/file_1.txt");
File *file2 = FILE_OF(subDir, "/file_2.txt");
createFile(file1);
createFile(file2);

assert(!isEmptyDir(rootDir));    // directory have files and subdirectories
assert(deleteDirectory(rootDir)); // remove dir with contents
assert(!isDirExists(rootDir));
```

### Copy one file to other file
```c
File *srcFile = NEW_FILE("/file_1.txt");
createFile(srcFile);

File *destFile = NEW_FILE("/file_2.txt"); // file will be created if not exist 
assert(copyFile(srcFile, destFile)); // return true if file has been copied
```

### Copy entire directory to other directory
```c
File *srcDir = NEW_FILE("/src");
MKDIR(srcDir->path);

File *file = FILE_OF(srcDir, "/file_1.txt");
createFile(file);  // add some file to directory

File *destDir = NEW_FILE("/dest");
MKDIR(destDir->path);        // create destination directory
assert(copyDirectory(srcDir, destDir)); // copy source directory with all contents to "/dest"
```

### Check the directory for emptiness
```c
File *rootDir = NEW_FILE("/root");
MKDIR(rootDir->path);
assert(isEmptyDir(rootDir)); // empty directory

File *file = FILE_OF(rootDir, "/file_1.txt");
createFile(file);  // now directory contains one file
assert(!isEmptyDir(rootDir));    // not empty dir
```

### Move file to directory
```c
File *rootDir = NEW_FILE("/root");   // create root dir
MKDIR(rootDir->path);

File *file = NEW_FILE("/root/file.txt");
createFile(file);

// Create "new_root" directory
File *newRootDir = NEW_FILE("/new_root");
MKDIR(newRootDir->path);

assert(moveFileToDir(file, newRootDir)); // move existing file to new directory
```

### Move entire directory to other directory
```c
File *rootDir = NEW_FILE("/root");   // create root dir
MKDIR(rootDir->path);

File *subDirs = FILE_OF(rootDir, "/dir1/dir2");
createSubDirs(subDirs);
MKDIR(subDirs->path);

// Add some files
File *file1 = NEW_FILE("/root/dir1/dir2/file_1.txt");
File *file2 = NEW_FILE("/root/dir1/file_2.txt");
File *file3 = NEW_FILE("/root/file_3.txt");
createFile(file1);
createFile(file2);
createFile(file3);

// Create "new_root" directory
File *newRootDir = NEW_FILE("/new_root");
MKDIR(newRootDir->path);

assert(moveDirToDir(rootDir, newRootDir)); // move all contents from "/root" directory to "/new_root"
```

### Write chars to file
```c
File *file = NEW_FILE("/root/file.txt");
createFileDirs(file);
createFile(file);

char *data = "Some message";
uint32_t len = strlen(data);

assert(writeCharsToFile(file, data, len, false) == len);   // return byte count that was written, "false" - means to not append data
```

#### Write `BufferString` to file
```c
File *file = NEW_FILE("/root/file.txt");
createFileDirs(file);
createFile(file);

BufferString *str = NEW_STRING_64("\nHello World!!!");
assert(writeStringToFile(file, str, true) == str->length); // 'true' - append data to existing text
```

### File `file.txt` content
```text
Some message
Hello World!!!
```

### Read content from file
```c
File *file = NEW_FILE("/root/file.txt");
createFileDirs(file);
createFile(file);

char *data = "Some message";
uint32_t len = strlen(data);
writeCharsToFile(file, data, len, false);

char buffer[32] = {0};
assert(readFileToBuffer(file, buffer, 32) == len); // return written data length
assert(strcmp(data, buffer) == 0); // same content
```

### Display human-readable version of the file size

***NOTE:*** If the size is over 1GB, the size is returned as the number of whole GB, i.e. the size is rounded down to the nearest GB boundary.

```c
BufferString *str = EMPTY_STRING(16);
// Bytes
byteCountToDisplaySize(128, str);
printf("[%s]\n", stringValue(str)); // [128 bytes]
clearString(str);

// KB
byteCountToDisplaySize(128000, str);
printf("[%s]\n", stringValue(str)); // [125 KB]
clearString(str);

// MB
byteCountToDisplaySize(128000000, str); // [122 MB]
printf("[%s]\n", stringValue(str));
clearString(str);

// GB
byteCountToDisplaySize(128000000000, str);
printf("[%s]\n", stringValue(str));     // [119 GB]
clearString(str);

// TB
byteCountToDisplaySize(128000000000000, str);
printf("[%s]\n", stringValue(str));     // [116 TB]
clearString(str);
```

#### Even shorter
```c
BufferString *str = byteCountToDisplaySize(128000, EMPTY_STRING(16));
```

### Convert human-readable file size to bytes

```c
printf("[%llu]\n", displaySizeToBytes("1 TB"));         // 1,099,511,627,776 bytes
printf("[%llu]\n", displaySizeToBytes("2 gb"));         // 2,147,483,648 bytes
printf("[%llu]\n", displaySizeToBytes("  3   MB  "));   // 3,145,728 bytes
printf("[%llu]\n", displaySizeToBytes("4kb"));          // 4,096 Bytes
printf("[%llu]\n", displaySizeToBytes("555"));          // 555 Bytes
```

### Generate file checksum
```c
File *file = NEW_FILE("/root/file.txt");
createFileDirs(file);
createFile(file);

char *data = "Some message";
uint32_t len = strlen(data);
writeCharsToFile(file, data, len, false);

uint32_t fileSize = getFileSize(file);
char buffer[fileSize];
uint32_t crc32 = fileChecksumCRC32(file, buffer, fileSize);
uint16_t crc16 = fileChecksumCRC16(file, buffer, fileSize);
printf("CRC 32: [%ul]\n", crc32);   // CRC 32: [4219986347l]
printf("CRC 16: [%u]\n", crc16);   // CRC 16: [53423l]
```
