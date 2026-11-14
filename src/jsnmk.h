#ifndef JSNMK_H
#define JSNMK_H

#include <stdint.h>
#include <stdbool.h>

typedef struct jsnmk_buf {
    char *p;
    uint32_t size;
} jsnmk_buf_t;

bool jsnmk_obj_begin(jsnmk_buf_t *b);
bool jsnmk_obj_end(jsnmk_buf_t *b);

bool jsnmk_key_obj_begin(jsnmk_buf_t *b, const char *key);
bool jsnmk_key_obj_end(jsnmk_buf_t *b);

bool jsnmk_arr_begin(jsnmk_buf_t *b);
bool jsnmk_arr_end(jsnmk_buf_t *b);

bool jsnmk_key_arr_begin(jsnmk_buf_t *b, const char *key);
bool jsnmk_key_arr_end(jsnmk_buf_t *b);

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
