#include <vgs.h>

void log(const char* text, int32_t num)
{
    char d32[12];
    vgs_console_print(text);
    vgs_d32str(d32, num);
    vgs_console_println(d32);
}

int main(int argc, char* argv)
{
    int32_t ret = 123;
    log("call vgs_exit with exit code: ", ret);
    vgs_exit(ret);
    vgs_console_println("return 456 (this message will not shown)");
    return 456;
}
