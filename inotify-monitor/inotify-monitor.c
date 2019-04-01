//Standard I/O and library
#include <stdio.h>
#include <stdlib.h>
//Defines error number??
#include <errno.h>
//Struct types and stuff?
#include <sys/types.h>
//Inotify API
#include <linux/inotify.h>
//Signal handling
#include <signal.h>
// read and close
#include <unistd.h>
// opendir()
#include <dirent.h>


//Define constants
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) ) /*I'm really not sure why 
                                                         it's 1024 * (EVENT_SIZE +16)*/
//Function prototypes
//Inotify 
int inotify_add_watch(int,char*,int);
int inotify_init();
int inotify_rm_watch(int,int);

void onFileCreated(struct inotify_event*);
void onFileDeleted(struct inotify_event*);
void onFileCloseWrite(struct inotify_event*);

void sig_handler(int);

// These have to be global to be handled at interrupt ¯\_(ツ)_/¯

int fileDescriptor;
int watchDescriptor;

//Inotify and calls
int main() {

    if (signal(SIGINT, sig_handler) == SIG_ERR)
    printf("\ncan't catch SIGINT\n");
    
    int length, i = 0; /*These are for iterating through all of the events*/
    int buffer[EVENT_BUF_LEN]; /*Buffer to read events into? */


    // Create and initialise instance of inotify
    fileDescriptor = inotify_init();

    // inotify_init returns -1 on unsuccessful init
    if (fileDescriptor < 0) {
        // Return an error message
        perror("inotify_init()");
        printf("init");
    }

    DIR* dir = opendir("/home/main/Source/C/env/createProject/");
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
    watchDescriptor = inotify_add_watch(fileDescriptor,"/home/main/Source/C/env/createProject/", IN_CLOSE_WRITE | IN_CREATE | IN_DELETE);

    //Do indefinately
    while (1) {
        i = 0;
        /*read to determine the event change happens on the directory. 
        The read blocks until the change event occurs*/ 
        length = read(fileDescriptor,buffer,EVENT_BUF_LEN); 
        // Read returns -1 on failure
        if ( length < 0) {
            perror("read()");
        }

        /*Read returns the list of change events
        Read through changes and handle one by one */
        while (i < length) {    
            struct inotify_event *event = (struct inotify_event *) & buffer[i];
            if ( event->len ) {
                if (event->mask & IN_CREATE) {
                    onFileCreated(event);
                }
                else if (event->mask & IN_DELETE) {
                    onFileDeleted(event);
                }
                else if (event->mask & IN_CLOSE_WRITE) {
                    onFileCloseWrite(event);
                }
            }
            /* No clue why */
            i += EVENT_SIZE + event->len;
        }
       
    }

    return 0;
}

void onFileCreated(struct inotify_event* event) {
    printf("File %s was created. \n", event->name);
}
void onFileDeleted(struct inotify_event* event) {
    printf("File %s was deleted. \n", event->name);
}
void onFileCloseWrite(struct inotify_event* event) {
    printf("File %s was modified.\n", event->name);
}

//Handle SIGINT (Ctrl+C)
void sig_handler(int signo)
{
  if (signo == SIGINT)
    printf("\n received SIGINT\n");
    inotify_rm_watch(fileDescriptor,watchDescriptor);
    close(fileDescriptor);
    exit(0);
}


