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
    vgs_putlog("vgs_strcmp(\"AbCd\",\"aBcD\") = %d", expect32d(vgs_strcmp("AbCd", "aBcD"), -1));
    vgs_putlog("vgs_stricmp(\"AbCd\",\"aBcD\") = %d", expect32d(vgs_stricmp("AbCd", "aBcD"), 0));

    vgs_putlog("vgs_isdigit(\'0\') ... %d", expect32d(vgs_isdigit('0'), TRUE));
    vgs_putlog("vgs_isdigit(\'9\') ... %d", expect32d(vgs_isdigit('9'), TRUE));
    vgs_putlog("vgs_isdigit(\'a\') ... %d", expect32d(vgs_isdigit('a'), FALSE));
    vgs_putlog("vgs_isdigit(\'z\') ... %d", expect32d(vgs_isdigit('z'), FALSE));
    vgs_putlog("vgs_isdigit(\'A\') ... %d", expect32d(vgs_isdigit('A'), FALSE));
    vgs_putlog("vgs_isdigit(\'Z\') ... %d", expect32d(vgs_isdigit('Z'), FALSE));
    vgs_putlog("vgs_isdigit(\'*\') ... %d", expect32d(vgs_isdigit('*'), FALSE));

    vgs_putlog("vgs_isupper(\'0\') ... %d", expect32d(vgs_isupper('0'), FALSE));
    vgs_putlog("vgs_isupper(\'9\') ... %d", expect32d(vgs_isupper('9'), FALSE));
    vgs_putlog("vgs_isupper(\'a\') ... %d", expect32d(vgs_isupper('a'), FALSE));
    vgs_putlog("vgs_isupper(\'z\') ... %d", expect32d(vgs_isupper('z'), FALSE));
    vgs_putlog("vgs_isupper(\'A\') ... %d", expect32d(vgs_isupper('A'), TRUE));
    vgs_putlog("vgs_isupper(\'Z\') ... %d", expect32d(vgs_isupper('Z'), TRUE));
    vgs_putlog("vgs_isupper(\'*\') ... %d", expect32d(vgs_isupper('*'), FALSE));

    vgs_putlog("vgs_islower(\'0\') ... %d", expect32d(vgs_islower('0'), FALSE));
    vgs_putlog("vgs_islower(\'9\') ... %d", expect32d(vgs_islower('9'), FALSE));
    vgs_putlog("vgs_islower(\'a\') ... %d", expect32d(vgs_islower('a'), TRUE));
    vgs_putlog("vgs_islower(\'z\') ... %d", expect32d(vgs_islower('z'), TRUE));
    vgs_putlog("vgs_islower(\'A\') ... %d", expect32d(vgs_islower('A'), FALSE));
    vgs_putlog("vgs_islower(\'Z\') ... %d", expect32d(vgs_islower('Z'), FALSE));
    vgs_putlog("vgs_islower(\'*\') ... %d", expect32d(vgs_islower('*'), FALSE));

    vgs_putlog("vgs_isalpha(\'0\') ... %d", expect32d(vgs_isalpha('0'), FALSE));
    vgs_putlog("vgs_isalpha(\'9\') ... %d", expect32d(vgs_isalpha('9'), FALSE));
    vgs_putlog("vgs_isalpha(\'a\') ... %d", expect32d(vgs_isalpha('a'), TRUE));
    vgs_putlog("vgs_isalpha(\'z\') ... %d", expect32d(vgs_isalpha('z'), TRUE));
    vgs_putlog("vgs_isalpha(\'A\') ... %d", expect32d(vgs_isalpha('A'), TRUE));
    vgs_putlog("vgs_isalpha(\'Z\') ... %d", expect32d(vgs_isalpha('Z'), TRUE));
    vgs_putlog("vgs_isalpha(\'*\') ... %d", expect32d(vgs_isalpha('*'), FALSE));

    vgs_putlog("vgs_isalnum(\'0\') ... %d", expect32d(vgs_isalnum('0'), TRUE));
    vgs_putlog("vgs_isalnum(\'9\') ... %d", expect32d(vgs_isalnum('9'), TRUE));
    vgs_putlog("vgs_isalnum(\'a\') ... %d", expect32d(vgs_isalnum('a'), TRUE));
    vgs_putlog("vgs_isalnum(\'z\') ... %d", expect32d(vgs_isalnum('z'), TRUE));
    vgs_putlog("vgs_isalnum(\'A\') ... %d", expect32d(vgs_isalnum('A'), TRUE));
    vgs_putlog("vgs_isalnum(\'Z\') ... %d", expect32d(vgs_isalnum('Z'), TRUE));
    vgs_putlog("vgs_isalnum(\'*\') ... %d", expect32d(vgs_isalnum('*'), FALSE));

    vgs_putlog("vgs_toupper(\'0\') ... %d", expect32d(vgs_toupper('0'), '0'));
    vgs_putlog("vgs_toupper(\'9\') ... %d", expect32d(vgs_toupper('9'), '9'));
    vgs_putlog("vgs_toupper(\'a\') ... %d", expect32d(vgs_toupper('a'), 'A'));
    vgs_putlog("vgs_toupper(\'z\') ... %d", expect32d(vgs_toupper('z'), 'Z'));
    vgs_putlog("vgs_toupper(\'A\') ... %d", expect32d(vgs_toupper('A'), 'A'));
    vgs_putlog("vgs_toupper(\'Z\') ... %d", expect32d(vgs_toupper('Z'), 'Z'));
    vgs_putlog("vgs_toupper(\'*\') ... %d", expect32d(vgs_toupper('*'), '*'));

    vgs_putlog("vgs_tolower(\'0\') ... %d", expect32d(vgs_tolower('0'), '0'));
    vgs_putlog("vgs_tolower(\'9\') ... %d", expect32d(vgs_tolower('9'), '9'));
    vgs_putlog("vgs_tolower(\'a\') ... %d", expect32d(vgs_tolower('a'), 'a'));
    vgs_putlog("vgs_tolower(\'z\') ... %d", expect32d(vgs_tolower('z'), 'z'));
    vgs_putlog("vgs_tolower(\'A\') ... %d", expect32d(vgs_tolower('A'), 'a'));
    vgs_putlog("vgs_tolower(\'Z\') ... %d", expect32d(vgs_tolower('Z'), 'z'));
    vgs_putlog("vgs_tolower(\'*\') ... %d", expect32d(vgs_tolower('*'), '*'));

    vgs_putlog("vgs_degree(0, 0, 0, 0) = %d", expect32d(vgs_degree(0, 0, 0, 0), 0));

    vgs_putlog("vgs_degree(0, 0, 0, -100) = %d", expect32d(vgs_degree(0, 0, 0, -100), 270));
    vgs_putlog("- vgs_cos(270) = %d", expect32d(vgs_cos(270), 0));
    vgs_putlog("- vgs_sin(270) = %d", expect32d(vgs_sin(270), -256));

    vgs_putlog("vgs_degree(0, 0, 100, -100) = %d", expect32d(vgs_degree(0, 0, 100, -100), 315));
    vgs_putlog("- vgs_cos(315) = %d", expect32d(vgs_cos(315), 181));
    vgs_putlog("- vgs_sin(315) = %d", expect32d(vgs_sin(315), -181));

    vgs_putlog("vgs_degree(0, 0, 100,0) = %d", expect32d(vgs_degree(0, 0, 100, 0), 0));
    vgs_putlog("- vgs_cos(0) = %d", expect32d(vgs_cos(0), 256));
    vgs_putlog("- vgs_sin(0) = %d", expect32d(vgs_sin(0), 0));

    vgs_putlog("vgs_degree(0, 0, 100, 100) = %d", expect32d(vgs_degree(0, 0, 100, 100), 45));
    vgs_putlog("- vgs_cos(45) = %d", expect32d(vgs_cos(45), 181));
    vgs_putlog("- vgs_sin(45) = %d", expect32d(vgs_sin(45), 181));

    vgs_putlog("vgs_degree(0, 0, 0, 100) = %d", expect32d(vgs_degree(0, 0, 0, 100), 90));
    vgs_putlog("- vgs_cos(90) = %d", expect32d(vgs_cos(90), 0));
    vgs_putlog("- vgs_sin(90) = %d", expect32d(vgs_sin(90), 256));

    vgs_putlog("vgs_degree(0, 0, -100, 100) = %d", expect32d(vgs_degree(0, 0, -100, 100), 135));
    vgs_putlog("- vgs_cos(135) = %d", expect32d(vgs_cos(135), -181));
    vgs_putlog("- vgs_sin(135) = %d", expect32d(vgs_sin(135), 181));

    vgs_putlog("vgs_degree(0, 0, -100, 0) = %d", expect32d(vgs_degree(0, 0, -100, 0), 180));
    vgs_putlog("- vgs_cos(180) = %d", expect32d(vgs_cos(180), -256));
    vgs_putlog("- vgs_sin(180) = %d", expect32d(vgs_sin(180), 0));

    vgs_putlog("vgs_degree(0, 0, -100, -100) = %d", expect32d(vgs_degree(0, 0, -100, -100), 225));
    vgs_putlog("- vgs_cos(225) = %d", expect32d(vgs_cos(225), -181));
    vgs_putlog("- vgs_sin(225) = %d", expect32d(vgs_sin(225), -181));

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
