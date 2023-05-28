#pragma once

#include "BaseTestTemplate.h"
#include "FileUtils.h"

#if defined(_WIN32) || defined(_WIN64)
    #define FROM_PATH "\\tmp"
#else
    #define FROM_PATH "/tmp/dir"
#endif

#define FILE_SEP FILE_NAME_SEPARATOR_STR

#define ROOT "C:\\Users\\Usr\\Desktop\\Projects\\dir/"

static MunitResult testNewFile(const MunitParameter params[], void *data) {
    File *file_1 = NEW_FILE("test_file.txt");
    assert_not_null(file_1);
    assert_uint32(13, ==, file_1->pathLength);
    assert_string_equal("test_file.txt", file_1->path);
    assert_null(file_1->file);
    assert_null(file_1->dir);

    File *file_2 = NEW_FILE("dir1/dir2/file.txt");
    assert_not_null(file_2);
    assert_uint32(file_2->pathLength, ==, 18);
    assert_string_equal("dir1" FILE_SEP "dir2" FILE_SEP "file.txt", file_2->path);

    File *file_3 = NEW_FILE(NULL);
    assert_null(file_3);

    File *rootDir = NEW_FILE("/dir1/dir2/");
    File *subDir = FILE_OF(rootDir, "\\dir3\\dir4");
    assert_string_equal(FILE_SEP "dir1" FILE_SEP "dir2" FILE_SEP "dir3" FILE_SEP "dir4", subDir->path);

    File *file_4 = FILE_OF(subDir, "test_4_file.txt");
    assert_string_equal(FILE_SEP "dir1" FILE_SEP "dir2" FILE_SEP "dir3" FILE_SEP "dir4" FILE_SEP "test_4_file.txt", file_4->path);

    File *file_5 = FILE_OF(subDir, file_1->path);
    assert_string_equal(FILE_SEP "dir1" FILE_SEP "dir2" FILE_SEP "dir3" FILE_SEP "dir4" FILE_SEP "test_file.txt", file_5->path);

    File *file_6 = PARENT_FILE(file_5);
    assert_string_equal(FILE_SEP "dir1" FILE_SEP "dir2" FILE_SEP "dir3" FILE_SEP "dir4", file_6->path);

    File *file_7 = PARENT_FILE(file_1);
    assert_string_equal("", file_7->path);   // no parent

    File *file_8 = FILE_OF(file_6, NULL);
    assert_null(file_8);

    File *file_9 = NEW_FILE(ROOT "/file_9.txt");
    assert_true(strstr(file_9->path, FILE_SEP FILE_SEP) == NULL);  // no multiple path separators

    return MUNIT_OK;
}

static MunitResult testCreateFileAndDir(const MunitParameter params[], void *data) {
    // Single file
    File *file_1 = NEW_FILE(FROM_PATH "/test_file.txt");
    assert_false(isFileExists(file_1));
    assert_true(createSubDirs(file_1));
    assert_true(createFile(file_1));

    assert_true(isFile(file_1));
    assert_true(isFileExists(file_1));
    remove(file_1->path);

    // Sub dirs
    File *rootDir = NEW_FILE(FROM_PATH "/dir_t_1/dir_t_2/");
    deleteDirectory(rootDir);

    File *file_2 = FILE_OF(rootDir, "test_file_2.txt");
    assert_false(isFileExists(file_2));
    assert_true(createFileDirs(file_2));
    assert_true(createFile(file_2));

    assert_true(isFile(file_2));
    assert_true(isFileExists(file_2));
    assert_false(isDirectory(file_2));
    assert_false(isDirExists(file_2));

    // Rename
    File *file_3 = NEW_FILE(FROM_PATH "test_file_xx.txt");
    assert_true(renameFileTo(file_2, file_3));
    assert_false(isFileExists(file_2));
    assert_true(isFileExists(file_3));
    remove(file_3->path);

    // Create dirs
    File *dir = NEW_FILE(FROM_PATH "/dir_1/dir_2/dir_3");
    assert_false(isDirExists(dir));
    assert_true(createSubDirs(dir));
    assert_true(MKDIR(dir->path) == 0);
    assert_true(isDirExists(dir));
    assert_true(isDirectory(dir));

    // Delete all dirs
    deleteDirectory(dir);
    assert_false(isDirExists(dir));

    return MUNIT_OK;
}

static MunitResult testFileSize(const MunitParameter params[], void *data) {
    File *file_1 = NEW_FILE(FROM_PATH "/test_size_file.txt");
    assert_true(createSubDirs(file_1));
    assert_true(createFile(file_1));
    assert_uint64(0, ==, getFileSize(file_1));

    char *message = "Some test message";
    assert_uint32(writeCharsToFile(file_1, message, strlen(message), false), ==, strlen(message));
    assert_uint64(strlen(message), ==, getFileSize(file_1));
    remove(file_1->path);

    return MUNIT_OK;
}

static MunitResult testFileNameAndParent(const MunitParameter params[], void *data) {
    File *file = NEW_FILE(FROM_PATH "/dir1/dir2/some_file.txt");
    BufferString *str = EMPTY_STRING(PATH_MAX_LEN);
    getFileName(file, str);
    assert_string_equal("some_file.txt", str->value);
    clearString(str);

    File *file_2 = NEW_FILE("some_file_2.txt");
    getFileName(file_2, str);
    assert_string_equal("some_file_2.txt", str->value);
    clearString(str);

    getParentName(file, str);
    assert_string_equal(FROM_PATH FILE_SEP "dir1" FILE_SEP "dir2", str->value);
    clearString(str);

    return MUNIT_OK;
}

static MunitResult testFileList(const MunitParameter params[], void *data) {
    File *rootDir = NEW_FILE(FROM_PATH "/dir1");
    deleteDirectory(rootDir);

    File *subDir = FILE_OF(rootDir, "/dir2");
    File *subSubDir = FILE_OF(subDir, "/dir3");
    assert_true(createSubDirs(subSubDir)); // creates all sub dirs
    assert_true(MKDIR(subSubDir->path) == 0);

    File *file_1 = FILE_OF(rootDir, "/file_1.txt");
    File *file_2 = FILE_OF(subDir, "/file_2.txt");
    File *file_3 = FILE_OF(subSubDir, "/file_3.txt");
    assert_true(createFile(file_1));
    assert_true(createFile(file_2));
    assert_true(createFile(file_3));

    fileVector *vec = NEW_VECTOR_64(file);
    listFiles(rootDir, vec, false);
    assert_uint32(fileVecSize(vec), ==, 1);
    assert_string_equal(fileVecGet(vec, 0).path, file_1->path);
    fileVecClear(vec);

    listFiles(rootDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 3);
    assert_true(fileVecContains(vec, *file_1));
    assert_true(fileVecContains(vec, *file_2));
    assert_true(fileVecContains(vec, *file_3));
    fileVecClear(vec);

    listFilesAndDirs(rootDir, vec, false);
    assert_uint32(fileVecSize(vec), ==, 2);
    assert_true(fileVecContains(vec, *subDir));
    assert_true(fileVecContains(vec, *file_1));
    fileVecClear(vec);

    listFilesAndDirs(rootDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 5);
    assert_true(fileVecContains(vec, *subDir));
    assert_true(fileVecContains(vec, *subSubDir));
    assert_true(fileVecContains(vec, *file_1));
    assert_true(fileVecContains(vec, *file_2));
    assert_true(fileVecContains(vec, *file_3));
    fileVecClear(vec);

    cleanDirectory(rootDir);
    listFilesAndDirs(rootDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 0);
    assert_true(isEmptyDir(rootDir));

    assert_true(deleteDirectory(rootDir));
    assert_false(isDirExists(rootDir));

    return MUNIT_OK;
}

static MunitResult testCopyFileAndDir(const MunitParameter params[], void *data) {
    // Copy file
    File *rootDir = NEW_FILE(FROM_PATH "/dir1");
    File *src = FILE_OF(rootDir, "/dir2/src_file.txt");
    assert_true(createFileDirs(src));
    assert_true(createFile(src));

    char *message = "Some test message";
    assert_uint32(writeCharsToFile(src, message, strlen(message), false), ==, strlen(message));

    File *dest = NEW_FILE(FROM_PATH "/dir1/dir2/dest_file.txt");
    assert_true(createFile(dest));
    assert_uint64(getFileSize(dest), ==, 0);

    assert_true(copyFile(src, dest));
    assert_uint64(getFileSize(dest), ==, strlen(message));

    assert_true(isFileExists(src));
    assert_true(isFileExists(dest));

    // Copy dir
    File *copyDir = NEW_FILE(FROM_PATH "/cp_dir");
    deleteDirectory(copyDir);

    assert_true(MKDIR(copyDir->path) == 0);
    assert_true(copyDirectory(rootDir, copyDir));
    assert_false(isEmptyDir(copyDir));

    fileVector *vec = NEW_VECTOR_64(file);
    listFilesAndDirs(copyDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 3);

    // Cleanup
    deleteDirectory(rootDir);
    deleteDirectory(copyDir);

    return MUNIT_OK;
}

static MunitResult testMoveFileAndDir(const MunitParameter params[], void *data) {
    // Src dir
    File *rootDir = NEW_FILE(FROM_PATH "/dir_t");
    deleteDirectory(rootDir);

    File *testDir = FILE_OF(rootDir, "/dir_1/dir_2");
    assert_true(createSubDirs(testDir));
    assert_true(MKDIR(testDir->path) == 0);

    File *file_1 = FILE_OF(rootDir, "/file_1.txt");
    assert_true(createFile(file_1));

    File *file_2 = FILE_OF(rootDir, "dir_1/file_2.txt");
    assert_true(createFile(file_2));

    File *file_3 = FILE_OF(rootDir, "dir_1/dir_2/file_3.txt");
    assert_true(createFile(file_3));

    // Dest dir
    File *destDir = NEW_FILE(FROM_PATH "/dest_dir");
    deleteDirectory(destDir);
    assert_true(MKDIR(destDir->path) == 0);

    File *srcDir = NEW_FILE(FROM_PATH "/dir_t");
    assert_true(moveDirToDir(srcDir, destDir));

    fileVector *vec = NEW_VECTOR_64(file);
    listFilesAndDirs(destDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 5);
    fileVecClear(vec);

    listFiles(destDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 3);
    for (int i = 0; i < fileVecSize(vec); i++) {
        File file = fileVecGet(vec, i);
        assert_true(strstr(file.path, "file_1.txt") != NULL ||
                    strstr(file.path, "file_2.txt") != NULL ||
                    strstr(file.path, "file_3.txt") != NULL);
    }
    fileVecClear(vec);

    // Move file to dir
    File *srcFile = NEW_FILE("file_to_move.txt");
    assert_true(createFile(srcFile));
    assert_true(moveFileToDir(srcFile, destDir));

    listFiles(destDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 4); // +1 file
    fileVecClear(vec);

    // Cleanup
    deleteDirectory(rootDir);
    deleteDirectory(destDir);

    return MUNIT_OK;
}

static MunitResult testReadFileToBuffer(const MunitParameter params[], void *data) {
    File *file = NEW_FILE("test_file_buff.txt");
    assert_true(createFile(file));

    char *str = "Some test text";
    uint32_t len = strlen(str);
    assert_true(writeCharsToFile(file, str, len, false) == len);
    assert_uint32(getFileSize(file), ==, len);

    char buffer[len + 1];
    memset(buffer, 0, len);
    assert_true(readFileToBuffer(file, buffer, len) == len);
    assert_string_equal(str, buffer);

    remove(file->path);
    return MUNIT_OK;
}

static MunitResult testReadFileToString(const MunitParameter params[], void *data) {
    File *file = NEW_FILE("test_file_str.txt");
    BufferString *str = NEW_STRING_64("Some test text");

    // check for not existing file
    assert_true(writeStringToFile(file, str, false) == 0);

    // create file and check
    assert_true(createFile(file));
    assert_true(writeStringToFile(file, str, false) == str->length);

    BufferString *result = EMPTY_STRING(64);
    assert_true(readFileToString(file, result) == str->length);
    assert_true(isBuffStringEquals(str, result));

    remove(file->path);
    return MUNIT_OK;
}

static MunitResult testBytesToStr(const MunitParameter params[], void *data) {
    BufferString *str = EMPTY_STRING(64);

    // Bytes
    byteCountToDisplaySize(128, str);
    assert_string_equal("128 bytes", str->value);
    clearString(str);

    // KB
    byteCountToDisplaySize(128000, str);
    assert_string_equal("125 KB", str->value);
    clearString(str);

    // MB
    byteCountToDisplaySize(128000000, str);
    assert_string_equal("122 MB", str->value);
    clearString(str);

    // GB
    byteCountToDisplaySize(128000000000, str);
    assert_string_equal("119 GB", str->value);
    clearString(str);

    // TB
    byteCountToDisplaySize(128000000000000, str);
    assert_string_equal("116 TB", str->value);
    clearString(str);

    return MUNIT_OK;
}

static MunitResult displaySizeToBytesTest(const MunitParameter params[], void *data) {
    // TB
    uint64_t byteCount = displaySizeToBytes("1 TB");
    assert_uint64(byteCount, ==, 1099511627776);

    // GB
    byteCount = displaySizeToBytes("1GB");
    assert_uint64(byteCount, ==, 1073741824);

    // MB
    byteCount = displaySizeToBytes("     1      mb        ");
    assert_uint64(byteCount, ==, 1048576);

    // KB
    byteCount = displaySizeToBytes("\n\t\r1\n\t\rkB\n\t\r");
    assert_uint64(byteCount, ==, 1024);

    // Bytes
    byteCount = displaySizeToBytes("12345678");
    assert_uint64(byteCount, ==, 12345678);

    // Invalid value
    byteCount = displaySizeToBytes("ab");
    assert_uint64(byteCount, ==, 0);

    // NULL
    byteCount = displaySizeToBytes(NULL);
    assert_uint64(byteCount, ==, 0);

    // Unknown size will be ignored
    byteCount = displaySizeToBytes("123 RP");
    assert_uint64(byteCount, ==, 123);

    return MUNIT_OK;
}

static MunitResult testFileCrc32(const MunitParameter params[], void *data) {
    File *file = NEW_FILE("test_file_crc32.txt");
    assert_true(createFile(file));

    char *str = "Some test text";
    uint32_t len = strlen(str);
    assert_true(writeCharsToFile(file, str, len, false) == len);

    char buffer[len];
    uint32_t code = fileChecksumCRC32(file, buffer, len);
    assert_uint32(code, ==, 1749825379);

    remove(file->path);
    return MUNIT_OK;
}

static MunitResult testFileCrc16(const MunitParameter params[], void *data) {
    File *file = NEW_FILE("test_file_crc16.txt");
    assert_true(createFile(file));

    char *str = "Some test text";
    uint32_t len = strlen(str);
    assert_true(writeCharsToFile(file, str, len, false) == len);

    char buffer[len];
    uint16_t code = fileChecksumCRC16(file, buffer, len);
    assert_uint16(code, ==, 36064);

    remove(file->path);
    return MUNIT_OK;
}


static MunitTest fileUtilsTests[] = {
        {.name =  "Test newFile() - should correctly create File struct", .test = testNewFile},
        {.name =  "Test create file and dir - should correctly create files and directories", .test = testCreateFileAndDir},
        {.name =  "Test getFileSize() - should correctly return file length", .test = testFileSize},
        {.name =  "Test parent and file name - should correctly get file name", .test = testFileNameAndParent},
        {.name =  "Test list of files - should correctly get all files from dir", .test = testFileList},
        {.name =  "Test copy file/dir - should correctly copy file and directory", .test = testCopyFileAndDir},
        {.name =  "Test move file/dir - should correctly move file and directory", .test = testMoveFileAndDir},
        {.name =  "Test file to buffer - should correctly read file to byte array", .test = testReadFileToBuffer},
        {.name =  "Test file to string - should correctly read file to buffer string", .test = testReadFileToString},
        {.name =  "Test bytes to string - should correctly convert bytes to KB/MB/GB/TB", .test = testBytesToStr},
        {.name =  "Test string to bytes - should correctly convert string with KB/MB/GB/TB to byte count", .test = displaySizeToBytesTest},
        {.name =  "Test file CRC32 - should correctly generate check code from file", .test = testFileCrc32},
        {.name =  "Test file CRC16 - should correctly generate check code from file", .test = testFileCrc16},
        END_OF_TESTS
};

static const MunitSuite fileUtilsTestSuite = {
        .prefix = "FileUtils: ",
        .tests = fileUtilsTests,
        .suites = NULL,
        .iterations = 1,
        .options = MUNIT_SUITE_OPTION_NONE
};
