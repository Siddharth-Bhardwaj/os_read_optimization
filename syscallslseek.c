#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        perror("Error opening file");
        return 1;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);

    if (file_size == -1)
    {
        perror("Error getting file size");
        close(fd);
        return 1;
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    for (off_t i = 0; i < file_size; i++)
    {
        if (lseek(fd, i, SEEK_SET) == -1)
        {
            perror("Error seeking in file");
            close(fd);
            return 1;
        }
    }
    gettimeofday(&end_time, NULL);

    close(fd);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                          (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("Elapsed time: %f seconds\n", elapsed_time);

    double speed = file_size / (1024 * 1024 * elapsed_time);
    printf("Speed in MiB/s: %f\n", speed);

    speed = file_size / elapsed_time;
    printf("Speed in B/s: %f\n", speed);

    return 0;
}