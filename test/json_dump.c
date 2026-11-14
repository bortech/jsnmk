/*
 * Emits one JSON document per line, produced by the library.
 * validate_json.py feeds this output to Python's json parser.
 *
 * Exit code is non-zero as soon as any jsnmk call fails, so a partially
 * written document can never reach the parser unnoticed.
 */
#include "jsnmk.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define DOC_CAP 1024

static char doc[DOC_CAP];

static jsnmk_buf_t doc_init(void)
{
    memset(doc, 0, sizeof(doc));
    return (jsnmk_buf_t){ .p = doc, .size = sizeof(doc) };
}

static int doc_emit(const jsnmk_buf_t *b)
{
    size_t used = (size_t)(b->p - doc);
    if (used == 0 || used > sizeof(doc)) {
        fprintf(stderr, "jsnmk_json_dump: invalid document length %zu\n", used);
        return 1;
    }
    printf("%.*s\n", (int)used, doc);
    return 0;
}

#define CHECK(CALL)                                                       \
    do {                                                                  \
        if (!(CALL)) {                                                    \
            fprintf(stderr, "jsnmk_json_dump: call failed: %s\n", #CALL); \
            return 1;                                                     \
        }                                                                 \
    } while (0)

static int dump_basic(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_bool(&b, "bool", true));
    CHECK(jsnmk_key_cstr(&b, "name", "John"));
    CHECK(jsnmk_key_f32(&b, "value", 3.14f));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_all_scalars(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_bool(&b, "run", true));
    CHECK(jsnmk_key_bool(&b, "stop", false));
    CHECK(jsnmk_key_null(&b, "empty"));
    CHECK(jsnmk_key_cstr(&b, "name", "John"));
    CHECK(jsnmk_key_fstr(&b, "time", "%02u:%02u:%02u", 21, 2, 13));
    CHECK(jsnmk_key_obj_begin(&b, "obj"));
    CHECK(jsnmk_key_i32(&b, "i32", -42));
    CHECK(jsnmk_key_u32(&b, "u32", 69));
    CHECK(jsnmk_key_f32(&b, "f32", 3.14f));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_key_arr_begin(&b, "list"));
    CHECK(jsnmk_bool(&b, true));
    CHECK(jsnmk_bool(&b, false));
    CHECK(jsnmk_null(&b));
    CHECK(jsnmk_i32(&b, -42));
    CHECK(jsnmk_u32(&b, 69));
    CHECK(jsnmk_f32(&b, 3.14f));
    CHECK(jsnmk_fstr(&b, "%d.%d.%d", 1, 2, 3));
    CHECK(jsnmk_cstr(&b, "Colorado"));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_mixed_document(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_obj_begin(&b, "meta"));
    CHECK(jsnmk_key_i32(&b, "v", 1));
    CHECK(jsnmk_key_arr_begin(&b, "tags"));
    CHECK(jsnmk_cstr(&b, "a"));
    CHECK(jsnmk_cstr(&b, "b"));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_key_arr_begin(&b, "items"));
    CHECK(jsnmk_i32(&b, 1));
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_bool(&b, "ok", true));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_key_bool(&b, "done", false));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_array_of_scalars(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_null(&b));
    CHECK(jsnmk_bool(&b, true));
    CHECK(jsnmk_bool(&b, false));
    CHECK(jsnmk_i32(&b, -42));
    CHECK(jsnmk_u32(&b, 4294967295u));
    CHECK(jsnmk_f32(&b, 3.14f));
    CHECK(jsnmk_cstr(&b, "hi"));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_empty_object(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_empty_array(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_empty_containers(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_obj_begin(&b, "obj"));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_key_arr_begin(&b, "arr"));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_integer_bounds(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_i32(&b, "min", INT32_MIN));
    CHECK(jsnmk_key_i32(&b, "max", INT32_MAX));
    CHECK(jsnmk_key_u32(&b, "umax", UINT32_MAX));
    CHECK(jsnmk_key_i32(&b, "zero", 0));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_f32(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_f32(&b, "a", 1.0f));
    CHECK(jsnmk_key_f32(&b, "b", 0.5f));
    CHECK(jsnmk_key_f32(&b, "c", 3.14f));
    CHECK(jsnmk_key_f32(&b, "d", 1000000.0f));
    CHECK(jsnmk_key_f32(&b, "e", -0.0f));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_empty_key(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_i32(&b, "", 1));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_nested_array(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_i32(&b, 1));
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_i32(&b, 2));
    CHECK(jsnmk_i32(&b, 3));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_array_with_object(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_i32(&b, 1));
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_i32(&b, "a", 2));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_array_of_objects(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_i32(&b, "a", 1));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_i32(&b, "b", 2));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_array_of_arrays(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_i32(&b, 1));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_i32(&b, 2));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_array_with_mixed_containers(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_i32(&b, "a", 1));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_i32(&b, 2));
    CHECK(jsnmk_arr_begin(&b));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_object_with_array_of_objects(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_arr_begin(&b, "items"));
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_i32(&b, "id", 1));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_i32(&b, "id", 2));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_arr_end(&b));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

static int dump_utf8(void)
{
    jsnmk_buf_t b = doc_init();
    CHECK(jsnmk_obj_begin(&b));
    CHECK(jsnmk_key_cstr(&b, "greeting", "Привет, мир!"));
    CHECK(jsnmk_key_cstr(&b, "path", "/tmp/файл"));
    CHECK(jsnmk_obj_end(&b));
    CHECK(jsnmk_finalyze(&b));
    return doc_emit(&b);
}

/* Keep the number of emitted documents in sync with the expected count
 * passed to validate_json.py from test/CMakeLists.txt (currently 17). */
int main(void)
{
    if (dump_basic() != 0) return 1;
    if (dump_all_scalars() != 0) return 1;
    if (dump_mixed_document() != 0) return 1;
    if (dump_array_of_scalars() != 0) return 1;
    if (dump_empty_object() != 0) return 1;
    if (dump_empty_array() != 0) return 1;
    if (dump_empty_containers() != 0) return 1;
    if (dump_integer_bounds() != 0) return 1;
    if (dump_f32() != 0) return 1;
    if (dump_empty_key() != 0) return 1;
    if (dump_nested_array() != 0) return 1;
    if (dump_array_with_object() != 0) return 1;
    if (dump_utf8() != 0) return 1;
    if (dump_array_of_objects() != 0) return 1;
    if (dump_array_of_arrays() != 0) return 1;
    if (dump_array_with_mixed_containers() != 0) return 1;
    if (dump_object_with_array_of_objects() != 0) return 1;
    return 0;
}
