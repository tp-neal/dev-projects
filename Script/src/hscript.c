/******************************************************************
 * Project Name: Hscript
 * Author: Tyler Neal
 * File: hscript.c
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
 * ./hscript <program name> <arguments> ... <log_directory_name>
 ******************************************************************/

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/limits.h>
#include "hscript.h"
#include <stdarg.h>
#include <errno.h>
#include <stdbool.h>

int log_fd; // File descriptor for file in which all error output will be logged
proc_type_t proc_t = PRE_FORK; // Used for determining which process printed error

//==================================================================
//                            Main
//==================================================================

int main(int argc, char** argv) {

    // Open file for logging error information
    log_fd = open("err_log", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (log_fd == -1) {
        return ERR_LOG_FILE_OPEN;
    }

    // Initialize environment container
    Environmental_Info env_info = {0};
    Streams* streams = &env_info.streams; // pointer used for visual clarity after forking

    // Build the environment, and catch any errors
    int env_status;
    CEC(env_status = buildEnvironment(&env_info, argc, &argv),
                        printError(1, "[main] - Failed to setup environment - Code: %d", env_status), 
                        destroyFDManager(env_info.fd_mngr));

    // Fork child for command execution
    pid_t pid = fork();

    // Catch forking error
    if (pid < 0) {
        printError(1, "[main] - Error while forking - Info: %s", strerror(errno));
        destroyFDManager(env_info.fd_mngr);
        return ERR_FORKING;
    }

    // Child (executes command)
    else if (pid == 0) {

        // Change global for error prints
        proc_t = CHILD;

        // Cleanup uneccesary file decriptors
        int unnecessary_fds[] = {
            streams->input.pipe[1],
            streams->output.pipe[0],
            streams->error.pipe[0],
            streams->input.fd,
            streams->output.fd,
            streams->error.fd}; 
        for (unsigned int i = 0; i < sizeof(unnecessary_fds)/sizeof(int); i++) {
            CEC(closeManagedFD(unnecessary_fds[i], env_info.fd_mngr), 
                    printError(0, "[main] - Unable to close unecessary pipe ends"),
                    destroyFDManager(env_info.fd_mngr));
        }

        // Reroutes childs standard input/output to pipes
        CEC(redirectStreams(&env_info.streams), 
                printError(0, "[main] - Unable to redirect streams"),
                destroyFDManager(env_info.fd_mngr));

        // Exec into designated command
        execvp(env_info.command, argv);
        printError(1, "[main] - Execvp failed to execute command - Info: %s", strerror(errno));
    }

    // Parent (monitors streams)
    else {

        // Change global for error prints
        proc_t = PARENT;

        // Cleanup uneccesary pipe ends
        int unnecessary_fds[] = {
            streams->input.pipe[0],
            streams->output.pipe[1],
            streams->error.pipe[1]};
        for (unsigned int i = 0; i < sizeof(unnecessary_fds)/sizeof(int); i++) {
            CEC(closeManagedFD(unnecessary_fds[i], env_info.fd_mngr), 
                    printError(0, "[main] - Unable to close unecessary pipe ends"),
                    destroyFDManager(env_info.fd_mngr));
        }

        // Selection variables
        int status = 0;
        fd_set readfds;

        // Continue checking fds until child has finished executing
        while (waitpid(pid, &status, WNOHANG) == 0) {

            // Clear fd_set for reselection
            FD_ZERO(&readfds);

            // Monitor stdin, stdout, and stderr
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(streams->output.pipe[0], &readfds);
            FD_SET(streams->error.pipe[0], &readfds);
            const int FD_BOUND = (streams->error.pipe[1] + 1);

            // Attempt to monitor stream descriptors
            CEC(select(FD_BOUND, &readfds, NULL, NULL, NULL) <= 0,
                    printError(1, "[main] - Failed to monitor file descriptors - Info: %s", strerror(errno)),
                    destroyFDManager(env_info.fd_mngr));

            // Stdin ready to read
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                transferData(STDIN_FILENO, 
                             streams->input.fd, 
                             streams->input.pipe[1],  
                             streams->input.path, 
                             streams->input.pipe);
            }

            // Command output ready to read
            if (FD_ISSET(streams->output.pipe[0], &readfds)) {
                transferData(streams->output.pipe[0], 
                             streams->output.fd, 
                             STDOUT_FILENO, 
                             streams->output.path, 
                             NULL);
            }

            // Command error ready to read
            if (FD_ISSET(streams->error.pipe[0], &readfds)) {
                transferData(streams->error.pipe[0], 
                             streams->error.fd,
                             STDERR_FILENO,
                             streams->error.path, 
                             NULL);
            }
        }
    }

    // Exit program
    destroyFDManager(env_info.fd_mngr);
    return SUCCESS;
}

//==================================================================
//                       Environmental Setup
//==================================================================

/**
 * @brief Sets up environment including directories, logfiles, and pipe creation
 * 
 * @param env_info Struct containing information about the file environment
 * @param argc Number of arguments passed to program
 * @param argv Pointer to array of arguments in form of strings
 * @return int 0 on success : (-) on error
 */
int buildEnvironment(Environmental_Info* env_info, int argc, char*** argv) {
    // Set default fd values to -1
    env_info->fd_mngr = allocateFDManager();
    if (!env_info->fd_mngr) {
        return ERR_FD_MNGR_ALLOCATION;
    }
    
    // Parse through command line arguments
    CE(parseArguments(argc, argv, &env_info->command, &env_info->dir_name));

    // Create directory to hold log files
    CE(createDirectory((const char*)env_info->dir_name));

    // Open new files for logging stdin stdout and stderr
    CE(initLogFiles((const char*)env_info->dir_name, env_info->fd_mngr, &env_info->streams));

    // Create pipes for each stream connected to new log files
    CE(initPipes(env_info->fd_mngr, &env_info->streams));

    return SUCCESS;
}

/**
 * @brief Parses command line arguments for the required command and directory.
 * Modifies the argv array to be compatible for the execvp call.
 * 
 * @param argc Number of arguments passed to program
 * @param argv Array of arguments in form of character arrays
 * @param command Pointer to command string
 * @param dirName Pointer to directory name string
 * @return int 0 on success : (-) on error
 */
int parseArguments(int argc, char*** argv, char** command, char** dirName) {
    if (argc < 3) {
        printError(0, "Invalid argument count\n" 
                       "Usage: ./hscript <program name> <optional_arguments> <directory>");
        return ERR_INVALID_USAGE;
    }

    // Dereference argv for simplicity
    char** args = *argv;

    // Retrieve arguments
    *command = args[1];
    *dirName = args[argc - 1];

    // Modify argv for execvp call in child
    for (int i = 0; i < argc - 1; i++) {
        args[i] = args[i + 1];
    }

    args[argc-2] = NULL; // remove directory name
    args[argc-1] = NULL; // set last argument to NULL for execvp

    return SUCCESS;
}

/**
 * @brief Opens the files in which the command's stdin, stderr, and stdout will be logged
 * 
 * @param dir_name Directory name for log files to be stored
 * @param fd_mngr Struct that manages fd's for abrupt cleanup
 * @param streams Pointer to structs that contain stdin, stderr, and stdout stream info
 * @return int 0 on success : (-) on error
 */
int initLogFiles(const char* dir_name, FD_Manager* fd_mngr, Streams* streams) {    
    // Create array for iterating streams
    Stream_Info* stream_info[3];
    stream_info[0] = &streams->input;
    stream_info[1] = &streams->output;
    stream_info[2] = &streams->error;

    // Concatenate file name to log directory for full path
    for (int i = 0; i < 3; i++) {
        // Store full path
        snprintf(stream_info[i]->path, PATH_MAX, "%s/%d", dir_name, i);

        // Open file and check error
        int fd = createFile(stream_info[i]->path, 0, fd_mngr);
        if (fd < 0) {
            return fd;
        }

        // Store the fd in stream_info container
        stream_info[i]->fd = fd;
    }
    
    return SUCCESS;
}

/**
 * @brief Creates pipes for I/O rerouting
 * 
 * @param fd_mngr Struct that manages fd's for abrupt cleanup
 * @param streams Pointer to structs that contain stdin, stderr, and stdout stream info
 * @return int 0 on success : (-) on error
 */
int initPipes(FD_Manager* fd_mngr, Streams* streams) {
    int* stream_pipes[3];
    stream_pipes[0] = streams->input.pipe;
    stream_pipes[1] = streams->output.pipe;
    stream_pipes[2] = streams->error.pipe;

    // Create each pipe
    for (int i = 0; i < 3; i++) {
        CE(createPipe(stream_pipes[i], fd_mngr));
    }

    return SUCCESS;
}

/**
 * @brief Redirects streams for child (command) process to the created pipes
 * 
 * @param streams Pointer to structs that contain stdin, stderr, and stdout stream info
 * @return int 0 on success : (-) on error
 */
int redirectStreams(Streams* streams) {
    if (dup2(streams->input.pipe[0], STDIN_FILENO) == -1 || 
        dup2(streams->output.pipe[1], STDOUT_FILENO) == -1 || 
        dup2(streams->error.pipe[1], STDERR_FILENO) == -1) 
    {
        printError(1, "[redirectStreams] - Could not redirect standard filestreams - Info: %s", strerror(errno));
        return ERR_STREAM_REDIRECT;
    }

    return SUCCESS;
}

//==================================================================
//                       Directory Management
//==================================================================

/**
 * @brief Open a new file and add its descriptor to the fd manager
 * 
 * @param path Path to file concatenated with the filename
 * @return int fd of opened file on success : (-) on error
 */
int createFile(const char* path, mode_t mode, FD_Manager* fd_mngr) {
    // Default mode if necessary
    if (mode == 0) {
        mode = 0644;
    }

    // Open file and assign fd
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, mode);

    // Catch error
    if (fd == -1) {
        printError(2, "[createFile] - Could not create path '%s' - Info: %s", path, strerror(errno));
        return ERR_FILE_OPEN; 
    }
    
    // Keep track of fd for cleanup later
    CE(addFDToManager(fd, fd_mngr));

    // Return the opened file descriptor
    return fd;
}

/**
 * @brief Create a new pipe
 * 
 * @param pipeFD Pointer to array of two fds to be piped
 * @return int 0 on success : (-) on error
 */
int createPipe(int* pipeFD, FD_Manager* fd_mngr) {
    // Attempt to make pipe, and catch errors
    if (pipe(pipeFD) != 0) {
        printError(1, "[createPipe] - Could not create pipe - Info: %s", strerror(errno));
        return ERR_PIPE_CREATE;
    }

    // Keep track of fd for cleanup later
    CE(addFDToManager(pipeFD[0], fd_mngr));
    CE(addFDToManager(pipeFD[1], fd_mngr));

    return SUCCESS;
}

/**
 * @brief Open a new directory
 * 
 * @param dir_name Name of directory to be created
 * @return int 0 on success : (-) on error
 */
int createDirectory(const char* dir_name) {
    // Attempt to make directory, and catch errors
    if (mkdir(dir_name, 0700) != 0) {
        printError(2, "[createDirectory] - Could not create directory '%s' - Info: %s", dir_name, strerror(errno));
        return ERR_DIR_CREATE;
    }

    return SUCCESS;
}

//==================================================================
//                    File Descriptor Management
//==================================================================

/**
 * @brief Allocate an fd manager, zero it out, and return a pointer to it
 * 
 * @return FD_Manager* pointer to the allocated struct
 */
FD_Manager* allocateFDManager() {
    // Allocate manager struct
    FD_Manager* fd_mngr = (FD_Manager*)malloc(sizeof(FD_Manager));
    if (!fd_mngr) {
        printError(1, "[allocateFDManager] - Could not allocate space for FD_Manager - Info: %s", strerror(errno));
        return NULL;
    }

    // Zero out fd manager array and initialize count
    zeroFDArray(fd_mngr);
    
    return fd_mngr;
}

/**
 * @brief Closes all file descriptors stored, and frees the manager memory
 * 
 * @param fd_mngr Struct containing the file descriptors to be cleaned up
 * @return int 0 on success : (-) on error
 */
int destroyFDManager(FD_Manager* fd_mngr){
    if (fd_mngr) {
        cleanupFDManager(fd_mngr);
        free(fd_mngr);
        return 0;
    }
    return -1;
}

/**
 * @brief Zero's out the fd_array to -1's
 * 
 * @param fd_mngr Fd manager struct to be zero'd out
 */
void zeroFDArray(FD_Manager* fd_mngr) {
    for (int i = 0; i < MAX_FDS; i++){
        fd_mngr->fd_arr[i] = -1;
    }
    fd_mngr->fd_counter = 0;
}

/**
 * @brief Adds an fd to the manager, so it can be closed upon abrupt exit
 * 
 * @param fd File descriptor to be added to list
 * @param fd_mngr Container for fd list and counter
 * @return int 0 on success : (-) on error
 */
int addFDToManager(int fd, FD_Manager* fd_mngr) {
    // Catch any issues before adding the fd
    if (fd == -1) {
        printError(0, "[addFDToManager] - Cannot add fd of default value '-1'");
        return ERR_BAD_FD;
    } else if (fd_mngr->fd_counter >= MAX_FDS) {
        printError(1, "[addFDToManager] Cannot add fd '%d' as FD array is already full", fd);
        return ERR_FD_ARRAY_FULL;
    }

    // Add fd to list
    fd_mngr->fd_arr[fd_mngr->fd_counter++] = fd;

    return SUCCESS;
}

/**
 * @brief Closes an managed fd's and updates the manager
 * 
 * @param fd File descriptor to be closed
 * @param fd_mngr Container for fd list and counter
 * @return int 0 on success : (-) on error
 */
int closeManagedFD(int fd, FD_Manager* fd_mngr) {
    // Catch any issues before closing the fd
    if (fd == -1) {
        printError(1, "[closeManagedFD] - Cannot add FD of default value '-1'", fd);
        return ERR_BAD_FD;
    }

    // Find FD index
    int fd_index = -1; // -1 If not found
    for (int i = 0; i < fd_mngr->fd_counter; i++) {
        if (fd_mngr->fd_arr[i] == fd) {
            fd_index = i;
            break;
        }
    }

    // FD not found
    if (fd_index == -1) {
        printError(1, "[closeManagedFD] Fd '%d' was not found in manager list", fd);
        return ERR_FD_NOT_FOUND;
    }

    // Close fd and hift the remaining fds in the list
    CE(closeFD(fd));
    for (int i = fd_index; i < fd_mngr->fd_counter - 1; i++) {
        fd_mngr->fd_arr[i] = fd_mngr->fd_arr[i + 1];
    }
    fd_mngr->fd_arr[fd_mngr->fd_counter - 1] = -1;

    // Decrement the counter
    fd_mngr->fd_counter--;

    return SUCCESS;
}

/**
 * @breif Closes all file descriptors in the fd manager
 * 
 * @param fd_mngr Container for fd list and counter
 * @return int 0 on success : (-) on error
 */
int cleanupFDManager(FD_Manager* fd_mngr) {
    for (int i = fd_mngr->fd_counter-1; i >= 0; i--) {
        CE(closeFD(fd_mngr->fd_arr[i]));
        fd_mngr->fd_arr[i] = -1;
    }
    
    return SUCCESS;
}

/**
 * @brief Close a file descriptor, or print error and exit if unable
 * 
 * @param fd File descriptor to be closed
 * @return int 0 on success : (-) on error
 */
int closeFD(int fd) {
    // Attempt to close FD
    if (close(fd) != 0) {
        printError(2, "[closeFD] - Could not close fd '%d' - Info: %s", fd, strerror(errno));
        return ERR_FD_CLOSE;
    }

    return SUCCESS;
}

//==================================================================
//                       Additional Helpers
//==================================================================

/**
 * @brief Prints an error to the logfile
 * 
 * @param arg_count Number of format specifiers in formatted string
 * @param format Formatted string optionally containing specifiers
 * @param ... variables for string formatting
 */
void printError(int arg_count, char *format, ...) {

    // Format print string for writing
    va_list args;
    va_start(args, format);

    char buffer[1024];
    int len = 0;
    char* prefix = (proc_t == PRE_FORK) ? "PF" : (proc_t == CHILD) ? "C" : "P";

    // Build error message safely
    len += snprintf(buffer + len, sizeof(buffer) - len, "\n(%s) Error: ", prefix);
    if (arg_count == 0) {
        len += snprintf(buffer + len, sizeof(buffer) - len, "%s", format);
    } else {
        // Use vsnprintf for variadic arguments
        len += vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
    }
    len += snprintf(buffer + len, sizeof(buffer) - len, "\n\n");

    // Write to log file
    write(log_fd, buffer, len);

    va_end(args);
    exit(-1);  // Ensure program exits after error
}

/**
 * @brief Writes data to both a log file, and destination fd
 * 
 * @param srcFD File from which data is read
 * @param logFD Log file to be written to
 * @param destFD Additional file to be written to
 * @param logPath Path to the logfile
 * @param pipe Pipe holding the file descriptors to be read/written from
 * @return int 0 on success : (-) on error
 */
int transferData(int srcFD, int logFD, int destFD, const char *logPath, int pipe[]) {
    // Read from source file
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(srcFD, buffer, BUFFER_SIZE);
    
    // Handle read errors
    if (bytes_read == -1) {
        printError(1, "[transferData] - Could not read from source fd '%d': %s", 
                  srcFD, strerror(errno));
        return ERR_FILE_READ;
    }

    // Handle EOF for pipe
    if (bytes_read == 0 && pipe != NULL && destFD == pipe[1]) {
        return closeFD(pipe[1]);
    }

    // Write to log file
    ssize_t total_written_log = 0;
    while (total_written_log < bytes_read) {
        ssize_t written = write(logFD, buffer + total_written_log, 
                                bytes_read - total_written_log);
        if (written == -1) {
            if (errno == EINTR) continue;  // Retry on interrupt
            printError(1, "[transferData] - Could not write to log file %s: %s", 
                       logPath, strerror(errno));
            return ERR_FILE_WRITE;
        }
        total_written_log += written;
    }

    // Write to destination
    ssize_t total_written_dest = 0;
    while (total_written_dest < bytes_read) {
        ssize_t written = write(destFD, buffer + total_written_dest, 
                              bytes_read - total_written_dest);
        if (written == -1) {
            if (errno == EINTR) continue;  // Retry on interrupt
            printError(1, "[transferData] - Could not write to destination fd '%d': %s", 
                      destFD, strerror(errno));
            return ERR_FILE_WRITE;
        }
        total_written_dest += written;
    }

    return SUCCESS;
}

/**
 * @brief Prints contents of fd manager to stderr for debugging
 * 
 * @param fd_mngr Fd manger who's fd array is to be printed
 */
void printFDManager(FD_Manager* fd_mngr) {
    fprintf(stderr, "{|");
    for (int i = 0; i < MAX_FDS; i++) {
        fprintf(stderr, " [%d]:%d |", i, fd_mngr->fd_arr[i]);
    }
    fprintf(stderr, "} fd_counter: %d\n", fd_mngr->fd_counter);
}