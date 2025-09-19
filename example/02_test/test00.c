#include <vgs.h>
#include <log.h>

int main(int argc, char* argv)
{
    char buf[80];

    const char* text = "This is test string.";
    uint32_t length = vgs_strlen(text);
    vgs_putlog("length of \"%s\" = %u", text, length);
    vgs_memset(buf, 0x01, sizeof(buf));
    vgs_memcpy(buf, text, length + 1);

    int32_t ret = 0;
    for (int i = 0; i < sizeof(buf); i++) {
        if (buf[i] == 0x01) {
            ret++;
        }
    }

    vgs_putlog("call vgs_exit with exit code: %d", ret);
    vgs_exit(ret);
    vgs_putlog("return 456 (this message will not shown)");
    return 456;
}
