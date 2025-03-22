#include <locale.h> // used for unicode printing
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "uci.h"
#include "movegen.h"
#include "utils.h" // includes <stdio.h>

#define WIDE_PRINT false
#define MAX_STR_SIZE 32

int main(void) {
    char *str = (char*)malloc(MAX_STR_SIZE * sizeof(char));

    if (WIDE_PRINT) {
        setlocale(LC_CTYPE, ""); // used for unicode printing
        // ensure stdout is wide-oriented
        if (fwide(stdout, 0) <= 0) {
            fwide(stdout, 1);
        }
    }
    init_move_lookup_tables();

    while (1) {
        fgets(str, MAX_STR_SIZE, stdin);

        if (strcmp(str, "uci\n") == 0)
            return uci();
    }
}
