# JSNMK

[![Build and run tests](https://github.com/bortech/jsnmk/actions/workflows/build_tests.yml/badge.svg)](https://github.com/bortech/jsnmk/actions/workflows/build_tests.yml)

A zero-allocation JSON writer for caller-provided fixed-size buffers,
intended for use in embedded, MCUs and other constrained targets.

Project is inspired by [rafagafe/json-maker](https://github.com/rafagafe/json-maker)
and intended to use with no-allocating snprintf realization like
[eyalroz/printf](https://github.com/eyalroz/printf) or similar.

Add `src/jsnmk.c` and `src/jsnmk.h` to your project, or link the CMake
interface target `jsnmk`.

## Example

```c
#include "jsnmk.h"
#include <stdio.h>

int main(void)
{
    char storage[128];
    jsnmk_buf_t b = { .p = storage, .size = sizeof(storage) };

    jsnmk_obj_begin(&b);
    jsnmk_key_cstr(&b, "name", "John");
    jsnmk_key_i32(&b, "age", 42);
    jsnmk_key_fstr(&b, "time", "%02u:%02u", 13, 37);
    jsnmk_key_arr_begin(&b, "tags");
    jsnmk_cstr(&b, "embedded");
    jsnmk_cstr(&b, "json");
    jsnmk_arr_end(&b);
    jsnmk_obj_end(&b);
    jsnmk_finalyze(&b);

    printf("%.*s\n", jsnmk_size(&b, storage), storage);
    return 0;
}
```

Output:

```json
{"name":"John","age":42,"time":"13:37","tags":["embedded","json"]}
```

## Notes

- Optionally check the return value of every call. If the output does not fit,
  `b.size` is set to 0 and further writes fail.
- Close every container, then call `jsnmk_finalyze()` once to remove the
  final comma and NUL-terminate the buffer.
- Keys and string values are written verbatim; no JSON escaping is
  performed. `f32` values use `%g`.
