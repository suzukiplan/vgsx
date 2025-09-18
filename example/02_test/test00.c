#include <vgs.h>
#include <log.h>

int main(int argc, char* argv)
{
    int32_t ret = 123;
    vgs_putlog("call vgs_exit with exit code: %d", ret);
    vgs_exit(ret);
    vgs_putlog("return 456 (this message will not shown)");
    return 456;
}
