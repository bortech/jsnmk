#include "greatest.h"
#include "jsnmk.h"
#include <stdint.h>

TEST test1(void) {
    char buf[256] = {0};
    jsnmk_buf_t b = {.p = buf, .size = sizeof(buf)};

    jsnmk_obj_begin(&b);
    jsnmk_key_bool(&b, "bool", true);
    jsnmk_key_cstr(&b, "name", "John");
    jsnmk_key_f32(&b, "value", 3.14);
    jsnmk_obj_end(&b);

    uint32_t total_bytes_written = b.p - buf;
    ASSERT_STRN_EQ("{\"bool\":true,\"name\":\"John\",\"value\":3.14}", buf, total_bytes_written);

    PASS();
}

GREATEST_MAIN_DEFS();
int main(int argc, char *argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_TEST(test1);

    GREATEST_MAIN_END();
}
