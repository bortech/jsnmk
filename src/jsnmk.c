#include "jsnmk.h"

#include <stdarg.h>
#include <stddef.h>

#ifdef USE_STDIO_SNPRINTF
#include <stdio.h>
#define snprintf_ snprintf
#define vsnprintf_ vsnprintf
#else
extern int  snprintf_(char* s, size_t count, const char* format, ...);
extern int vsnprintf_(char* s, size_t count, const char* format, va_list arg);
#endif

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

    int n = snprintf_(b->p, b->size, "\"%s\":", name);
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

/*
 * Precondition: a matching begin call has already written at least one
 * byte, so b->p points past the start of the caller's buffer.
 *
 * Every end call leaves a trailing comma; the enclosing end consumes it,
 * and jsnmk_finalyze() removes the final one.
 *
 * If this precondition is violated, for example by calling an end
 * function without a matching begin on a fresh buffer, the check below
 * reads b->p[-1] and the behavior is undefined. See jsnmk.h.
 */
__attribute__((warn_unused_result))
static bool jsnmk_end(jsnmk_buf_t *b, char c)
{
    if (*(b->p - 1) == ',') {
        *(b->p - 1) = c;
    } else {
        if (!jsnmk_ch(b, c))
            return false;
    }
    return jsnmk_ch(b, ',');
}

__attribute__((warn_unused_result))
static bool jsnmk_vfmt(jsnmk_buf_t *b, const char* fmt, va_list args)
{
    int n = vsnprintf_(b->p, b->size, fmt, args);
    if (n < 0 || (uint32_t)n >= b->size) {
        b->size = 0;
        return false;
    }

    b->p += n, b->size -= n;
    return true;
}

__attribute__ ((__format__ (__printf__, 2, 3)))
__attribute__((warn_unused_result))
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
    return jsnmk_begin(b, 0, '{');
}

bool jsnmk_obj_end(jsnmk_buf_t *b)
{
    return jsnmk_end(b, '}');
}

bool jsnmk_key_arr_begin(jsnmk_buf_t *b, const char *key)
{
    return jsnmk_begin(b, key, '[');
}

bool jsnmk_arr_begin(jsnmk_buf_t *b)
{
    return jsnmk_begin(b, 0, '[');
}

bool jsnmk_arr_end(jsnmk_buf_t *b)
{
    return jsnmk_end(b, ']');
}

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

/*
 * Precondition: at least one byte has been written, so b->p points past
 * the start of the buffer.
 */
bool jsnmk_finalyze(jsnmk_buf_t *b)
{
    if (b->p[-1] != ',')
        return false;
    b->p--;
    b->size++;
    *b->p = 0;
    return true;
}

