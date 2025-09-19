#include <vgs.h>
#include <log.h>

const char* expect(const char* actual, const char* expect)
{
    if (0 == vgs_strcmp(actual, expect)) {
        return expect;
    }
    vgs_putlog("Detected an unexpected string: \"%s\" (expected: \"%s\")", actual, expect);
    vgs_exit(-1);
    return NULL;
}

int32_t expect32d(int32_t actual, int32_t expect)
{
    if (actual == expect) {
        return expect;
    }
    vgs_putlog("Detected an unexpected value: %d (expected: %d)", actual, expect);
    vgs_exit(-1);
    return -1;
}

int main(int argc, char* argv)
{
    char buf[80];

    const char* text = "This is test string.";
    uint32_t length = vgs_strlen(text);
    vgs_putlog("length of \"%s\" = %u", text, length);
    vgs_memset(buf, 0x01, sizeof(buf));
    vgs_memcpy(buf, text, length + 1);
    vgs_putlog("vgs_strchr(\'t\') ... %s", expect(vgs_strchr(text, 't'), "test string."));
    vgs_putlog("vgs_strrchr(\'t\') ... %s", expect(vgs_strrchr(text, 't'), "tring."));
    vgs_putlog("vgs_strcmp(\"100\",\"101\") = %d", expect32d(vgs_strcmp("100", "101"), -1));
    vgs_putlog("vgs_strcmp(\"102\",\"101\") = %d", expect32d(vgs_strcmp("102", "101"), 1));
    vgs_putlog("vgs_strcmp(\"102\",\"1000\") = %d", expect32d(vgs_strcmp("102", "1000"), 1));
    vgs_putlog("vgs_strcmp(\"103\",\"103\") = %d", expect32d(vgs_strcmp("103", "103"), 0));
    vgs_putlog("vgs_strncmp(\"1031\",\"1032\",3) = %d", expect32d(vgs_strncmp("1031", "1032", 3), 0));
    vgs_putlog("vgs_strncmp(\"1031\",\"1032\",4) = %d", expect32d(vgs_strncmp("1031", "1032", 4), -1));
    vgs_putlog("vgs_strstr(\"%s\",\"is t\") ... %s", text, expect(vgs_strstr(text, "is t"), "is test string."));

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
