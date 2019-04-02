//Standard I/O and library
#include <stdio.h>
#include <stdlib.h>
//Defines error number??
#include <errno.h>
//Inotify API
#include <linux/inotify.h>
//Signal handling
#include <signal.h>
// read and close
#include <unistd.h>
// opendir()
#include <dirent.h>
//Time
#include <time.h>

/*Define constants*/

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16)) /*I'm really not sure why \
                                                 it's 1024 * (EVENT_SIZE +16)*/
//Path of directory to monitor
#define DIRECTORY_PATH "/home/main/Source/C/env/createProject/Tasks-C-global"
//Path to scriptadd.sh
#define SCRIPTADD_PATH "/home/main/.startupScripts/updateCTasks/./scriptadd.sh"
#define SCRIPTREMOVE_PATH "/home/main/.startupScripts/updateCTasks/./scriptremove.sh"
//Function prototypes
//Inotify
int inotify_add_watch(int, char *, int);
int inotify_init();
int inotify_rm_watch(int, int);

void on_file_created(struct inotify_event *);
void on_file_deleted(struct inotify_event *);
void on_file_closeWrite(struct inotify_event *);

void sig_handler(int);
void execute_command(char *, char *, char *);
void format_time(char *);

// These have to be global to be handled at interrupt ¯\_(ツ)_/¯

int fileDescriptor;
int watchDescriptor;

//Inotify and calls
int main()
{

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");

    int length, i = 0;         /*These are for iterating through all of the events*/
    int buffer[EVENT_BUF_LEN]; /*Buffer to read events into? */

    // Create and initialise instance of inotify
    fileDescriptor = inotify_init();

    // inotify_init returns -1 on unsuccessful init
    if (fileDescriptor < 0)
    {
        // Return an error message
        perror("inotify_init()");
        printf("init");
    }

    DIR *dir = opendir(DIRECTORY_PATH);
    if (dir)
    {
        /* Directory exists. */
        closedir(dir);
    }
    else if (ENOENT == errno)
    {
        /* Directory does not exist. */
        printf("\n Directory does not exist \n");
        close(fileDescriptor);
        exit(1);
    }
    else
    {
        /* opendir() failed for some other reason. */
    }

    // Add a watch for the directory
    // Watch for IN_MODIFY, IN_CREATE and IN_DELETE in createProject folder
    watchDescriptor = inotify_add_watch(fileDescriptor, DIRECTORY_PATH, 
    IN_CLOSE_WRITE | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_MOVED_TO);

    //Do indefinately
    while (1)
    {
        i = 0;
        /*read to determine the event change happens on the directory. 
        The read blocks until the change event occurs*/
        length = read(fileDescriptor, buffer, EVENT_BUF_LEN);
        // Read returns -1 on failure
        if (length < 0)
        {
            perror("read()");
        }

        /*Read returns the list of change events
        Read through changes and handle one by one */
        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                if (event->mask & (IN_CREATE | IN_MOVED_TO))
                {
                    on_file_created(event);
                }
                else if (event->mask & (IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM))
                {
                    on_file_deleted(event);
                }
                else if (event->mask & IN_CLOSE_WRITE)
                {
                    on_file_closeWrite(event);
                }
            }
            /* No clue why */
            i += EVENT_SIZE + event->len;
        }
    }

    return 0;
}

void on_file_created(struct inotify_event *event)
{
    /*Display for debug*/
    char timestp[50];
    format_time(timestp);
    printf("%s ", timestp);
    printf("File %s was created.", event->name);
    printf("Event full path: \n%s/%s\n", DIRECTORY_PATH, event->name);

    //Goal: add file to create.sh
    execute_command(SCRIPTADD_PATH, event->name, DIRECTORY_PATH);
}
void on_file_deleted(struct inotify_event *event)
{
    /*Display for debug*/
    char timestp[50];
    format_time(timestp);
    printf("%s ", timestp);
    printf("File %s was deleted.", event->name);
    printf(" Event full path: \n%s/%s\n", DIRECTORY_PATH, event->name);

    //Goal: find file entry and remove from create.sh
    execute_command(SCRIPTREMOVE_PATH, event->name, DIRECTORY_PATH);
}
void on_file_closeWrite(struct inotify_event *event)
{
    char timestp[50];
    format_time(timestp);
    printf("%s ", timestp);
    printf("File %s was modified", event->name);
    printf(" Event full path: \n%s/%s\n", DIRECTORY_PATH, event->name);
    //Not implemented
}

void execute_command(char *commandPath, char *arg1, char *arg2)
{
    /*Construct as string command*/
    int BUF_LEN = 100;
    //Buffer to allocate memory block for
    char *commandBuffer = malloc(BUF_LEN * sizeof(char));

    int size = snprintf(commandBuffer, 400, "%s '%s' %s", commandPath, arg1, arg2);

    if (size >= BUF_LEN)
    {
        free(commandBuffer);
        char newCommandBuffer[size + 1];
        snprintf(newCommandBuffer, (size + 1) * sizeof(char), "%s '%s' %s", commandPath, arg1, arg2);
        system(newCommandBuffer);
    }
    else
    {
        system(commandBuffer);
    }
}

//Handle SIGINT (Ctrl+C)
void sig_handler(int signo)
{
    if (signo == SIGINT)
        printf("\n received SIGINT\n");
    inotify_rm_watch(fileDescriptor, watchDescriptor);
    close(fileDescriptor);
    exit(0);
}
// Store the formatted string of time in the output, for logging
void format_time(char *output)
{
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(output, "[%d/%d/%d %d:%d:%d]", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}
