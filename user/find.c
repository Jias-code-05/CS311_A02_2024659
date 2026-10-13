


#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"

#define PATHSIZE 512

static int
run_command(char *path, char **cmd, int count)
{
    char *args[MAXARG];

    for (int i = 0; i < count; i++)
        args[i] = cmd[i];

    args[count] = path;
    args[count + 1] = 0;

    int pid = fork();

    if (pid < 0) {
        fprintf(2, "find: fork failed\n");
        return -1;
    }

    if (pid == 0) {
        exec(cmd[0], args);
        fprintf(2, "find: exec %s failed\n", cmd[0]);
        exit(1);
    }

    int status = 0;

    if (wait(&status) < 0) {
        fprintf(2, "find: wait failed\n");
        return -1;
    }

    return status == 0 ? 0 : -1;
}

static int
find_path(char *path, char *target, char **cmd, int count)
{
    struct stat st;
    struct dirent de;
    char next[PATHSIZE];
    char name[DIRSIZ + 1];
    int failed = 0;

    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return -1;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return -1;
    }

    char *base = path + strlen(path);

    while (base > path && base[-1] != '/')
        base--;

    if (strcmp(base, target) == 0) {
        if (cmd == 0)
            printf("%s\n", path);
        else if (run_command(path, cmd, count) < 0)
            failed = 1;
    }

    if (st.type == T_DIR) {
        int n;

        while ((n = read(fd, &de, sizeof(de))) > 0) {
            if (n != sizeof(de)) {
                fprintf(2, "find: directory read error\n");
                failed = 1;
                break;
            }

            if (de.inum == 0)
                continue;

            memmove(name, de.name, DIRSIZ);
            name[DIRSIZ] = '\0';

            if (strcmp(name, ".") == 0 ||
                strcmp(name, "..") == 0)
                continue;

            int len = strlen(path);
            int namelen = strlen(name);

            if (len + 1 + namelen + 1 > PATHSIZE) {
                fprintf(2, "find: path too long\n");
                failed = 1;
                continue;
            }

            strcpy(next, path);
            next[len] = '/';
            strcpy(next + len + 1, name);

            if (find_path(next, target, cmd, count) < 0)
                failed = 1;
        }

        if (n < 0) {
            fprintf(2, "find: directory read failed\n");
            failed = 1;
        }
    }

    if (close(fd) < 0)
        failed = 1;

    return failed ? -1 : 0;
}

int
main(int argc, char *argv[])
{
    char **cmd = 0;
    int count = 0;

    if (argc < 3) {
        fprintf(2, "Usage: find path name [-exec command args...]\n");
        exit(1);
    }

    if (argc > 3) {
        if (strcmp(argv[3], "-exec") != 0 || argc < 5) {
            fprintf(2, "find: invalid -exec arguments\n");
            exit(1);
        }

        count = argc - 4;

        if (count > MAXARG - 2) {
            fprintf(2, "find: too many arguments\n");
            exit(1);
        }

        cmd = &argv[4];
    }

    if (find_path(argv[1], argv[2], cmd, count) < 0)
        exit(1);

    exit(0);
}