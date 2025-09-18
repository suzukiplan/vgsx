#include <vgs.h>

int main(int argc, char* argv)
{
    vgs_console_print("call vgs_exit(123)\n");
    vgs_exit(123);
    vgs_console_print("return 456 (this message will not shown)\n");
    return 456;
}
