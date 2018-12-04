#include <stdio.h> // printf()
#include <stdlib.h> // exit()

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

int main()
{
    printf("libpng version %s\n", PNG_LIBPNG_VER_STRING);

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

    /* {input, expected_result} values pairs. Expected values - png12 output */
    const int in_expected_pairs[][2] = {
        { PNG_FILTER_VALUE_NONE, PNG_FILTER_NONE },
        { PNG_FILTER_VALUE_SUB, PNG_FILTER_SUB },
#ifdef EXTRA_PNG_FILTERS_CHECKS
        { PNG_FILTER_VALUE_UP, PNG_FILTER_UP },
        { PNG_FILTER_VALUE_AVG, PNG_FILTER_AVG },
        { PNG_FILTER_VALUE_PAETH, PNG_FILTER_PAETH },
        /* undocumented value,  libpng <...>: Unknown row filter for method 0 */
        { 0x5, 0x9 },
        { 0x6, 0x9 },
        { 0x7, 0x9 },
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
        { 0x100 | 0x5, 0x9 },
        { 0x100 | 0x6, 0x9 },
        { 0x100 | 0x7, 0x9 },
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
#if (PNG_LIBPNG_VER_MAJOR == 1) && (PNG_LIBPNG_VER_MINOR <= 6)
    const png_byte * do_filter_ptr = &(png_ptr->do_filter);
#endif

    printf(" in v1.2 out (hex values, v1.2 - libpng12 output value)\n");
    for (int i=0; i < tests_count; ++i)
    {
        const int in = in_expected_pairs[i][0];
        in_filter = in;
        const int expected = in_expected_pairs[i][1];
        png_set_filter(png_ptr, method, in);
#if (PNG_LIBPNG_VER_MAJOR == 1) && (PNG_LIBPNG_VER_MINOR <= 6)
        const int out = *do_filter_ptr;
#else
        /* In libpng17 png_set_filter() is macro, set PNG_SF_GET bit to get value */
        const int out = png_setting(png_ptr, PNG_SF_GET | PNG_SW_COMPRESS_filters,
            method, 0xfff); // arbitrary value
#endif
        printf("%3x  %02x  %02x %s\n", in, expected, out, (expected==out) ? "" : "!=");
    }

    return SUCCESS;
}
