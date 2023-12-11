#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>

void generate_data(char *buffer, size_t block_size)
{
    for (size_t i = 0; i < block_size; i++)
    {
        buffer[i] = 'A' + (random() % 26);
    }
}

void readFile(size_t block_size, int block_count, int fd, char *buf, int b_speed, int mb_speed)
{
    struct timeval start_time, end_time;
    off_t size = lseek(fd, 0, SEEK_END);
    if (size < (block_size * block_count))
    {
        printf("File size too small for given block size and count\n");
        close(fd);
        exit(1);
    }
    lseek(fd, 0, SEEK_SET);
    ssize_t bytes_read = 0;
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < block_count; i++)
    {
        bytes_read += read(fd, buf, block_size);
        if (bytes_read == -1)
        {
            perror("Error reading file");
            close(fd);
            exit(1);
        }
        else if (bytes_read == 0)
        {
            printf("break\n");
            break;
        }
    }
    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                          (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("Elapsed time: %f seconds\n", elapsed_time);

    if (mb_speed == 1)
    {
        double speed = bytes_read / (1024 * 1024 * elapsed_time);
        printf("Speed in MiB/s: %f\n", speed);
    }
    if (b_speed == 1)
    {
        double speed = bytes_read / elapsed_time;
        printf("Speed in B/s: %f\n", speed);
    }
}

void writeFile(char *buf, size_t block_size, int block_count, int fd)
{
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    while (block_count > 0)
    {
        generate_data(buf, block_size);
        write(fd, buf, block_size);
        block_count--;
    }
    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                          (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("Elapsed time: %f seconds\n", elapsed_time);
}

int main(int argc, char **argv)
{
    if (argc != 5 && argc != 6 && argc != 7)
    {
        printf("usage: ./run <filename> [-r|-w] <block_size> <block_count>\n");
        exit(1);
    }
    int mode = (strcmp(argv[2], "-r") == 0) ? O_RDONLY : O_WRONLY | O_CREAT | O_TRUNC;
    int fd = open(argv[1], mode, 0777);
    if (fd < 0)
    {
        printf("failed to open file %s\n", argv[1]);
        exit(1);
    }
    int block_size = atoi(argv[3]), block_count = atoi(argv[4]);
    int b_speed = 0, mb_speed = 0;
    if (argc == 6)
    {
        if (strcmp(argv[5], "-b") == 0)
        {
            b_speed = 1;
        }
        else if (strcmp(argv[5], "-mb") == 0)
        {
            mb_speed = 1;
        }
    }
    else if (argc == 7)
    {
        if (strcmp(argv[5], "-b") == 0)
        {
            b_speed = 1;
        }
        else if (strcmp(argv[5], "-mb") == 0)
        {
            mb_speed = 1;
        }
        if (strcmp(argv[6], "-b") == 0)
        {
            b_speed = 1;
        }
        else if (strcmp(argv[6], "-mb") == 0)
        {
            mb_speed = 1;
        }
    }
    char *buf = malloc(block_size);
    if (strcmp(argv[2], "-r") == 0)
    {
        readFile(block_size, block_count, fd, buf, b_speed, mb_speed);
    }
    else
    {
        writeFile(buf, block_size, block_count, fd);
    }
    free(buf);
    close(fd);
    return 0;
}