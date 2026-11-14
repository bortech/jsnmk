#include "jsnmk.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

__attribute__((warn_unused_result))
static bool jsnmk_ch(jsnmk_buf_t *b, char c)
{
    if (b->size == 0)
        return false;
    *b->p++ = c;
    b->size--;
    return true;
}

__attribute__((warn_unused_result))
static bool jsnmk_name(jsnmk_buf_t *b, const char *name)
{
    if (!name)
        return false;

    int n = snprintf(b->p, b->size, "\"%s\":", name);
    if (n < 0 || (uint32_t)n >= b->size) {
        b->size = 0;
        return false;
    }
    b->p += n, b->size -= n;
    return true;
}

__attribute__((warn_unused_result))
static bool jsnmk_begin(jsnmk_buf_t *b, const char *name, char c)
{
    if (name)
        if (!jsnmk_name(b, name))
            return false;
    return jsnmk_ch(b, c);
}

__attribute__((warn_unused_result))
static bool jsnmk_end(jsnmk_buf_t *b, bool named, char c)
{
    if (*(b->p - 1) == ',') {
        *(b->p - 1) = c;
    } else {
        if (!jsnmk_ch(b, c))
            return false;
    }
    if (named)
        return jsnmk_ch(b, ',');
    return true;
}

static bool jsnmk_vfmt(jsnmk_buf_t *b, const char* fmt, va_list args)
{
    int n = vsnprintf(b->p, b->size, fmt, args);
    if (n < 0 || (uint32_t)n >= b->size) {
        b->size = 0;
        return false;
    }

    b->p += n, b->size -= n;
    return true;
}

__attribute__ ((__format__ (__printf__, 2, 3)))
static bool jsnmk_fmt_value(jsnmk_buf_t *b, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    bool r = jsnmk_vfmt(b, fmt, args);
    va_end(args);
    return r;
}

bool jsnmk_key_obj_begin(jsnmk_buf_t *b, const char *key)
{
    return jsnmk_begin(b, key, '{');
}

bool jsnmk_obj_begin(jsnmk_buf_t *b)
{
    return jsnmk_begin(b, NULL, '{');
}

bool jsnmk_key_obj_end(jsnmk_buf_t *b)
{
    return jsnmk_end(b, true, '}');
}

bool jsnmk_obj_end(jsnmk_buf_t *b)
{
    return jsnmk_end(b, false, '}');
}

bool jsnmk_key_arr_begin(jsnmk_buf_t *b, const char *key)
{
    return jsnmk_begin(b, key, '[');
}

bool jsnmk_key_arr_end(jsnmk_buf_t *b)
{
    return jsnmk_end(b, true, ']');
}

bool jsnmk_arr_begin(jsnmk_buf_t *b)
{
    return jsnmk_begin(b, NULL, '[');
}

bool jsnmk_arr_end(jsnmk_buf_t *b)
{
    return jsnmk_end(b, false, ']');
}

__attribute__ ((__format__ (__printf__, 2, 3)))
bool jsnmk_fstr(jsnmk_buf_t *b, const char *fmt, ...)
{
    if (!jsnmk_ch(b, '"'))
        return false;

    va_list args;
    va_start(args, fmt);
    bool r = jsnmk_vfmt(b, fmt, args);
    va_end(args);
    if (!r)
        return false;

    if (!jsnmk_ch(b, '"'))
        return false;

    return jsnmk_ch(b, ',');
}

bool jsnmk_cstr(jsnmk_buf_t *b, const char *value)
{
    return jsnmk_fmt_value(b, "\"%s\",", value);
}

bool jsnmk_bool(jsnmk_buf_t *b, bool value)
{
    return jsnmk_fmt_value(b, "%s,", value ? "true" : "false");
}

bool jsnmk_null(jsnmk_buf_t *b)
{
    return jsnmk_fmt_value(b, "null,");
}

bool jsnmk_i32(jsnmk_buf_t *b, int32_t value)
{
    return jsnmk_fmt_value(b, "%d,", value);
}

bool jsnmk_u32(jsnmk_buf_t *b, uint32_t value)
{
    return jsnmk_fmt_value(b, "%u,", value);
}

bool jsnmk_f32(jsnmk_buf_t *b, float value)
{
    return jsnmk_fmt_value(b, "%g,", value);
}

bool jsnmk_key_null(jsnmk_buf_t *b, const char *key)
{
    if (!jsnmk_name(b, key))
        return false;
    return jsnmk_null(b);
}

bool jsnmk_key_bool(jsnmk_buf_t *b, const char *key, bool value)
{
    if (!jsnmk_name(b, key))
        return false;
    return jsnmk_bool(b, value);
}

bool jsnmk_key_i32(jsnmk_buf_t *b, const char *key, int32_t value)
{
    if (!jsnmk_name(b, key))
        return false;
    return jsnmk_i32(b, value);
}

bool jsnmk_key_u32(jsnmk_buf_t *b, const char *key, uint32_t value)
{
    if (!jsnmk_name(b, key))
        return false;
    return jsnmk_u32(b, value);
}

bool jsnmk_key_f32(jsnmk_buf_t *b, const char *key, float value)
{
    if (!jsnmk_name(b, key))
        return false;
    return jsnmk_f32(b, value);
}

bool jsnmk_key_cstr(jsnmk_buf_t *b, const char *key, const char *value)
{
    if (!jsnmk_name(b, key))
        return false;
    return jsnmk_cstr(b, value);
}

__attribute__ ((__format__ (__printf__, 3, 4)))
bool jsnmk_key_fstr(jsnmk_buf_t *b, const char *key, const char *fmt, ...)
{
    if (!jsnmk_name(b, key))
        return false;

    if (!jsnmk_ch(b, '"'))
        return false;

    va_list args;
    va_start(args, fmt);
    bool r = jsnmk_vfmt(b, fmt, args);
    va_end(args);
    if (!r)
        return false;

    if (!jsnmk_ch(b, '"'))
        return false;

    return jsnmk_ch(b, ',');
}

#ifdef COMPILE_TEST

int main()
{
    char buf[256] = {0};
    jsnmk_buf_t b = {.p = buf, .size = sizeof(buf)};

    jsnmk_obj_begin(&b);
        jsnmk_key_bool(&b, "run", true);
        jsnmk_key_bool(&b, "stop", false);
        jsnmk_key_null(&b, "empty");
        jsnmk_key_cstr(&b, "name", "John");
        jsnmk_key_fstr(&b, "time", "%02u:%02u:%02u", 21, 02, 13);
        jsnmk_key_obj_begin(&b, "obj");
            jsnmk_key_i32(&b, "i32", -42);
            jsnmk_key_u32(&b, "u32", 69);
            jsnmk_key_f32(&b, "f32", 3.14);
        jsnmk_key_obj_end(&b);
        jsnmk_key_arr_begin(&b, "list");
            jsnmk_bool(&b, true);
            jsnmk_bool(&b, false);
            jsnmk_null(&b);
            jsnmk_i32(&b, -42);
            jsnmk_u32(&b, 69);
            jsnmk_f32(&b, 3.14);
            jsnmk_fstr(&b, "%d.%d.%d", 1, 2, 3);
            jsnmk_cstr(&b, "Colorado");
        jsnmk_key_arr_end(&b);
    jsnmk_obj_end(&b);

    printf("%.*s\n", (int)(sizeof(buf) - b.size), buf);
    return 0;
}

#endif // COMPILE_TEST
