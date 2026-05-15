# RauMa Standard Library API

This document describes the v0.2.0 bridge-compatible standard library surface.
The API is intentionally concrete while the compiler's generic and enum lowering
continue to mature.

## `std/core/option.rm`

- `option_is_some_int(opt bool, has_value bool) bool`
- `option_is_none_int(opt bool, has_value bool) bool`
- `option_unwrap_or_int(opt bool, has_value bool, value int, fallback int) int`
- `option_unwrap_or_str(opt bool, has_value bool, value str, fallback str) str`
- `option_unwrap_int(opt bool, has_value bool, value int) int`

## `std/core/result.rm`

- `result_is_ok(ok bool) bool`
- `result_is_err(ok bool) bool`
- `result_unwrap_or_int(ok bool, value int, fallback int) int`
- `result_unwrap_or_str(ok bool, value str, fallback str) str`
- `result_error_code(ok bool, code int) int`

## `std/str/str.rm`

- `str_len_fn(s str) int`
- `str_eq_fn(a str, b str) bool`
- `str_byte_at(s str, i int) int`
- `str_concat_fn(a str, b str) str`
- `str_substr(s str, start int, end int) str`
- `str_is_empty(s str) bool`
- `str_first_char(s str) int`
- `str_last_char(s str) int`

## `std/io`

- `print_str(s str)`
- `println_str(s str)`
- `print_int(i int)`
- `println_int(i int)`
- `print_bool(b bool)`
- `println_bool(b bool)`
- `println_newline()`
- `read_all(path str) str`
- `write_all(path str, content str) bool`
- `read_is_empty(path str) bool`

## `std/conv/conv.rm`

- `int_to_str(n int) str`
- `str_to_int(s str) int`
- `bool_to_str(value bool) str`
- `str_to_bool(s str) bool`

`int_to_str` and `str_to_int` currently cover single decimal digits because the
bridge subset does not yet expose a general integer formatting/parsing runtime.

## `std/collections/vec.rm`

- `vec_len_int(len int) int`
- `vec_is_empty_int(len int) bool`
- `vec_can_get_int(len int, index int) bool`
- `vec_next_cap_int(cap int) int`

This is a bridge-compatible vector helper surface. Real `Vec<T>` remains gated
on generic struct lowering.

## `std/math`

- `abs_int(x int) int`
- `min_int(a int, b int) int`
- `max_int(a int, b int) int`
- `clamp_int(value int, low int, high int) int`

Floating-point math remains a later backend/runtime task.
