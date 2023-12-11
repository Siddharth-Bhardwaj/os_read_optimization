#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

size_t filesize;

void generate_data(char *buffer, size_t block_size)
{
    for (size_t i = 0; i < block_size; i++)
    {
        buffer[i] = 'A' + (random() % 26);
    }
}

void write_file(const char *filename, size_t block_size, size_t block_count)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if (fd == -1)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char *buffer = malloc(block_size);

    for (size_t i = 0; i < block_count; i++)
    {
        generate_data(buffer, block_size);

        ssize_t bytes_written = write(fd, buffer, block_size);

        if (bytes_written == -1)
        {
            perror("Error writing to file");
            free(buffer);
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    free(buffer);
    close(fd);
}

double measure_read_time(const char *filename, size_t block_size, size_t block_count)
{
    if (filesize < (block_size * block_count))
    {
        write_file(filename, block_size, block_count * 2);
        int fd = open(filename, O_RDONLY, 0777);
        filesize = lseek(fd, 0, SEEK_END);
        close(fd);
    }
    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        perror("Error opening file");
        exit(1);
    }

    char *buffer = malloc(block_size);

    struct timeval start_time, end_time;

    gettimeofday(&start_time, NULL);

    for (size_t i = 0; i < block_count; i++)
    {
        ssize_t bytes_read = read(fd, buffer, block_size);

        if (bytes_read == -1)
        {
            perror("Error reading file");
            free(buffer);
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    gettimeofday(&end_time, NULL);

    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                          (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    free(buffer);
    close(fd);

    return elapsed_time;
}

size_t find_optimal_block_count(const char *filename, size_t block_size)
{
    size_t block_count = 1;
    double elapsed_time;

    int fd = open(filename, O_RDONLY, 0666);
    if (fd > 0)
    {
        size_t size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        if (size < ((size_t)5368709120)) // 5GB
        {
            write_file(filename, 5120, 1048576); // 5GB
        }
    }
    else
    {
        write_file(filename, 5120, 1048576);
    }
    filesize = lseek(fd, 0, SEEK_END);
    close(fd);
    do
    {
        elapsed_time = measure_read_time(filename, block_size, block_count);
        block_count *= 2;
    } while (elapsed_time < 10);

    return block_count / 2;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <filename> <block_size>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    size_t block_size = atoi(argv[2]);

    size_t optimal_block_count = find_optimal_block_count(filename, block_size);

    printf("Optimal block count: %zu\n", optimal_block_count);
    printf("Optimal file size: %zu MB\n", (block_size * optimal_block_count) / (1024 * 1024));

    return EXIT_SUCCESS;
}