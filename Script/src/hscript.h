/******************************************************************
 * Project Name: Hscript
 * Author: Tyler Neal
 * File: hscript.h
 * Date: 2/10/2025
 *
 * Description:
 * This program provides functionality for managing multiple file
 * descriptors, creating files and directories, and piping data
 * between processes. Its primary use-case is to execute a given
 * command, log its input, output, and errors to respective files,
 * and display the command's output and errors in real-time to the
 * terminal.
 *
 * The program ensures proper cleanup of opened file descriptors
 * in error scenarios and executes the given command using a
 * child process.
 *
 * Argument format:
 * ./hscript <program name> <arguments> <log_directory_name>
 ******************************************************************/

#ifndef HSCRIPT_H
#define HSCRIPT_H

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>

#define MAX_FDS 9 // Number of FDS an FD_Manager can track
#define BUFFER_SIZE 1024 // Buffer size used to write to fds in transferData()

//==================================================================
//                           Error Handling
//==================================================================

/**
 * @brief Calls a function and checks for an error code. If an error
 * is detected, it will return the same code. Allows simple passing
 * of error codes to main, where errors are handled.
 * 
 */
#define CE(call) do { \
    int status = (call); \
    if (status != 0) {\
        return status; \
    } \
} while(0) 

/**
 * @brief Calls a function and checks for an error code. If an error
 * is detected, it prints the error string, and calls provided clean
 * -up function. Often used in main to cleanup the function once a
 * terminal error is encountered.
 * 
 */
#define CEC(call, print, cleanup) do { \
    int status = (call); \
    if (status != 0) {\
        (print); \
        if ((cleanup)) { \
            (cleanup); \
        } \
        return status; \
    } \
} while(0) 

/**
 * @brief Contains error codes relating to various potential issues
 * encountered during runtime. Error codes are passed upwards to main
 * for debugging and error handling.
 * 
 */
typedef enum {
    SUCCESS = 0,

    // File creation
    ERR_FILE_OPEN = -100,
    ERR_FILE_READ = -101,
    ERR_FILE_WRITE = -102,
    ERR_LOG_FILE_OPEN = -103,

    // Pipe creation
    ERR_PIPE_CREATE = -200,

    // Directory creation
    ERR_DIR_CREATE = -300,

    // FD Management
    ERR_BAD_FD = -400,
    ERR_FD_ARRAY_FULL = -401,
    ERR_FD_CLOSE = -402,
    ERR_FD_NOT_FOUND = -403,
    ERR_FD_MNGR_ALLOCATION = -404,

    // Argument Validation
    ERR_INVALID_USAGE = -500,

    // Environmental Management
    ERR_STREAM_REDIRECT = -600,
    ERR_FORKING = -601

} script_error_t;

//==================================================================
//            Evironmental Management Structures/Enums
//==================================================================

/**
 * @brief Used for debug printing. Helps describe which process printed
 * the error info. Dedicated enumeration for clarity.
 * 
 */
typedef enum {
    PRE_FORK = 0,
    CHILD = 1,
    PARENT = 2
} proc_type_t;

/**
 * @brief - Struct containing opened file descriptors, allows for easy
 * and abrupt file descriptor cleanup.
 */
typedef struct {
    int fd_arr[MAX_FDS];
    int fd_counter;
} FD_Manager;

/**
 * @brief - Contains file stream info for stream redirection such as
 * file descriptor, path, and a pipe.
 */
typedef struct {
    int fd;
    char path[PATH_MAX];
    int pipe[2];
} Stream_Info;

/**
 * @brief - Holds info for each filestream, input, output, and error.
 */
typedef struct {
    Stream_Info input;
    Stream_Info output;
    Stream_Info error;
} Streams;

/**
 * @brief - Container for filesystem environmental info used for
 * cleaner argument passing and consolidation.
 */
typedef struct {
    FD_Manager* fd_mngr;
    char* command;
    char* dir_name;
    Streams streams;
} Environmental_Info;

//==================================================================
//                       Function Declarations 
//==================================================================

// ------- Environment Creation -------
int buildEnvironment(Environmental_Info* env_info, int argc, char*** argv);
int parseArguments(int argc, char*** argv, char** command, char** dirName);
int initLogFiles(const char* dir_name, FD_Manager* fd_mngr, Streams* streams);
int initPipes(FD_Manager* fd_mngr, Streams* streams);
int redirectStreams(Streams* streams);

// ------- Directory Management -------
int createFile(const char* path, mode_t mode, FD_Manager* fd_mngr);
int createPipe(int* pipeFD, FD_Manager* fd_mngr);
int createDirectory(const char* dir_name);

// ---- File Descriptor Management ----
FD_Manager* allocateFDManager();
int destroyFDManager(FD_Manager* fd_mngr);
void zeroFDArray(FD_Manager* fd_mngr);
int addFDToManager(int fd, FD_Manager* fd_mngr);
int closeManagedFD(int fd, FD_Manager* fd_mngr);
int cleanupFDManager(FD_Manager* fd_mngr);
int closeFD(int fd);

// -------- Additional Helpers --------
void printError(int arg_count, char *format, ...);
int transferData(int srcFD, int logFD, int destFD, const char *logPath, int pipe[]);
void printFDManager(FD_Manager* fd_mngr);

#endif // HSCRIPT_H