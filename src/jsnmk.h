#ifndef JSNMK_H
#define JSNMK_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @file jsnmk.h
 * @brief Minimal JSON writer for caller-provided fixed-size buffers.
 */

/**
 * @defgroup jsnmk jsnmk
 * @brief Zero-allocation JSON writer for constrained targets.
 *
 * The library writes JSON text directly into a caller-owned buffer. It
 * performs no allocation and keeps no state outside jsnmk_buf_t, which
 * makes it suitable for microcontrollers and other constrained systems.
 *
 * @par Buffer contract
 * - The caller owns the storage and initializes @c p and @c size.
 * - @c p is the next write position; @c size is the remaining capacity.
 * - The written length is the distance from the buffer start to @c b->p.
 * - No hidden state is kept beyond the fields of jsnmk_buf_t.
 *
 * @par Output protocol
 * - Every successful value call and every successful end call leaves a
 *   trailing comma.
 * - The enclosing end call replaces the comma left by its last member
 *   (or appends the bracket for an empty container) with @c '}' or
 *   @c ']', then appends its own comma.
 * - Call jsnmk_finalyze() once at the end: it removes the final comma and
 *   writes a NUL byte (0x00), so the result can be used as a C string.
 * - An end call must match a begin call. Keyed begins open containers too
 *   and are closed with the common jsnmk_obj_end()/jsnmk_arr_end().
 * - The final comma must fit in the buffer before it is removed, so a
 *   document ending with an end call needs one spare byte of peak
 *   capacity.
 *
 * @par Failure handling
 * - If the output does not fit, the call returns @c false and sets
 *   @c b->size to zero; partial bytes may remain and further writes
 *   fail.
 * - A NULL key is rejected with @c false and no bytes are written.
 * - jsnmk_finalyze() returns @c false if the last byte is not a comma and
 *   leaves the buffer unchanged.
 * - A NULL buffer, NULL string value, or NULL format string is undefined
 *   behavior.
 *
 * @par Escaping and validation
 * - Keys and string values are written verbatim. No JSON escaping is
 *   performed, and UTF-8 is not validated.
 * - @c jsnmk_f32() uses @c "%g"; NaN and infinities are not valid JSON.
 * - The library does not check that begin/end calls are balanced; pairing
 *   and a single final jsnmk_finalyze() call are the caller's duty.
 *
 * @warning Calling an end function without a matching begin cannot
 *          produce valid JSON; when nothing has been written yet it also
 *          reads before the buffer and is undefined behavior. Calling
 *          jsnmk_finalyze() before any byte has been written always reads
 *          before the buffer and is undefined behavior.
 *
 * @{
 */

/**
 * @brief State of a caller-provided output buffer.
 *
 * Initialize the structure with the beginning of the storage and its
 * capacity:
 * @code
 * char storage[256];
 * jsnmk_buf_t b = { .p = storage, .size = sizeof(storage) };
 * @endcode
 *
 * The library advances @c p and decreases @c size as it writes. The
 * buffer is not NUL-terminated until a successful jsnmk_finalyze() call.
 * The terminator is written into the byte freed by removing the last
 * comma and is not included in the written length.
 *
 * @warning @c p and @c size must describe a valid writable range; the
 *          library does not check the range itself.
 */
typedef struct jsnmk_buf {
    char *p;       /**< Next byte to write. */
    uint32_t size; /**< Number of writable bytes remaining at @c p. */
} jsnmk_buf_t;

/** @name Container starts
 *  @{ */

/**
 * @brief Begin an unkeyed JSON object.
 *
 * Appends @c '{'. The object must be closed with jsnmk_obj_end().
 *
 * @param[in,out] b Buffer state.
 * @retval true  The opening brace was written.
 * @retval false The buffer is too small.
 * @see jsnmk_obj_end, jsnmk_key_obj_begin
 */
bool jsnmk_obj_begin(jsnmk_buf_t *b);

/**
 * @brief Begin an unkeyed JSON array.
 *
 * Appends @c '['. The array must be closed with jsnmk_arr_end().
 *
 * @param[in,out] b Buffer state.
 * @retval true  The opening bracket was written.
 * @retval false The buffer is too small.
 * @see jsnmk_arr_end, jsnmk_key_arr_begin
 */
bool jsnmk_arr_begin(jsnmk_buf_t *b);

/** @} */

/** @name Unkeyed values
 *  @{ */

/**
 * @brief Append a JSON @c null value.
 *
 * On success the value is followed by a comma.
 *
 * @param[in,out] b Buffer state.
 * @retval true  The value was written.
 * @retval false The buffer is too small (@c b->size becomes zero).
 */
bool jsnmk_null(jsnmk_buf_t *b);

/**
 * @brief Append a JSON boolean value.
 *
 * Writes @c true or @c false, followed by a comma on success.
 *
 * @param[in,out] b Buffer state.
 * @param[in]     value Boolean value to serialize.
 * @retval true  The value was written.
 * @retval false The buffer is too small (@c b->size becomes zero).
 */
bool jsnmk_bool(jsnmk_buf_t *b, bool value);

/**
 * @brief Append a signed 32-bit integer in decimal.
 *
 * Writes the value followed by a comma on success.
 *
 * @param[in,out] b Buffer state.
 * @param[in]     value Integer value to serialize.
 * @retval true  The value was written.
 * @retval false The buffer is too small (@c b->size becomes zero).
 */
bool jsnmk_i32(jsnmk_buf_t *b, int32_t value);

/**
 * @brief Append an unsigned 32-bit integer in decimal.
 *
 * Writes the value followed by a comma on success.
 *
 * @param[in,out] b Buffer state.
 * @param[in]     value Integer value to serialize.
 * @retval true  The value was written.
 * @retval false The buffer is too small (@c b->size becomes zero).
 */
bool jsnmk_u32(jsnmk_buf_t *b, uint32_t value);

/**
 * @brief Append a 32-bit floating-point number.
 *
 * The value is formatted with @c "%g" (six significant digits by
 * default), so @c 1.0f is written as @c 1. On success it is followed by
 * a comma.
 *
 * @param[in,out] b Buffer state.
 * @param[in]     value Floating-point value to serialize.
 * @retval true  The value was written.
 * @retval false The buffer is too small (@c b->size becomes zero).
 *
 * @warning NaN and infinities are written as @c nan and @c inf, which are
 *          not valid JSON. The decimal separator follows the current C
 *          locale; JSON requires a period.
 */
bool jsnmk_f32(jsnmk_buf_t *b, float value);

/**
 * @brief Append a JSON string from a NUL-terminated C string.
 *
 * The string is written between quotes and followed by a comma on
 * success.
 *
 * @param[in,out] b     Buffer state.
 * @param[in]     value NUL-terminated string; must not be NULL.
 * @retval true  The string was written.
 * @retval false The buffer is too small (@c b->size becomes zero).
 *
 * @warning The string is written verbatim. No escaping is performed:
 *          embedded quotes, backslashes, control characters, or invalid
 *          UTF-8 produce invalid JSON.
 */
bool jsnmk_cstr(jsnmk_buf_t *b, const char *value);

/**
 * @brief Append a JSON string produced by a printf-style format.
 *
 * The formatted text is written between quotes and followed by a comma
 * on success.
 *
 * @param[in,out] b   Buffer state.
 * @param[in]     fmt printf-compatible format string; must not be NULL.
 *                    Remaining arguments are interpreted by @p fmt.
 * @retval true  The formatted string was written.
 * @retval false The buffer is too small (@c b->size becomes zero).
 *
 * @warning The formatted text is not escaped. The caller must ensure that
 *          it contains no characters that require JSON escaping.
 */
__attribute__ ((__format__ (__printf__, 2, 3)))
bool jsnmk_fstr(jsnmk_buf_t *b, const char *fmt, ...);

/** @} */

/** @name Keyed members
 *
 * Each function writes @c "key": followed by its value. Keys and string
 * values are written verbatim; no JSON escaping is performed.
 *
 * @warning A NULL key is rejected with @c false and no bytes are written.
 *          Keys containing quotes, backslashes, control characters, or
 *          invalid UTF-8 produce invalid JSON.
 *  @{ */

/**
 * @brief Begin an object stored under a key.
 *
 * Writes @c "key": followed by @c '{'. The object must be closed with
 * jsnmk_obj_end().
 *
 * @param[in,out] b   Buffer state.
 * @param[in]     key Object key; must not be NULL.
 * @retval true  The key and opening brace were written.
 * @retval false @p key is NULL, or the buffer is too small.
 * @see jsnmk_obj_end
 */
bool jsnmk_key_obj_begin(jsnmk_buf_t *b, const char *key);

/**
 * @brief Begin an array stored under a key.
 *
 * Writes @c "key": followed by @c '['. The array must be closed with
 * jsnmk_arr_end().
 *
 * @param[in,out] b   Buffer state.
 * @param[in]     key Array key; must not be NULL.
 * @retval true  The key and opening bracket were written.
 * @retval false @p key is NULL, or the buffer is too small.
 * @see jsnmk_arr_end
 */
bool jsnmk_key_arr_begin(jsnmk_buf_t *b, const char *key);

/**
 * @brief Append a member with a JSON @c null value.
 *
 * Writes @c "key":null, on success.
 *
 * @param[in,out] b   Buffer state.
 * @param[in]     key Member key; must not be NULL.
 * @retval true  The member was written.
 * @retval false @p key is NULL, or the buffer is too small.
 */
bool jsnmk_key_null(jsnmk_buf_t *b, const char *key);

/**
 * @brief Append a member with a JSON boolean value.
 *
 * Writes @c "key":true, or @c "key":false, on success.
 *
 * @param[in,out] b     Buffer state.
 * @param[in]     key   Member key; must not be NULL.
 * @param[in]     value Boolean value to serialize.
 * @retval true  The member was written.
 * @retval false @p key is NULL, or the buffer is too small.
 */
bool jsnmk_key_bool(jsnmk_buf_t *b, const char *key, bool value);

/**
 * @brief Append a member with a signed 32-bit integer.
 *
 * Writes @c "key": followed by the decimal value and a comma.
 *
 * @param[in,out] b     Buffer state.
 * @param[in]     key   Member key; must not be NULL.
 * @param[in]     value Integer value to serialize.
 * @retval true  The member was written.
 * @retval false @p key is NULL, or the buffer is too small.
 */
bool jsnmk_key_i32(jsnmk_buf_t *b, const char *key, int32_t value);

/**
 * @brief Append a member with an unsigned 32-bit integer.
 *
 * Writes @c "key": followed by the decimal value and a comma.
 *
 * @param[in,out] b     Buffer state.
 * @param[in]     key   Member key; must not be NULL.
 * @param[in]     value Integer value to serialize.
 * @retval true  The member was written.
 * @retval false @p key is NULL, or the buffer is too small.
 */
bool jsnmk_key_u32(jsnmk_buf_t *b, const char *key, uint32_t value);

/**
 * @brief Append a member with a 32-bit floating-point value.
 *
 * Writes @c "key": followed by the value in @c "%g" format and a comma.
 *
 * @param[in,out] b     Buffer state.
 * @param[in]     key   Member key; must not be NULL.
 * @param[in]     value Floating-point value to serialize.
 * @retval true  The member was written.
 * @retval false @p key is NULL, or the buffer is too small.
 * @note The formatting caveats of jsnmk_f32() apply.
 */
bool jsnmk_key_f32(jsnmk_buf_t *b, const char *key, float value);

/**
 * @brief Append a member with a string value.
 *
 * Writes @c "key":"value", on success. Neither the key nor the value is
 * escaped.
 *
 * @param[in,out] b     Buffer state.
 * @param[in]     key   Member key; must not be NULL.
 * @param[in]     value NUL-terminated string; must not be NULL.
 * @retval true  The member was written.
 * @retval false @p key is NULL, or the buffer is too small.
 */
bool jsnmk_key_cstr(jsnmk_buf_t *b, const char *key, const char *value);

/**
 * @brief Append a member with a printf-formatted string value.
 *
 * Writes @c "key":"formatted", on success. Neither the key nor the
 * formatted text is escaped.
 *
 * @param[in,out] b   Buffer state.
 * @param[in]     key Member key; must not be NULL.
 * @param[in]     fmt printf-compatible format string; must not be NULL.
 *                    Remaining arguments are interpreted by @p fmt.
 * @retval true  The member was written.
 * @retval false @p key is NULL, or the buffer is too small.
 */
__attribute__ ((__format__ (__printf__, 3, 4)))
bool jsnmk_key_fstr(jsnmk_buf_t *b, const char *key, const char *fmt, ...);

/** @} */

/** @name Closing and finalization
 *  @{ */

/**
 * @brief Close the current JSON object.
 *
 * If the last byte is a comma, it is replaced with @c '}'; otherwise a
 * @c '}' is appended. A comma is then appended so that the object can be
 * followed by another value. The enclosing close call consumes that
 * comma, and jsnmk_finalyze() removes the document's final comma.
 *
 * @param[in,out] b Buffer state.
 * @retval true  The object was closed.
 * @retval false The closing bracket or the following comma did not fit
 *               (@c b->size becomes zero).
 *
 * @pre At least one byte has been written and an object is open. Use it
 *      after jsnmk_obj_begin() or jsnmk_key_obj_begin().
 *
 * @warning Closing an object without a matching begin cannot produce
 *          valid JSON; if nothing has been written yet, it also reads
 *          @c b->p[-1] and is undefined behavior.
 * @see jsnmk_obj_begin, jsnmk_key_obj_begin, jsnmk_finalyze
 */
bool jsnmk_obj_end(jsnmk_buf_t *b);

/**
 * @brief Close the current JSON array.
 *
 * If the last byte is a comma, it is replaced with @c ']'; otherwise a
 * @c ']' is appended. A comma is then appended so that the array can be
 * followed by another value. The enclosing close call consumes that
 * comma, and jsnmk_finalyze() removes the document's final comma.
 *
 * @param[in,out] b Buffer state.
 * @retval true  The array was closed.
 * @retval false The closing bracket or the following comma did not fit
 *               (@c b->size becomes zero).
 *
 * @pre At least one byte has been written and an array is open. Use it
 *      after jsnmk_arr_begin() or jsnmk_key_arr_begin().
 *
 * @warning Closing an array without a matching begin cannot produce
 *          valid JSON; if nothing has been written yet, it also reads
 *          @c b->p[-1] and is undefined behavior.
 * @see jsnmk_arr_begin, jsnmk_key_arr_begin, jsnmk_finalyze
 */
bool jsnmk_arr_end(jsnmk_buf_t *b);

/**
 * @brief Finish the document: remove the final comma and NUL-terminate.
 *
 * If the last written byte is a comma, it is replaced with a NUL byte
 * (0x00), @c b->p is moved back by one byte, and @c b->size is
 * increased by one.
 * After a successful call, @c strlen(start) equals the written length and
 * the buffer can be used with ordinary C string functions.
 *
 * @param[in,out] b Buffer state.
 * @retval true  The trailing comma was removed and the terminator written.
 * @retval false The last byte is not a comma; the buffer is unchanged.
 *
 * @pre At least one byte has been written.
 *
 * @note Does not check that containers are balanced; begin/end pairing is
 *       the caller's responsibility.
 * @note The terminator is written over the removed comma, so no extra
 *       capacity is needed beyond the final-comma byte.
 * @note Finalization is terminal; do not write more data afterwards.
 *
 * @warning Calling it on a fresh buffer reads @c b->p[-1] and is
 *          undefined behavior.
 * @see jsnmk_obj_end, jsnmk_arr_end
 */
bool jsnmk_finalyze(jsnmk_buf_t *b);

/** @} */

/** @} */

#endif // JSNMK_H
