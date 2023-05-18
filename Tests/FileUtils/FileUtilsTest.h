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
    File file_1 = newFile("test_file.txt");
    assert_uint32(13, ==, file_1.pathLength);
    assert_string_equal("test_file.txt", file_1.path);
    assert_null(file_1.file);
    assert_null(file_1.dir);

    File file_2 = newFile("dir1/dir2/file.txt");
    assert_uint32(file_2.pathLength, ==, 18);
    assert_string_equal("dir1" FILE_SEP "dir2" FILE_SEP "file.txt", file_2.path);

    File file_3 = newFile(NULL);
    assert_uint32(file_3.pathLength, ==, 0);

    File rootDir = newFile("/dir1/dir2/");
    File subDir = newFileFromParent(&rootDir, "\\dir3\\dir4");
    assert_string_equal(FILE_SEP "dir1" FILE_SEP "dir2" FILE_SEP "dir3" FILE_SEP "dir4", subDir.path);

    File file_4 = newFileFromParent(&subDir, "test_4_file.txt");
    assert_string_equal(FILE_SEP "dir1" FILE_SEP "dir2" FILE_SEP "dir3" FILE_SEP "dir4" FILE_SEP "test_4_file.txt", file_4.path);

    File file_5 = newFileFromParent(&subDir, file_1.path);
    assert_string_equal(FILE_SEP "dir1" FILE_SEP "dir2" FILE_SEP "dir3" FILE_SEP "dir4" FILE_SEP "test_file.txt", file_5.path);

    File file_6 = getParentFile(&file_5);
    assert_string_equal(FILE_SEP "dir1" FILE_SEP "dir2" FILE_SEP "dir3" FILE_SEP "dir4", file_6.path);

    File file_7 = getParentFile(&file_1);
    assert_string_equal("", file_7.path);   // no parent

    File file_8 = newFileFromParent(&file_6, NULL);
    assert_string_equal("", file_8.path);

    File file_9 = newFile(ROOT "/file_9.txt");
    assert_true(strstr(file_9.path, FILE_SEP FILE_SEP) == NULL);  // no multiple path separators

    return MUNIT_OK;
}

static MunitResult testCreateFileAndDir(const MunitParameter params[], void *data) {
    // Single file
    File file_1 = newFile(FROM_PATH "/test_file.txt");
    assert_false(isFileExists(&file_1));
    assert_true(createSubDirs(&file_1));
    assert_true(createFile(&file_1));

    assert_true(isFile(&file_1));
    assert_true(isFileExists(&file_1));
    remove(file_1.path);

    // Sub dirs
    File rootDir = newFile(FROM_PATH "/dir_t_1/dir_t_2/");
    deleteDirectory(&rootDir);

    File file_2 = newFileFromParent(&rootDir, "test_file_2.txt");
    assert_false(isFileExists(&file_2));
    assert_true(createFileDirs(&file_2));
    assert_true(createFile(&file_2));

    assert_true(isFile(&file_2));
    assert_true(isFileExists(&file_2));
    assert_false(isDirectory(&file_2));
    assert_false(isDirExists(&file_2));

    // Rename
    File file_3 = newFile(FROM_PATH "test_file_xx.txt");
    assert_true(renameFileTo(&file_2, &file_3));
    assert_false(isFileExists(&file_2));
    assert_true(isFileExists(&file_3));
    remove(file_3.path);

    // Create dirs
    File dir = newFile(FROM_PATH "/dir_1/dir_2/dir_3");
    assert_false(isDirExists(&dir));
    assert_true(createSubDirs(&dir));
    assert_true(MKDIR(dir.path) == 0);
    assert_true(isDirExists(&dir));
    assert_true(isDirectory(&dir));

    // Delete all dirs
    deleteDirectory(&dir);
    assert_false(isDirExists(&dir));

    return MUNIT_OK;
}

static MunitResult testFileSize(const MunitParameter params[], void *data) {
    File file_1 = newFile(FROM_PATH "/test_size_file.txt");
    assert_true(createSubDirs(&file_1));
    assert_true(createFile(&file_1));
    assert_uint64(0, ==, getFileSize(&file_1));

    char *message = "Some test message";
    assert_uint32(writeCharsToFile(&file_1, message, strlen(message), false), ==, strlen(message));
    assert_uint64(strlen(message), ==, getFileSize(&file_1));
    remove(file_1.path);

    return MUNIT_OK;
}

static MunitResult testFileNameAndParent(const MunitParameter params[], void *data) {
    File file = newFile(FROM_PATH "/dir1/dir2/some_file.txt");
    BufferString *str = EMPTY_STRING(PATH_MAX_LEN);
    getFileName(&file, str);
    assert_string_equal("some_file.txt", str->value);
    clearString(str);

    File file_2 = newFile("some_file_2.txt");
    getFileName(&file_2, str);
    assert_string_equal("some_file_2.txt", str->value);
    clearString(str);

    getParentName(&file, str);
    assert_string_equal(FROM_PATH FILE_SEP "dir1" FILE_SEP "dir2", str->value);
    clearString(str);

    return MUNIT_OK;
}

static MunitResult testFileList(const MunitParameter params[], void *data) {
    File rootDir = newFile(FROM_PATH "/dir1");
    deleteDirectory(&rootDir);

    File subDir = newFileFromParent(&rootDir, "/dir2");
    File subSubDir = newFileFromParent(&subDir, "/dir3");
    assert_true(createSubDirs(&subSubDir)); // creates all sub dirs
    assert_true(MKDIR(subSubDir.path) == 0);

    File file_1 = newFileFromParent(&rootDir, "/file_1.txt");
    File file_2 = newFileFromParent(&subDir, "/file_2.txt");
    File file_3 = newFileFromParent(&subSubDir, "/file_3.txt");
    assert_true(createFile(&file_1));
    assert_true(createFile(&file_2));
    assert_true(createFile(&file_3));

    fileVector *vec = NEW_VECTOR_64(file);
    listFiles(&rootDir, vec, false);
    assert_uint32(fileVecSize(vec), ==, 1);
    assert_string_equal(fileVecGet(vec, 0).path, file_1.path);
    fileVecClear(vec);

    listFiles(&rootDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 3);
    assert_true(fileVecContains(vec, file_1));
    assert_true(fileVecContains(vec, file_2));
    assert_true(fileVecContains(vec, file_3));
    fileVecClear(vec);

    listFilesAndDirs(&rootDir, vec, false);
    assert_uint32(fileVecSize(vec), ==, 2);
    assert_true(fileVecContains(vec, subDir));
    assert_true(fileVecContains(vec, file_1));
    fileVecClear(vec);

    listFilesAndDirs(&rootDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 5);
    assert_true(fileVecContains(vec, subDir));
    assert_true(fileVecContains(vec, subSubDir));
    assert_true(fileVecContains(vec, file_1));
    assert_true(fileVecContains(vec, file_2));
    assert_true(fileVecContains(vec, file_3));
    fileVecClear(vec);

    cleanDirectory(&rootDir);
    listFilesAndDirs(&rootDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 0);
    assert_true(isEmptyDir(&rootDir));

    assert_true(deleteDirectory(&rootDir));
    assert_false(isDirExists(&rootDir));

    return MUNIT_OK;
}

static MunitResult testCopyFileAndDir(const MunitParameter params[], void *data) {
    // Copy file
    File rootDir = newFile(FROM_PATH "/dir1");
    File src = newFileFromParent(&rootDir, "/dir2/src_file.txt");
    assert_true(createFileDirs(&src));
    assert_true(createFile(&src));

    char *message = "Some test message";
    assert_uint32(writeCharsToFile(&src, message, strlen(message), false), ==, strlen(message));

    File dest = newFile(FROM_PATH "/dir1/dir2/dest_file.txt");
    assert_true(createFile(&dest));
    assert_uint64(getFileSize(&dest), ==, 0);

    assert_true(copyFile(&src, &dest));
    assert_uint64(getFileSize(&dest), ==, strlen(message));

    assert_true(isFileExists(&src));
    assert_true(isFileExists(&dest));

    // Copy dir
    File copyDir = newFile(FROM_PATH "/cp_dir");
    deleteDirectory(&copyDir);

    assert_true(MKDIR(copyDir.path) == 0);
    assert_true(copyDirectory(&rootDir, &copyDir));
    assert_false(isEmptyDir(&copyDir));

    fileVector *vec = NEW_VECTOR_64(file);
    listFilesAndDirs(&copyDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 3);

    // Cleanup
    deleteDirectory(&rootDir);
    deleteDirectory(&copyDir);

    return MUNIT_OK;
}

static MunitResult testMoveFileAndDir(const MunitParameter params[], void *data) {
    // Src dir
    File rootDir = newFile(FROM_PATH "/dir_t");
    deleteDirectory(&rootDir);

    File testDir = newFileFromParent(&rootDir, "/dir_1/dir_2");
    assert_true(createSubDirs(&testDir));
    assert_true(MKDIR(testDir.path) == 0);

    File file_1 = newFileFromParent(&rootDir, "/file_1.txt");
    assert_true(createFile(&file_1));

    File file_2 = newFileFromParent(&rootDir, "dir_1/file_2.txt");
    assert_true(createFile(&file_2));

    File file_3 = newFileFromParent(&rootDir, "dir_1/dir_2/file_3.txt");
    assert_true(createFile(&file_3));

    // Dest dir
    File destDir = newFile(FROM_PATH "/dest_dir");
    deleteDirectory(&destDir);
    assert_true(MKDIR(destDir.path) == 0);

    File srcDir = newFile(FROM_PATH "/dir_t");
    assert_true(moveDirToDir(&srcDir, &destDir));

    fileVector *vec = NEW_VECTOR_64(file);
    listFilesAndDirs(&destDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 5);
    fileVecClear(vec);

    listFiles(&destDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 3);
    for (int i = 0; i < fileVecSize(vec); i++) {
        File file = fileVecGet(vec, i);
        assert_true(strstr(file.path, "file_1.txt") != NULL ||
                    strstr(file.path, "file_2.txt") != NULL ||
                    strstr(file.path, "file_3.txt") != NULL);
    }
    fileVecClear(vec);

    // Move file to dir
    File srcFile = newFile("file_to_move.txt");
    assert_true(createFile(&srcFile));
    assert_true(moveFileToDir(&srcFile, &destDir));

    listFiles(&destDir, vec, true);
    assert_uint32(fileVecSize(vec), ==, 4); // +1 file
    fileVecClear(vec);

    // Cleanup
    deleteDirectory(&rootDir);
    deleteDirectory(&destDir);

    return MUNIT_OK;
}

static MunitResult testReadFileToBuffer(const MunitParameter params[], void *data) {
    File file = newFile("test_file_buff.txt");
    assert_true(createFile(&file));

    char *str = "Some test text";
    uint32_t len = strlen(str);
    assert_true(writeCharsToFile(&file, str, len, false) == len);
    assert_uint32(getFileSize(&file), ==, len);

    char buffer[len];
    assert_true(readFileToBuffer(&file, buffer, len) == len);
    assert_string_equal(str, buffer);

    remove(file.path);
    return MUNIT_OK;
}

static MunitResult testReadFileToString(const MunitParameter params[], void *data) {
    File file = newFile("test_file_str.txt");
    BufferString *str = NEW_STRING_64("Some test text");

    // check for not existing file
    assert_true(writeStringToFile(&file, str, false) == 0);

    // create file and check
    assert_true(createFile(&file));
    assert_true(writeStringToFile(&file, str, false) == str->length);

    BufferString *result = EMPTY_STRING(64);
    assert_true(readFileToString(&file, result) == str->length);
    assert_true(isBuffStringEquals(str, result));

    remove(file.path);
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

static MunitResult testFileCrc32(const MunitParameter params[], void *data) {
    File file = newFile("test_file_crc32.txt");
    assert_true(createFile(&file));

    char *str = "Some test text";
    uint32_t len = strlen(str);
    assert_true(writeCharsToFile(&file, str, len, false) == len);

    char buffer[len];
    uint32_t code = fileChecksumCRC32(&file, buffer, len);
    assert_uint32(code, ==, 1749825379);

    remove(file.path);
    return MUNIT_OK;
}

static MunitResult testFileCrc16(const MunitParameter params[], void *data) {
    File file = newFile("test_file_crc16.txt");
    assert_true(createFile(&file));

    char *str = "Some test text";
    uint32_t len = strlen(str);
    assert_true(writeCharsToFile(&file, str, len, false) == len);

    char buffer[len];
    uint16_t code = fileChecksumCRC16(&file, buffer, len);
    assert_uint16(code, ==, 36064);

    remove(file.path);
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
