#define _GNU_SOURCE     /* Needed to get O_LARGEFILE definition */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <unistd.h>

/* Read all available fanotify events from the file descriptor 'fd'. */
static void
handle_events(int fd)
{
    const struct fanotify_event_metadata *metadata;
    struct fanotify_event_metadata buf[200];
    ssize_t len;
    char path[PATH_MAX];
    ssize_t path_len;
    char procfd_path[PATH_MAX];
    struct fanotify_response response;

    /* Loop while events can be read from fanotify file descriptor. */
    for (;;) {

        /* Read some events. */
        len = read(fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN && errno != ETXTBSY) {
            //printf("read error %d\n", errno);
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* Check if end of available data reached. */
        if (len <= 0)
            break;

        /* Point to the first event in the buffer. */
        metadata = buf;

        /* Loop over all events in the buffer. */
        while (FAN_EVENT_OK(metadata, len)) {
            /* Check that run-time and compile-time structures match. */
            if (metadata->vers != FANOTIFY_METADATA_VERSION) {
                fprintf(stderr,
                        "Mismatch of fanotify metadata version.\n");
                exit(EXIT_FAILURE);
            }

            /* metadata->fd contains either FAN_NOFD, indicating a
               queue overflow, or a file descriptor (a nonnegative
               integer). Here, we simply ignore queue overflow. */
            if (metadata->fd >= 0) {

                /* Handle open permission event. */

                if (metadata->mask & FAN_OPEN_PERM) {

                    /* Allow file to be opened. */
                    response.fd = metadata->fd;
                    response.response = FAN_ALLOW;
                    write(fd, &response, sizeof(response));
                }

                if (metadata->mask & FAN_OPEN_EXEC_PERM) {
                    snprintf(procfd_path, sizeof(procfd_path), "/proc/self/fd/%d", metadata->fd);
                    path_len = readlink(procfd_path, path, sizeof(path) - 1);
                    if (path_len != -1) {
                        path[path_len] = '\0';
                        printf("FAN_OPEN_PERM: file %s\n", path);
                    }
                    else {
                        printf("FAN_OPEN+PREM: error");
                        continue;
                    }
                    /* Allow file to be opened. */
                    if (strstr(path, "virus"))
                        response.response = FAN_DENY;
                    else
                        response.response = FAN_ALLOW;
                    response.fd = metadata->fd;
                    write(fd, &response, sizeof(response));
                }
                
                if (metadata->mask & FAN_ACCESS_PERM) {
                    snprintf(procfd_path, sizeof(procfd_path), "/proc/self/fd/%d", metadata->fd);
                    path_len = readlink(procfd_path, path, sizeof(path) - 1);
                    if (path_len != -1) {
                        path[path_len] = '\0';
                        printf("FAN_OPEN_PERM: file %s\n", path);
                    }
                    else {
                        printf("FAN_OPEN+PREM: error");
                        continue;
                    }
                    /* Allow file to be opened. */
                    if (strstr(path, "virus"))
                        response.response = FAN_DENY;
                    else
                        response.response = FAN_ALLOW;
                    response.fd = metadata->fd;
                    write(fd, &response, sizeof(response));
                }

                /* Handle closing of writable file event. */
                if (metadata->mask & FAN_CLOSE_WRITE)
                    printf("FAN_CLOSE_WRITE: ");

                /* Retrieve and print pathname of the accessed file. */
                snprintf(procfd_path, sizeof(procfd_path),
                         "/proc/self/fd/%d", metadata->fd);
                path_len = readlink(procfd_path, path,
                                    sizeof(path) - 1);
                if (path_len == -1) {
                    perror("readlink");
                    exit(EXIT_FAILURE);
                }

                path[path_len] = '\0';
                printf("File %s\n", path);

                /* Close the file descriptor of the event. */
                close(metadata->fd);
            }

            /* Advance to next event. */
            metadata = FAN_EVENT_NEXT(metadata, len);
        }
    }
}
