#include "greatest.h"
#include "jsnmk.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ---------- helpers ---------- */

#define FIXTURE_CAP 256

typedef struct {
    char buf[FIXTURE_CAP];
    jsnmk_buf_t b;
} fixture_t;

static void fixture_init(fixture_t *f)
{
    memset(f->buf, 0, sizeof(f->buf));
    f->b = (jsnmk_buf_t){ .p = f->buf, .size = sizeof(f->buf) };
}

/* Check the exact output: length, bytes, and the b.size accounting. */
#define ASSERT_JSON(F, EXPECTED)                                          \
    do {                                                                  \
        size_t used_ = (size_t)((F)->b.p - (F)->buf);                     \
        ASSERT_EQ(strlen(EXPECTED), used_);                               \
        ASSERT_STRN_EQ((EXPECTED), (F)->buf, used_);                      \
        ASSERT_EQ((uint32_t)(sizeof((F)->buf) - used_), (F)->b.size);     \
    } while (0)

/* ---------- smoke: original example from tests.c ---------- */

TEST test1(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_bool(&f.b, "bool", true));
    ASSERT(jsnmk_key_cstr(&f.b, "name", "John"));
    ASSERT(jsnmk_key_f32(&f.b, "value", 3.14));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{\"bool\":true,\"name\":\"John\",\"value\":3.14}");
    PASS();
}

/* ---------- containers ---------- */

TEST empty_object(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{}");
    PASS();
}

TEST empty_array(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_arr_begin(&f.b));
    ASSERT(jsnmk_arr_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "[]");
    PASS();
}

TEST nested_empty_containers(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_obj_begin(&f.b, "obj"));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_key_arr_begin(&f.b, "arr"));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{\"obj\":{},\"arr\":[]}");
    PASS();
}

TEST object_with_all_scalar_types(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_null(&f.b, "n"));
    ASSERT(jsnmk_key_bool(&f.b, "t", true));
    ASSERT(jsnmk_key_bool(&f.b, "f", false));
    ASSERT(jsnmk_key_i32(&f.b, "i", -42));
    ASSERT(jsnmk_key_u32(&f.b, "u", 4294967295u));
    ASSERT(jsnmk_key_f32(&f.b, "x", 3.14f));
    ASSERT(jsnmk_key_cstr(&f.b, "s", "John"));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f,
        "{\"n\":null,\"t\":true,\"f\":false,\"i\":-42,"
        "\"u\":4294967295,\"x\":3.14,\"s\":\"John\"}");
    PASS();
}

TEST array_of_scalars(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_arr_begin(&f.b));
    ASSERT(jsnmk_null(&f.b));
    ASSERT(jsnmk_bool(&f.b, true));
    ASSERT(jsnmk_bool(&f.b, false));
    ASSERT(jsnmk_i32(&f.b, -42));
    ASSERT(jsnmk_u32(&f.b, 4294967295u));
    ASSERT(jsnmk_f32(&f.b, 3.14f));
    ASSERT(jsnmk_cstr(&f.b, "hi"));
    ASSERT(jsnmk_arr_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "[null,true,false,-42,4294967295,3.14,\"hi\"]");
    PASS();
}

TEST array_of_objects(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_arr_begin(&f.b));
    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_i32(&f.b, "a", 1));
    ASSERT(jsnmk_obj_end(&f.b));       /* leaves ',' */
    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_i32(&f.b, "b", 2));
    ASSERT(jsnmk_obj_end(&f.b));       /* leaves ',' */
    ASSERT(jsnmk_arr_end(&f.b));       /* consumes the last ',' */
    ASSERT(jsnmk_finalyze(&f.b));

    ASSERT_JSON(&f, "[{\"a\":1},{\"b\":2}]");
    PASS();
}

TEST array_of_arrays(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_arr_begin(&f.b));
    ASSERT(jsnmk_arr_begin(&f.b));
    ASSERT(jsnmk_i32(&f.b, 1));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_arr_begin(&f.b));
    ASSERT(jsnmk_i32(&f.b, 2));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_finalyze(&f.b));

    ASSERT_JSON(&f, "[[1],[2]]");
    PASS();
}

TEST array_with_mixed_containers(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_arr_begin(&f.b));
    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_i32(&f.b, "a", 1));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_i32(&f.b, 2));
    ASSERT(jsnmk_arr_begin(&f.b));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_finalyze(&f.b));

    ASSERT_JSON(&f, "[{\"a\":1},2,[],{}]");
    PASS();
}

TEST object_with_array_of_objects(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_arr_begin(&f.b, "items"));
    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_i32(&f.b, "id", 1));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_i32(&f.b, "id", 2));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_finalyze(&f.b));

    ASSERT_JSON(&f, "{\"items\":[{\"id\":1},{\"id\":2}]}");
    PASS();
}

TEST nested_objects_as_siblings(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_obj_begin(&f.b, "a"));
    ASSERT(jsnmk_key_i32(&f.b, "x", 1));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_key_obj_begin(&f.b, "b"));
    ASSERT(jsnmk_key_i32(&f.b, "y", 2));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{\"a\":{\"x\":1},\"b\":{\"y\":2}}");
    PASS();
}

TEST nested_array_in_array(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_arr_begin(&f.b));
    ASSERT(jsnmk_i32(&f.b, 1));
    ASSERT(jsnmk_arr_begin(&f.b));
    ASSERT(jsnmk_i32(&f.b, 2));
    ASSERT(jsnmk_i32(&f.b, 3));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_arr_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "[1,[2,3]]");
    PASS();
}

TEST object_with_array_and_array_with_object(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_arr_begin(&f.b, "list"));
    ASSERT(jsnmk_bool(&f.b, true));
    ASSERT(jsnmk_null(&f.b));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_key_obj_begin(&f.b, "last"));
    ASSERT(jsnmk_key_i32(&f.b, "id", 7));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{\"list\":[true,null],\"last\":{\"id\":7}}");
    PASS();
}

TEST mixed_document(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_obj_begin(&f.b, "meta"));
    ASSERT(jsnmk_key_i32(&f.b, "v", 1));
    ASSERT(jsnmk_key_arr_begin(&f.b, "tags"));
    ASSERT(jsnmk_cstr(&f.b, "a"));
    ASSERT(jsnmk_cstr(&f.b, "b"));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_key_arr_begin(&f.b, "items"));
    ASSERT(jsnmk_i32(&f.b, 1));
    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_bool(&f.b, "ok", true));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_arr_end(&f.b));
    ASSERT(jsnmk_key_bool(&f.b, "done", false));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f,
        "{\"meta\":{\"v\":1,\"tags\":[\"a\",\"b\"]},"
        "\"items\":[1,{\"ok\":true}],\"done\":false}");
    PASS();
}

/* ---------- scalars / formatting ---------- */

TEST top_level_values_are_finalized(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_null(&f.b));
    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "null");

    fixture_init(&f);
    ASSERT(jsnmk_bool(&f.b, false));
    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "false");

    fixture_init(&f);
    ASSERT(jsnmk_i32(&f.b, -42));
    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "-42");

    fixture_init(&f);
    ASSERT(jsnmk_u32(&f.b, 69));
    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "69");

    fixture_init(&f);
    ASSERT(jsnmk_f32(&f.b, 3.14f));
    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "3.14");

    fixture_init(&f);
    ASSERT(jsnmk_cstr(&f.b, "hi"));
    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "\"hi\"");

    fixture_init(&f);
    ASSERT(jsnmk_fstr(&f.b, "%d.%d.%d", 1, 2, 3));
    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "\"1.2.3\"");
    PASS();
}

TEST integer_bounds(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_i32(&f.b, "min", INT32_MIN));
    ASSERT(jsnmk_key_i32(&f.b, "max", INT32_MAX));
    ASSERT(jsnmk_key_u32(&f.b, "umax", UINT32_MAX));
    ASSERT(jsnmk_key_i32(&f.b, "zero", 0));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f,
        "{\"min\":-2147483648,\"max\":2147483647,"
        "\"umax\":4294967295,\"zero\":0}");
    PASS();
}

TEST f32_uses_g_format(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_f32(&f.b, "a", 1.0f));
    ASSERT(jsnmk_key_f32(&f.b, "b", 0.5f));
    ASSERT(jsnmk_key_f32(&f.b, "c", 3.14f));
    ASSERT(jsnmk_key_f32(&f.b, "d", 1000000.0f));
    ASSERT(jsnmk_key_f32(&f.b, "e", -0.0f));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{\"a\":1,\"b\":0.5,\"c\":3.14,\"d\":1e+06,\"e\":-0}");
    PASS();
}

TEST fstr_formats_with_printf_args(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_fstr(&f.b, "time", "%02u:%02u:%02u", 21, 2, 13));
    ASSERT(jsnmk_key_fstr(&f.b, "pair", "%d-%d", 1, 2));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{\"time\":\"21:02:13\",\"pair\":\"1-2\"}");
    PASS();
}

TEST empty_key_is_allowed(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_key_i32(&f.b, "", 1));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{\"\":1}");
    PASS();
}

/* ---------- buffer limits / error handling ---------- */

TEST finalize_rolls_back_and_nullterminate(void)
{
    char buf[32];
    memset(buf, 0xAA, sizeof(buf));
    jsnmk_buf_t b = { .p = buf, .size = sizeof(buf) };

    ASSERT(jsnmk_obj_begin(&b));
    ASSERT(jsnmk_obj_end(&b));       /* "{}," */
    ASSERT(jsnmk_finalyze(&b));      /* logical "{}" */

    ASSERT_EQ((ptrdiff_t)2, b.p - buf);
    ASSERT_STRN_EQ("{}", buf, 2);
    ASSERT_EQ((uint32_t)(sizeof(buf) - 2), b.size);
    /* Only p/size are rolled back; the comma stays in memory. */
    ASSERT_EQ('\0', buf[2]);
    for (size_t i = 3; i < sizeof(buf); i++)
        ASSERT_EQ((unsigned char)0xAA, (unsigned char)buf[i]);

    PASS();
}

TEST exact_buffer_size_needs_one_spare_byte(void)
{
    /* "{\"a\":1}" is 7 bytes, but snprintf needs room for '\0' while
       formatting the value, and the end call leaves a trailing comma that
       jsnmk_finalyze() removes afterwards. */
    char seven[7];
    jsnmk_buf_t b7 = { .p = seven, .size = sizeof(seven) };
    ASSERT(jsnmk_obj_begin(&b7));
    ASSERT_FALSE(jsnmk_key_i32(&b7, "a", 1));
    ASSERT_EQ((uint32_t)0, b7.size);

    char eight[8];
    jsnmk_buf_t b8 = { .p = eight, .size = sizeof(eight) };
    ASSERT(jsnmk_obj_begin(&b8));
    ASSERT(jsnmk_key_i32(&b8, "a", 1));
    ASSERT(jsnmk_obj_end(&b8));
    ASSERT(jsnmk_finalyze(&b8));
    ASSERT_EQ((ptrdiff_t)7, b8.p - eight);
    ASSERT_STRN_EQ("{\"a\":1}", eight, 7);
    ASSERT_EQ((uint32_t)1, b8.size);

    PASS();
}

TEST overflow_reports_failure_and_freezes_buffer(void)
{
    char small[24];
    jsnmk_buf_t b = { .p = small, .size = sizeof(small) };

    ASSERT(jsnmk_obj_begin(&b));
    ASSERT_FALSE(jsnmk_key_cstr(&b, "longkey", "0123456789abcdefghij"));
    ASSERT_EQ((uint32_t)0, b.size);

    /* Once b.size is 0 every further call fails, so callers must check
       every return value (or stop on the first failure). */
    ASSERT_FALSE(jsnmk_key_bool(&b, "x", true));
    ASSERT_FALSE(jsnmk_obj_end(&b));
    /* finalyze() cannot repair a failed document either. */
    ASSERT_FALSE(jsnmk_finalyze(&b));

    PASS();
}

TEST finalize_twice_is_rejected(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT(jsnmk_obj_end(&f.b));
    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{}");

    /* The document is already finalized: no trailing comma remains. */
    ASSERT_FALSE(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{}");

    PASS();
}

TEST null_key_is_rejected_without_writing(void)
{
    fixture_t f;
    fixture_init(&f);

    ASSERT(jsnmk_obj_begin(&f.b));
    ASSERT_FALSE(jsnmk_key_bool(&f.b, NULL, true));
    ASSERT(jsnmk_obj_end(&f.b));

    ASSERT(jsnmk_finalyze(&f.b));
    ASSERT_JSON(&f, "{}");
    PASS();
}

/* ---------- runner ---------- */

SUITE(smoke)
{
    RUN_TEST(test1);
}

SUITE(containers)
{
    RUN_TEST(empty_object);
    RUN_TEST(empty_array);
    RUN_TEST(nested_empty_containers);
    RUN_TEST(object_with_all_scalar_types);
    RUN_TEST(array_of_scalars);
    RUN_TEST(array_of_objects);
    RUN_TEST(array_of_arrays);
    RUN_TEST(array_with_mixed_containers);
    RUN_TEST(object_with_array_of_objects);
    RUN_TEST(nested_objects_as_siblings);
    RUN_TEST(nested_array_in_array);
    RUN_TEST(object_with_array_and_array_with_object);
    RUN_TEST(mixed_document);
}

SUITE(scalars)
{
    RUN_TEST(top_level_values_are_finalized);
    RUN_TEST(integer_bounds);
    RUN_TEST(f32_uses_g_format);
    RUN_TEST(fstr_formats_with_printf_args);
    RUN_TEST(empty_key_is_allowed);
}

SUITE(buffer_limits)
{
    RUN_TEST(finalize_rolls_back_and_nullterminate);
    RUN_TEST(exact_buffer_size_needs_one_spare_byte);
    RUN_TEST(overflow_reports_failure_and_freezes_buffer);
    RUN_TEST(finalize_twice_is_rejected);
    RUN_TEST(null_key_is_rejected_without_writing);
}

GREATEST_MAIN_DEFS();

int main(int argc, char *argv[])
{
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(smoke);
    RUN_SUITE(containers);
    RUN_SUITE(scalars);
    RUN_SUITE(buffer_limits);

    GREATEST_MAIN_END();
}
