#include <stdarg.h>

#include <console.h>
#include <printf.h>
#include <string.h>

#define MAX_DIGITS  10  /* 4294967295 */


/**
 * Output 'count' copies of 'pad'; nothing if 'count' is zero or negative.
 * Returns the number of characters written.
 */
static int print_pad(
    char pad,
    int count)
{
    int n = 0;

    for (; count > 0; count--, n++)
        putc(pad);

    return n;
}


/**
 * Output a string, right-aligned in 'width' columns. Returns the number of
 * characters written.
 */
static int print_str(
    const char *s,
    int width)
{
    int len = strlen(s);
    int n = print_pad(' ', width - len);

    puts(s);

    return n + len;
}


/**
 * Output 'val' in the given base, right-aligned in 'width' columns and
 * padded with 'pad'. For a negative number the caller passes the magnitude
 * in 'val' and sets 'negative'. Returns the number of characters written.
 */
static int print_num(
    u32 val,
    u32 base,
    bool negative,
    int width,
    char pad)
{
    static const char digits[] = "0123456789abcdef";
    char buf[MAX_DIGITS];
    int len = 0;
    int n = 0;

    /* digits come out lowest first, so collect them and print backwards */
    do {
        buf[len++] = digits[val % base];
        val /= base;
    } while (val);

    if (negative)
        width--;

    /* "-0042" but "  -42": the sign goes before zeros and after spaces */
    if (negative && pad == '0')
        putc('-');
    n += print_pad(pad, width - len);
    if (negative && pad != '0')
        putc('-');
    if (negative)
        n++;

    n += len;
    while (len)
        putc(buf[--len]);

    return n;
}


/**
 * See printf.h for the supported conversions.
 */
int printf(
    const char *fmt,
    ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);

    for (; *fmt; fmt++) {
        char pad = ' ';
        int width = 0;

        if (*fmt != '%') {
            putc(*fmt);
            n++;
            continue;
        }
        fmt++;

        if (*fmt == '0') {
            pad = '0';
            fmt++;
        }
        while (*fmt >= '0' && *fmt <= '9')
            width = width * 10 + (*fmt++ - '0');
        if (*fmt == 'l')
            fmt++;

        switch (*fmt) {
        case 'c':
            putc((char)va_arg(ap, int));
            n++;
            break;
        case 's':
            n += print_str(va_arg(ap, const char *), width);
            break;
        case 'd': {
            int val = va_arg(ap, int);

            /* negate as unsigned so INT_MIN does not overflow */
            if (val < 0)
                n += print_num(-(u32)val, 10, true, width, pad);
            else
                n += print_num(val, 10, false, width, pad);
            break;
        }
        case 'u':
            n += print_num(va_arg(ap, u32), 10, false, width, pad);
            break;
        case 'x':
            n += print_num(va_arg(ap, u32), 16, false, width, pad);
            break;
        case 'p':
            puts("0x");
            n += 2 + print_num((uintptr_t)va_arg(ap, void *), 16, false,
                               2 * sizeof(void *), '0');
            break;
        case '%':
            putc('%');
            n++;
            break;
        case '\0':
            /* format ends in a lone '%' */
            va_end(ap);
            return n;
        default:
            /* unknown conversion: show it as written */
            putc('%');
            putc(*fmt);
            n += 2;
            break;
        }
    }

    va_end(ap);

    return n;
}
