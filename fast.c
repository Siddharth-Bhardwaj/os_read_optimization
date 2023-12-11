#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/sysinfo.h>

struct ThreadArgs
{
    int fd;
    size_t file_size;
    off_t start_offset;
    size_t block_size;
    size_t total_blocks;
    size_t extra_bytes;
    unsigned int xor_result;
};

void *readFilePartition(void *args)
{
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    if (threadArgs->fd == -1)
    {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }
    unsigned int *buffer = (unsigned int *)malloc(threadArgs->block_size * sizeof(unsigned int));
    if (!buffer)
    {
        perror("Error allocating memory");
        close(threadArgs->fd);
        exit(EXIT_FAILURE);
    }

    unsigned int xor = 0;
    off_t offset = threadArgs->start_offset;
    for (size_t i = 0; i < threadArgs->total_blocks; i++)
    {
        size_t bytes_read = pread(threadArgs->fd, buffer, threadArgs->block_size, offset);
        if (bytes_read == -1)
        {
            perror("Error reading file");
            free(buffer);
            close(threadArgs->fd);
            exit(EXIT_FAILURE);
        }
        offset += bytes_read;
        for (size_t j = 0; j < bytes_read; j++)
        {
            xor ^= buffer[j];
        }
    }

    if (threadArgs->extra_bytes > 0)
    {
        unsigned int *extra_buffer = (unsigned int *)malloc(threadArgs->extra_bytes);
        if (!extra_buffer)
        {
            perror("Error allocating memory");
            close(threadArgs->fd);
            exit(EXIT_FAILURE);
        }
        off_t extra_offset = threadArgs->file_size - threadArgs->extra_bytes;
        size_t extra_read = pread(threadArgs->fd, extra_buffer, threadArgs->extra_bytes, extra_offset);
        if (extra_read == -1)
        {
            perror("Error reading extra bytes");
            free(extra_buffer);
            close(threadArgs->fd);
            exit(EXIT_FAILURE);
        }
        for (size_t j = 0; j < extra_read / sizeof(unsigned int); j++)
        {
            xor ^= extra_buffer[j];
        }
        free(extra_buffer);
    }

    threadArgs->xor_result = xor;

    free(buffer);
    pthread_exit(NULL);
}

void readFile(char *filename, size_t file_size)
{
    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    int thread_count = sysconf(_SC_NPROCESSORS_ONLN);
    while (thread_count > 1 && thread_count % 4 != 0)
    {
        thread_count--;
    }
    struct timeval start_time, end_time;
    struct sysinfo info;
    sysinfo(&info);
    size_t optimal_block_size = file_size / thread_count;
    optimal_block_size = optimal_block_size - (optimal_block_size % 4);
    size_t available_memory = info.freeram;
    if (optimal_block_size > (available_memory / thread_count))
    {
        while (optimal_block_size > (available_memory / thread_count))
        {
            optimal_block_size = optimal_block_size / 2;
        }
    }
    size_t extra_bytes = file_size % (optimal_block_size * thread_count);

    pthread_t threads[thread_count];
    struct ThreadArgs threadArgs[thread_count];

    for (int i = 0; i < thread_count; i++)
    {
        threadArgs[i].fd = fd;
        threadArgs[i].block_size = optimal_block_size;
        if (extra_bytes > 0 && i == (thread_count - 1))
        {
            threadArgs[i].extra_bytes = extra_bytes;
        }
        else
        {
            threadArgs[i].extra_bytes = 0;
        }
        threadArgs[i].file_size = file_size;
        threadArgs[i].total_blocks = file_size / (optimal_block_size * thread_count);
        threadArgs[i].start_offset = i * (optimal_block_size * threadArgs[i].total_blocks);
    }

    gettimeofday(&start_time, NULL);
    for (int i = 0; i < thread_count; i++)
    {
        if (pthread_create(&threads[i], NULL, readFilePartition, (void *)&threadArgs[i]) != 0)
        {
            perror("Error creating thread");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < thread_count; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            perror("Error joining thread");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }
    gettimeofday(&end_time, NULL);

    double elapsed_time_concurrent = (end_time.tv_sec - start_time.tv_sec) +
                                     (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    unsigned int xor_result = 0;
    for (int i = 0; i < thread_count; i++)
    {
        xor_result ^= threadArgs[i].xor_result;
    }

    printf("xor result: %08x\n", xor_result);
    printf("elapsed time: %f seconds\n", elapsed_time_concurrent);
    printf("speed in MiB/s: %f\n", file_size / (1024 * 1024 * elapsed_time_concurrent));

    close(fd);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *filename = argv[1];
    int fd = open(filename, O_RDONLY);

    if (fd < 0)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    size_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    close(fd);

    readFile(filename, file_size);

    return 0;
}