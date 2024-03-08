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

## Tests

The test suite also includes the [official compliance tests](https://github.com/toml-lang/toml-test).

```bash
# specify -r to show regressions
$ ./tests/run_tests.sh [-r]         # this runs the tomlibc tests
$ ./tests/run_toml_tests.sh [-r]    # this runs the toml-test suite tests
```

### Current Status

> [!NOTE]
> This has only been tested on macOS Sonoma Version 14.2.1 with clang 12.0.5

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

The failing tests involve leading zeros, `+` or `-` signs, invalid unicode characters, etc.
