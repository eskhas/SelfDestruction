#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <fstab.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#define MAX_FILES 32                    // 32 files
#define MAX_FILE_SIZE 200 * 1024 * 1024 // 200 MB

int main(int argc, char **argv)
{

    int i = 0, operationType;

    if (argc < 2)
    {
        printf("Usage: %s -b file1 file2 ... -o output.sau\n", argv[0]);
        printf("or: %s -a archive.sau directory\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "-b") == 0)
    {
        operationType = 1;
    }
    else if (strcmp(argv[1], "-a") == 0)
    {
        if (argc != 4)
        {
            printf("Usage: %s -a archive.sau directory\n", argv[0]);
            exit(1);
        }
        if (strstr(argv[2], ".sau") == NULL)
        {
            printf("Archive file is inappropriate or corrupt!\n");
            exit(1);
        }
        operationType = 0;
    }
    else
    {
        printf("Invalid option: %s\n", argv[1]);
        exit(1);
    }

    if (operationType == 0)
    { // tarsau -a
        printf("tarsau -a\n");
        struct fileinfo
        {
            char *filename;
            short int permission;
        };
        struct fileinfo fileper[MAX_FILES];

        FILE *arch;
        DIR *dir;
        char *archive = argv[2];
        char *directory = argv[3];
        ssize_t size;
        dir = opendir(directory);
        if (!dir)
        {
            closedir(dir);
        }
        else if (ENOENT == errno)
        {
            mkdir(directory, 0700);
        }
        else
        {
            fprintf(stderr, "Couldn't open the directory %s", directory);
            exit(1);
        }
        arch = fopen(archive, "r");
        int j = 0;
        int i = fgetc(arch);
        while (i != EOF)
        {
            while (i != "\n")
            {
                if (i == '|')
                {
                    fgetc(arch);
                    if (i != '|')
                    {
                        while (i != ",")
                        {
                            char c = fgetc(arch);
                            sprintf(fileper[j].filename, "%c", c);
                        }
                    }
                }
                elseif(i == ',');
                {
                    while (i != ",")
                    {
                        char c = fgetc(arch);
                        sprintf(fileper[j].permission, "%c", c);
                    }
                }
            }
            i = fgetc(arch);
        }
    }
    else
    { // tarsau -b
        FILE *output_file, *argument;
        ssize_t size;
        struct stat buf;
        int exists;
        char *fileName;

        for (i = 1; i < argc; i++)
        {

            if (strcmp(argv[i], "-o") == 0)
            {
                fileName = argv[i + 1];
                argc -= 2;
                break;
            }
            else
            {
                fileName = "a.sau";
            }
        }

        printf("tarsau -b\n");
        output_file = fopen(fileName, "w+");
        long filesize;
        for (i = 2; i < argc; i++)
        {
            exists = stat(argv[i], &buf);
            filesize += buf.st_size;
        }
        if (argc - 2 > MAX_FILES)
        { // The number of input files cannot be more than 32
            fprintf(stderr, "Exceeded max file count:32");
        }
        if (filesize > MAX_FILE_SIZE)
        { // The total size of input files cannot exceed 200 MBytes.
            fprintf(stderr, "Exceeded max file size:200MB");
        }
        fprintf(output_file, "%-10ld", filesize);
        for (i = 2; i < argc; i++)
        {
            exists = stat(argv[i], &buf);
            fprintf(output_file, "|%s,%04o,%ld|", argv[i], buf.st_mode & 0777, buf.st_size);
        }
        fprintf(output_file, "\n");
        // The first 10 bytes hold the numerical size of the first section
        char *tmp;
        for (i = 2; i < argc; i++)
        {
            exists = stat(argv[i], &buf);
            if (exists < 0)
            {
                fprintf(stderr, "%s not found\n", argv[i]);
            }
            else
            {

                argument = fopen(argv[i], "r");
                tmp = malloc(buf.st_size);

                size = fread(tmp, 1, buf.st_size, argument);
                if (size > 0)
                {
                    fwrite(tmp, 1, buf.st_size, output_file);
                    size = fread(tmp, 1, buf.st_size, argument);
                }
                free(tmp);
                fclose(argument);
            }
        }
        fclose(output_file);
    }
    return 0;
}