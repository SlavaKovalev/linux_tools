/*
https://man7.org/linux/man-pages/man7/fanotify.7.html
The second program is an example of fanotify being used with a
group that identifies objects by file handles.  The program marks
the filesystem object that is passed as a command-line argument
and waits until an event of type FAN_CREATE has occurred.  The
event mask indicates which type of filesystem object—either a
file or a directory—was created.  Once all events have been read
from the buffer and processed accordingly, the program simply
terminates.

The following shell sessions show two different invocations of
this program, with different actions performed on a watched
object.

The first session shows a mark being placed on /home/user.  This
is followed by the creation of a regular file,
/home/user/testfile.txt.  This results in a FAN_CREATE event
being generated and reported against the file's parent watched
directory object and with the created file name.  Program
execution ends once all events captured within the buffer have
been processed.

    # ./fanotify_fid /home/user
    Listening for events.
    FAN_CREATE (file created):
            Directory /home/user has been modified.
            Entry 'testfile.txt' is not a subdirectory.
    All events processed successfully. Program exiting.

    $ touch /home/user/testfile.txt              # In another terminal

The second session shows a mark being placed on /home/user.  This
is followed by the creation of a directory, /home/user/testdir.
This specific action results in a FAN_CREATE event being
generated and is reported with the FAN_ONDIR flag set and with
the created directory name.

    # ./fanotify_fid /home/user
    Listening for events.
    FAN_CREATE | FAN_ONDIR (subdirectory created):
            Directory /home/user has been modified.
            Entry 'testdir' is a subdirectory.
    All events processed successfully. Program exiting.

    $ mkdir -p /home/user/testdir          # In another terminal
*/

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fanotify.h>
#include <unistd.h>

#define BUF_SIZE 256

int
main(int argc, char **argv)
{
    int fd, ret, event_fd, mount_fd;
    ssize_t len, path_len;
    char path[PATH_MAX];
    char procfd_path[PATH_MAX];
    char events_buf[BUF_SIZE];
    struct file_handle *file_handle;
    struct fanotify_event_metadata *metadata;
    struct fanotify_event_info_fid *fid;
    const char *file_name;
    struct stat sb;

    if (argc != 2) {
        fprintf(stderr, "Invalid number of command line arguments.\n");
        exit(EXIT_FAILURE);
    }

    mount_fd = open(argv[1], O_DIRECTORY | O_RDONLY);
    if (mount_fd == -1) {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    /* Create an fanotify file descriptor with FAN_REPORT_DFID_NAME as
       a flag so that program can receive fid events with directory
       entry name. */

    fd = fanotify_init(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME, 0);
    if (fd == -1) {
        perror("fanotify_init");
        exit(EXIT_FAILURE);
    }

    /* Place a mark on the filesystem object supplied in argv[1]. */

    ret = fanotify_mark(fd, FAN_MARK_ADD | FAN_MARK_ONLYDIR,
                        FAN_CREATE | FAN_ONDIR,
                        AT_FDCWD, argv[1]);
    if (ret == -1) {
        perror("fanotify_mark");
        exit(EXIT_FAILURE);
    }

    printf("Listening for events.\n");

    /* Read events from the event queue into a buffer. */

    len = read(fd, events_buf, sizeof(events_buf));
    if (len == -1 && errno != EAGAIN) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    /* Process all events within the buffer. */

    for (metadata = (struct fanotify_event_metadata *) events_buf;
            FAN_EVENT_OK(metadata, len);
            metadata = FAN_EVENT_NEXT(metadata, len)) {
        fid = (struct fanotify_event_info_fid *) (metadata + 1);
        file_handle = (struct file_handle *) fid->handle;

        /* Ensure that the event info is of the correct type. */

        if (fid->hdr.info_type == FAN_EVENT_INFO_TYPE_FID ||
            fid->hdr.info_type == FAN_EVENT_INFO_TYPE_DFID) {
            file_name = NULL;
        } else if (fid->hdr.info_type == FAN_EVENT_INFO_TYPE_DFID_NAME) {
            file_name = file_handle->f_handle +
                        file_handle->handle_bytes;
        } else {
            fprintf(stderr, "Received unexpected event info type.\n");
            exit(EXIT_FAILURE);
        }

        if (metadata->mask == FAN_CREATE)
            printf("FAN_CREATE (file created):\n");

        if (metadata->mask == (FAN_CREATE | FAN_ONDIR))
            printf("FAN_CREATE | FAN_ONDIR (subdirectory created):\n");

     /* metadata->fd is set to FAN_NOFD when the group identifies
        objects by file handles.  To obtain a file descriptor for
        the file object corresponding to an event you can use the
        struct file_handle that's provided within the
        fanotify_event_info_fid in conjunction with the
        open_by_handle_at(2) system call.  A check for ESTALE is
        done to accommodate for the situation where the file handle
        for the object was deleted prior to this system call. */

        event_fd = open_by_handle_at(mount_fd, file_handle, O_RDONLY);
        if (event_fd == -1) {
            if (errno == ESTALE) {
                printf("File handle is no longer valid. "
                        "File has been deleted\n");
                continue;
            } else {
                perror("open_by_handle_at");
                exit(EXIT_FAILURE);
            }
        }

        snprintf(procfd_path, sizeof(procfd_path), "/proc/self/fd/%d",
                event_fd);

        /* Retrieve and print the path of the modified dentry. */

        path_len = readlink(procfd_path, path, sizeof(path) - 1);
        if (path_len == -1) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }

        path[path_len] = '\0';
        printf("\tDirectory '%s' has been modified.\n", path);

        if (file_name) {
            ret = fstatat(event_fd, file_name, &sb, 0);
            if (ret == -1) {
                if (errno != ENOENT) {
                    perror("fstatat");
                    exit(EXIT_FAILURE);
                }
                printf("\tEntry '%s' does not exist.\n", file_name);
            } else if ((sb.st_mode & S_IFMT) == S_IFDIR) {
                printf("\tEntry '%s' is a subdirectory.\n", file_name);
            } else {
                printf("\tEntry '%s' is not a subdirectory.\n",
                        file_name);
            }
        }

        /* Close associated file descriptor for this event. */

        close(event_fd);
    }

    printf("All events processed successfully. Program exiting.\n");
    exit(EXIT_SUCCESS);
}