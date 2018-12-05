#include <stdio.h> // printf()
#include <stdlib.h> // exit()
#if __STDC_VERSION__ >= 201112L
/* C11 support */
#include <stdnoreturn.h>
#endif

#include <png.h>
#if (PNG_LIBPNG_VER_MAJOR == 1) && (PNG_LIBPNG_VER_MINOR > 2) && (PNG_LIBPNG_VER_MINOR <= 6)
#include <pngpriv.h>
#endif

enum exit_code {
    SUCCESS = 0,
    FAILURE = 1,
    PNG_CREATE_WRITE_FAILED = 2,
    PNG_CREATE_INFO_FAILED = 3,
    LIBPNG_ERROR = 4,
};

static int in_filter = PNG_NO_FILTERS;

#if __STDC_VERSION__ >= 201112L
noreturn
#endif
void user_error_fn(png_structp png_ptr, png_const_charp error_cstr)
{
    (void)png_ptr;
    printf("filter 0x%0x libpng error: %s\n", in_filter, error_cstr);
    exit(LIBPNG_ERROR);
}

void user_warning_fn(png_structp png_ptr, png_const_charp warning_cstr)
{
    (void)png_ptr;
    printf("filter 0x%0x libpng warning: %s\n", in_filter, warning_cstr);
}

void user_write_data(png_structp png_ptr, png_bytep data, size_t length)
{
    (void)png_ptr;
    (void)data;
    (void)length;
}

void user_flush_data(png_structp png_ptr)
{
    (void)png_ptr;
}

struct libpng
{
    png_structp png_ptr;
    png_infop info_ptr;
};

int libpng_write_create(struct libpng * png_write)
{
    png_write->png_ptr = NULL;
    png_write->info_ptr = NULL;

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
        NULL, user_error_fn, user_warning_fn);
    if (!png_ptr)
    {
        return PNG_CREATE_WRITE_FAILED;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        return PNG_CREATE_INFO_FAILED;
    }
    png_set_write_fn(png_ptr, NULL, user_write_data, user_flush_data);

#if (PNG_LIBPNG_VER_MAJOR == 1) && (PNG_LIBPNG_VER_MINOR == 6)
    png_set_benign_errors(png_ptr, 1); /* warning for filters 5-7 */
#endif /* libpng16 */

    png_write->png_ptr = png_ptr;
    png_write->info_ptr = info_ptr;

    return SUCCESS;
}

void libpng_write_destroy(struct libpng * png_write)
{
    png_destroy_write_struct(&png_write->png_ptr, &png_write->info_ptr);
    png_write->png_ptr = NULL;
    png_write->info_ptr = NULL;
}

int main()
{
    printf("libpng version %s\n", PNG_LIBPNG_VER_STRING);

    /* {input, expected_result} values pairs. Expected values - png12 output */
    const int in_expected_pairs[][2] = {
        { PNG_FILTER_VALUE_NONE, PNG_FILTER_NONE },
        { PNG_FILTER_VALUE_SUB, PNG_FILTER_SUB },
#ifdef EXTRA_PNG_FILTERS_CHECKS
        { PNG_FILTER_VALUE_UP, PNG_FILTER_UP },
        { PNG_FILTER_VALUE_AVG, PNG_FILTER_AVG },
        { PNG_FILTER_VALUE_PAETH, PNG_FILTER_PAETH },
        /* undocumented value,  libpng <...>: Unknown row filter for method 0 */
        { 0x5, 0 },
        { 0x6, 0 },
        { 0x7, 0 },
        /* single filter mask */
        { PNG_FILTER_NONE, PNG_FILTER_NONE },
        { PNG_FILTER_SUB , PNG_FILTER_SUB },
        /* mask and value mix, png12 gives 009 -> 009 */
        { PNG_FILTER_NONE | PNG_FILTER_VALUE_SUB, PNG_FILTER_NONE | PNG_FILTER_VALUE_SUB },
        { PNG_FILTER_NONE | PNG_FILTER_VALUE_UP, PNG_FILTER_NONE | PNG_FILTER_VALUE_UP },
        /* with bit in next byte */
        { 0x100 | PNG_FILTER_VALUE_NONE , PNG_FILTER_NONE },
        { 0x100 | PNG_FILTER_VALUE_SUB , PNG_FILTER_SUB },
        /* libpng <...>: Unknown row filter for method 0 */
        { 0x100 | 0x5, 0 },
        { 0x100 | 0x6, 0 },
        { 0x100 | 0x7, 0 },
        { 0x100 | PNG_FILTER_NONE , PNG_FILTER_NONE },
        { 0x100 | PNG_FILTER_SUB , PNG_FILTER_SUB },
        /* mix */
        { 0x100 | PNG_FILTER_NONE | PNG_FILTER_VALUE_SUB,
            PNG_FILTER_NONE | PNG_FILTER_VALUE_SUB },
#endif // EXTRA_PNG_FILTERS_CHECKS
        { PNG_NO_FILTERS, PNG_FILTER_NONE }, // == 0, == PNG_FILTER_VALUE_NONE
    };

    const int tests_count = sizeof(in_expected_pairs) / sizeof(in_expected_pairs[0]);

    const int method = PNG_FILTER_TYPE_BASE; /* "Currently, the only valid for "method" is 0" */

    struct libpng png_write = {NULL, NULL};
#ifdef REUSE_PNG_STRUCT
    printf("One png_structp for all png_set_filter() calls.\n");
#endif

    printf(" in v1.2 out (hex values, v1.2 - libpng12 output value)\n");
    for (int i=0; i < tests_count; ++i)
    {
        if (!png_write.png_ptr)
        {
            int status = libpng_write_create(&png_write);
            if (status != SUCCESS)
            {
                printf("Failed libpng_write_create(), return code %d", status);
                return status;
            }
        }
        const png_structp png_ptr = png_write.png_ptr;
        const int in = in_expected_pairs[i][0];
        in_filter = in; // hack for png errors and warnings reporting
        const int expected = in_expected_pairs[i][1];
        png_set_filter(png_ptr, method, in);
#if (PNG_LIBPNG_VER_MAJOR == 1) && (PNG_LIBPNG_VER_MINOR <= 6)
        const int out = png_ptr->do_filter;
#else
        /* In libpng17 png_set_filter() is macro, set PNG_SF_GET bit to get value */
        const int out = png_setting(png_ptr, PNG_SF_GET | PNG_SW_COMPRESS_filters,
            method, 0xfff); // arbitrary value
#endif
        printf("%3x  %02x  %02x %s\n", in, expected, out, (expected==out) ? "" : "!=");
#ifndef REUSE_PNG_STRUCT
        libpng_write_destroy(&png_write);
#endif
    }
#ifdef REUSE_PNG_STRUCT
        libpng_write_destroy(&png_write);
#endif

    return SUCCESS;
}
