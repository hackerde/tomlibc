> [!WARNING]  
> This project is still a work in progress.

# tomlibc

TOML parser written in C.
Written to be compliant with [TOML v1.0.0](https://toml.io/en/v1.0.0).

## Usage

A toml file [`sample.toml`](sample.toml) is provided along with a sample C program [`main.c`](main.c).
The only header that you need to include is `src/toml.h`.
An example [Makefile](Makefile) is also provided.

### Building

```bash
$ make
$ ./main
```

## Tests

The test suite also includes the [official compliance tests](https://github.com/toml-lang/toml-test).

```bash
$ ./tests/run_tests.sh          # this runs the tomlibc tests
$ ./tests/run_toml_tests.sh     # this runs the toml-test suite tests
```

### Current Status

> [!NOTE]
> This has only been tested on macOS Sonoma Version 14.2.1 with clang 12.0.5

**tomlibc**

```
Running tomlibc tests
==================
...
[TEST REPORT]: Passed 53 tests | Failed 2 tests | Percent: 96.36%
```

**toml-test**

```
Running toml-test tests
==================
...
[TEST REPORT]: Passed 462 tests | Failed 76 tests | Percent: 85.87%
```

## Limitations

Aside from the usual bug fixes, the following are *major* features that will be added soon:

- [ ] millisecond precision

There are also a few other *minor* issues that will eventually be addressed as well:

- [ ] print line and column correctly on error
- [ ] validate date and time
