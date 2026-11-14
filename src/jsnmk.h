#ifndef JSNMK_H
#define JSNMK_H

#include <stdint.h>
#include <stdbool.h>

/*
 * jsnmk - JSON writer for caller-provided fixed-size buffers.
 *
 * Buffer contract:
 *   - jsnmk_buf_t is caller-owned. There is no allocation, no hidden
 *     state beyond {p, size}, and the output is not NUL-terminated.
 *   - The written length is (b->p - start); b->size is the remaining
 *     capacity.
 *
 * Call contract:
 *   - Every jsnmk_*_end() call must be paired with the matching
 *     jsnmk_*_begin() call. Calling an end function without a matching
 *     begin violates this contract: the output cannot be valid JSON,
 *     and if nothing has been written yet the call is undefined
 *     behavior because the implementation inspects the byte before
 *     b->p (to replace a trailing comma with '}' or ']').
 *   - Values are written with a trailing comma; the matching end call
 *     turns it into the closing bracket.
 *   - Check every return value. On failure b->size becomes 0 and all
 *     subsequent calls fail; partially written bytes are not rolled back.
 */

typedef struct jsnmk_buf {
    char *p;
    uint32_t size;
} jsnmk_buf_t;

bool jsnmk_obj_begin(jsnmk_buf_t *b);
bool jsnmk_obj_end(jsnmk_buf_t *b);        /* requires jsnmk_obj_begin() */

bool jsnmk_key_obj_begin(jsnmk_buf_t *b, const char *key);
bool jsnmk_key_obj_end(jsnmk_buf_t *b);    /* requires jsnmk_key_obj_begin() */

bool jsnmk_arr_begin(jsnmk_buf_t *b);
bool jsnmk_arr_end(jsnmk_buf_t *b);        /* requires jsnmk_arr_begin() */

bool jsnmk_key_arr_begin(jsnmk_buf_t *b, const char *key);
bool jsnmk_key_arr_end(jsnmk_buf_t *b);    /* requires jsnmk_key_arr_begin() */

bool jsnmk_null(jsnmk_buf_t *b);
bool jsnmk_bool(jsnmk_buf_t *b, bool value);
bool jsnmk_i32(jsnmk_buf_t *b, int32_t value);
bool jsnmk_u32(jsnmk_buf_t *b, uint32_t value);
bool jsnmk_f32(jsnmk_buf_t *b, float value);
bool jsnmk_cstr(jsnmk_buf_t *b, const char *value);
bool jsnmk_fstr(jsnmk_buf_t *b, const char *fmt, ...);

bool jsnmk_key_null(jsnmk_buf_t *b, const char *key);
bool jsnmk_key_bool(jsnmk_buf_t *b, const char *key, bool value);
bool jsnmk_key_i32(jsnmk_buf_t *b, const char *key, int32_t value);
bool jsnmk_key_u32(jsnmk_buf_t *b, const char *key, uint32_t value);
bool jsnmk_key_f32(jsnmk_buf_t *b, const char *key, float value);
bool jsnmk_key_cstr(jsnmk_buf_t *b, const char *key, const char *value);
bool jsnmk_key_fstr(jsnmk_buf_t *b, const char *key, const char *fmt, ...);

#endif // JSNMK_H
