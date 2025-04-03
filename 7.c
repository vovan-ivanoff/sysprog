#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

void print_colored(const char *name, const char *color)
{
    printf("%s%s\033[0m", color, name);
}

void list_files(const char *path)
{
    struct dirent *entry;
    struct stat file_stat;
    DIR *dp = opendir(path);

    if (dp == NULL)
    {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
        {
            continue; // Skip hidden files
        }

        char full_path[BUFSIZ];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (lstat(full_path, &file_stat) == -1)
        {
            perror("stat");
            continue;
        }

        if (S_ISDIR(file_stat.st_mode))
        {
            printf("dir: ");
            print_colored(entry->d_name, "\033[1;34m"); // Blue for directories
        }
        else if (S_ISLNK(file_stat.st_mode))
        {
            printf("link: ");
            print_colored(entry->d_name, "\033[1;36m"); // Cyan for symlinks
        }
        else if (S_ISCHR(file_stat.st_mode))
        {
            printf("char: ");
            print_colored(entry->d_name, "\033[1;33m"); // Yellow for char-oriented
        }
        else if (S_ISBLK(file_stat.st_mode))
        {
            printf("block: ");
            print_colored(entry->d_name, "\033[1;32m"); // Green for Block-oriented
        }
        else if (S_ISFIFO(file_stat.st_mode))
        {
            printf("fifo: ");
            print_colored(entry->d_name, "\033[1;35m"); // Magenta for fifo
        }
        else
        {
            printf("regular: ");
            print_colored(entry->d_name, "\033[0m"); // Default color for files
        }

        printf(", First Inode: %lu\n", file_stat.st_ino);
    }

    closedir(dp);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <directory> [<directory> ...]\n", argv[0]);
        return -1;
    }

    for (int i = 1; i < argc; i++)
    {
        printf("Directory: %s\n", argv[i]);
        list_files(argv[i]);
    }

    return 0;
}