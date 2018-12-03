#include <stdio.h> // printf()

#include <png.h>

enum exit_code {
    SUCCESS = 0,
    FAILURE = 1,
};

int main()
{
    printf("libpng version %s\n", PNG_LIBPNG_VER_STRING);

    return SUCCESS;
}
