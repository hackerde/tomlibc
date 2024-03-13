> [!WARNING]  
> This project is still a work in progress.

# tomlibc

TOML parser written in C.
Written to be compliant with [TOML v1.0.0](https://toml.io/en/v1.0.0).

## Usage

A toml file [`sample.toml`](sample.toml) is provided along with a sample C program [`main.c`](main.c).
The only header that you need to include is `tomlib.h`.
An example [Makefile](Makefile) is also provided.

### Building

```bash
$ make
$ ./main
```

### Callbacks

Use the `toml_get_key` function for accessing keys.
Example:

```c
toml_key_t* k = toml_get_key(
                toml_get_key(
                toml_get_key( toml, "data" ),
                                    "constants" ),
                                    "max" );
```

The following are the functions for accessing values:

```
toml_get_string
toml_get_int
toml_get_float
toml_get_bool
toml_get_datetime
toml_get_array
```

All callback functions take the `key` as the argument and returns a pointer to the value.
If there was any error in retrieving the value, all of them return `NULL`.
For arrays, the length is stored in `value->len` and the data in `value->arr`.

Example of accessing a value in an array:

```c
toml_value_t* v = toml_get_array(
                  toml_get_key(
                  toml_get_key( toml, "data" ),
                                      "d3" ) );
// assert v->len>2
double* pi      = v->arr[ 2 ]->data;
```

TOML Datetime objects are stored in `struct tm` as defined in `<time.h>`.
Since that struct does not support millisecond precision, that can be found in `v->precision`.

## Tests

The test suite also includes the [official compliance tests](https://github.com/toml-lang/toml-test).

```bash
# specify -r to show regressions
$ ./tests/run_tests.sh         # this runs the tomlibc tests
$ ./tests/run_toml_tests.sh    # this runs the toml-test suite tests
```

### Current Status

> [!NOTE]
> This has only been tested on macOS Sonoma Version 14.2.1 with clang 15.0.0

**tomlibc**

```
Running tomlibc tests
==================
...
[TEST REPORT]: Passed 55 tests | Failed 0 tests | Percent: 100.00%
```

**toml-test**

```
Running toml-test tests
==================
...
[TEST REPORT]: Passed 511 tests | Failed 27 tests | Percent: 94.98%
```

The failing tests involve leading zeros, `+` or `-` signs with zeros, invalid or null unicode characters, etc.

## Benchmarks

The library was benchmarked against some existing TOML parsers.
The code is pretty simple - they all open the file, parse it, free the memory and exit.
The benchmarks were done using the following files (which were generated using python):

```bash
-rw-r--r--  1 foo  bar   5.2M Mar 12 20:29 big.toml
-rw-r--r--  1 foo  bar   321M Mar 12 20:13 gigantic.toml
$ wc -l big.toml
    202001 big.toml
$ wc -l gigantic.toml
    10120001 gigantic.toml
```

### tomlibc (c)

```bash
$ time ./tomlibc ../big.toml

real    0m0.383s
user    0m0.286s
sys     0m0.029s

$ time ./tomlibc ../gigantic.toml

real	0m15.947s
user	0m14.694s
sys     0m0.965s
```

### tomlc99 (c)

```bash
$ time ./tomlc99 ../big.toml

real	0m26.868s
user	0m26.727s
sys     0m0.037s

$ time ./tomlc99 ../gigantic.toml

aborted after 23m38.254s
```

### toml++ (cpp)

```bash
$ time ./tomlpp ../big.toml

real	0m1.290s
user	0m1.254s
sys     0m0.022s

$ time ./tomlpp ../gigantic.toml

real	1m3.974s
user	1m3.105s
sys     0m0.599s
```

### toml (rust)

```bash
$ time target/debug/main ../big.toml

real	0m3.130s
user	0m3.047s
sys     0m0.062s

$ time target/debug/main ../gigantic.toml

real	2m13.804s
user	2m10.359s
sys     0m2.566s
```

### toml (python)

```bash
$ time python3 parser.py ../big.toml

real	0m2.325s
user	0m2.214s
sys     0m0.053s

$ time python3 parser.py ../gigantic.toml

real	1m55.768s
user	1m53.099s
sys     0m1.913s
```
