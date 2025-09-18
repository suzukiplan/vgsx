#include <vgs.h>
#include <stdarg.h>

void putlog(const char* format, ...)
{
    char d32[12];
    va_list arg;
    va_start(arg, format);
    while (*format) {
        if (format[0] == '%' && format[1] == 'd') {
            vgs_d32str(d32, va_arg(arg, int32_t));
            vgs_console_print(d32);
            format += 2;
        } else if (format[0] == '%' && format[1] == 'u') {
            vgs_u32str(d32, va_arg(arg, uint32_t));
            vgs_console_print(d32);
            format += 2;
        } else if (format[0] == '%' && format[1] == 's') {
            vgs_console_print(va_arg(arg, const char*));
            format += 2;
        } else {
            VGS_OUT_CONSOLE = *format;
            format++;
        }
    }
    VGS_OUT_CONSOLE = '\n';
    va_end(arg);
}

int main(int argc, char* argv)
{
    int32_t ret = 123;
    putlog("call vgs_exit with exit code: %d (%s)", ret, "FOO");
    vgs_exit(ret);
    putlog("return 456 (this message will not shown)");
    return 456;
}
