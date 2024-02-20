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
$ mkdir -p obj
$ make
$ ./main
```

## Tests

The test suite also includes the [official compliance tests](https://github.com/toml-lang/toml-test).

```bash
$ mkdir -p obj
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
[REPORT]: Passed 52 tests | Failed 3 tests | Percent: 94.54%
```

**toml-test**

```
Running toml-test tests
==================
...
[REPORT]: Passed 423 tests | Failed 115 tests | Percent: 78.62%
```

## Limitations

Aside from the usual bug fixes, the following are *major* features that will be added soon:

- [ ] support for unicode escaping
- [ ] millisecond precision
- [ ] supporting `\r\n` as newline

There are also a few other *minor* issues that will eventually be addressed as well:

- [ ] print line and column correctly on error
- [ ] allow empty quoted keys

# NOTE (AJB) General Comments:
- The distinction btwn src/ and lib/ is unclear and unstandard. That said, if you ask what the standard is, I'm gonna just shrug because
  projects are all over the place. C/C++ seem to have the least consistant ecosystems and people fall into different "camps".
