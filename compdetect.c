#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("The configuration file was not specified.\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}