// Library Header Files
#include <cstdio>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <pwd.h>
#include <grp.h>
#include <ctime>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>

// Imported Files
#include "../include/ls_command.h"


void print_permissions(mode_t mode)
{
    // File type
    if (S_ISDIR(mode))
        printf("d");
    else if (S_ISLNK(mode))
        printf("l");
    else
        printf("-");

    // Owner
    printf("%c", (mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (mode & S_IXUSR) ? 'x' : '-');

    // Group
    printf("%c", (mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (mode & S_IXGRP) ? 'x' : '-');

    // Others
    printf("%c", (mode & S_IROTH) ? 'r' : '-');
    printf("%c", (mode & S_IWOTH) ? 'w' : '-');
    printf("%c", (mode & S_IXOTH) ? 'x' : '-');
}


void print_long_entry(const char* path, const char* name)
{
    char full_path[4096];

    int written = snprintf(
        full_path,
        sizeof(full_path),
        "%s/%s",
        path,
        name
    );

    if (written < 0 || written >= (int)sizeof(full_path))
    {
        fprintf(stderr, "ls error: path too long\n");
        return;
    }

    struct stat file_stat;

    if (lstat(full_path, &file_stat) == -1)
    {
        perror("ls error");
        return;
    }

    print_permissions(file_stat.st_mode);
    printf(" %lu", (unsigned long)file_stat.st_nlink);

    struct passwd *pw = getpwuid(file_stat.st_uid);

    if (pw != NULL)
        printf(" %s", pw->pw_name);
    else
        printf(" %d", file_stat.st_uid);

    struct group *gr = getgrgid(file_stat.st_gid);

    if (gr != NULL)
        printf(" %s", gr->gr_name);
    else
        printf(" %d", file_stat.st_gid);

    printf(" %lld", (long long)file_stat.st_size);
    char time_buffer[64];

    struct tm *time_info = localtime(&file_stat.st_mtime);
    if (time_info != NULL)
    {
        strftime(
            time_buffer,
            sizeof(time_buffer),
            "%b %d %H:%M",
            time_info
        );

        printf(" %s", time_buffer);
    }
    printf(" %s\n", name);
}


void list_dir(
    const char* path,
    bool is_hidden_visible,
    bool list_format
)
{
    DIR *dir = opendir(path);

    if (dir == NULL)
    {
        perror("ls error");
        return;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!is_hidden_visible && entry->d_name[0] == '.')
            continue;

        if (list_format)
        {
            print_long_entry(path, entry->d_name);
        }
        else
        {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
}


void manage_ls(struct Command *cmd)
{
    bool is_hidden_visible = false;
    bool list_format = false;

    char* target[AT_MOST_ARGS];
    int target_count = 0;

    for (int i = 1; i < cmd->arg_count; i++)
    {
        if (cmd->args[i][0] == '-')
        {
            if (cmd->args[i][1] == '\0')
            {
                fprintf(stderr, "ls: invalid option -- '-'\n");
                return;
            }

            int j = 1;

            while (cmd->args[i][j] != '\0')
            {
                char flag = cmd->args[i][j];

                if (flag == 'l')
                {
                    list_format = true;
                }
                else if (flag == 'a')
                {
                    is_hidden_visible = true;
                }
                else
                {
                    fprintf(stderr, "ls: invalid option -- '%c'\n", flag);
                    return;
                }

                j++;
            }
        }
        else
        {
            if (target_count >= AT_MOST_ARGS)
            {
                fprintf(stderr, "ls: too many arguments\n");
                return;
            }

            target[target_count] = cmd->args[i];
            target_count++;
        }
    }

    if (target_count == 0)
    {
        list_dir(".", is_hidden_visible, list_format);
        return;
    }

    for (int i = 0; i < target_count; i++)
    {
        struct stat target_stat;

        if (lstat(target[i], &target_stat) == -1)
        {
            fprintf(stderr, "ls: cannot access '%s': ", target[i]);
            perror("");
            continue;
        }

        if (!S_ISDIR(target_stat.st_mode))
        {
            if (list_format)
                print_long_entry(".", target[i]);
            else
                printf("%s\n", target[i]);

            continue;
        }

        if (target_count > 1)
            printf("%s:\n", target[i]);

        list_dir(
            target[i],
            is_hidden_visible,
            list_format
        );

        if (i < target_count - 1)
            printf("\n");
    }
}