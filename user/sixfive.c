#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAXNUM 1024

static int
separator(char c)
{
    return strchr(" -\r\t\n./,", c) != 0;
}

static int
process(int fd)
{
    char c;
    char num[MAXNUM];
    int len = 0;
    int valid = 1;
    int mod = 0;
    int n;

    while ((n = read(fd, &c, 1)) == 1) {
        if (separator(c)) {
            if (len > 0 && valid &&
                (mod % 5 == 0 || mod % 6 == 0)) {
                num[len] = '\0';

int start = 0;
while (num[start] == '0' && num[start + 1] != '\0')
    start++;

printf("%s\n", num + start);
            }

            len = 0;
            valid = 1;
            mod = 0;
        } else {
            if (c < '0' || c > '9') {
                valid = 0;
            } else {
                mod = (mod * 10 + c - '0') % 30;
            }

            if (len < MAXNUM - 1)
                num[len++] = c;
            else
                valid = 0;
        }
    }

    if (n < 0) {
        fprintf(2, "sixfive: read failed\n");
        return -1;
    }

    if (len > 0 && valid &&
        (mod % 5 == 0 || mod % 6 == 0)) {
     num[len] = '\0';

int start = 0;
while (num[start] == '0' && num[start + 1] != '\0')
    start++;

printf("%s\n", num + start);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int fd;
    int failed = 0;

    if (argc == 1) {
        if (process(0) < 0)
            exit(1);
    } else {
        for (int i = 1; i < argc; i++) {
            fd = open(argv[i], O_RDONLY);

            if (fd < 0) {
                fprintf(2, "sixfive: cannot open %s\n", argv[i]);
                failed = 1;
                continue;
            }

            if (process(fd) < 0)
                failed = 1;

            if (close(fd) < 0)
                failed = 1;
        }
    }

    exit(failed);
}