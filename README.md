> [!WARNING]  
> This project is still a work in progress.

# ctoml

TOML parser written in C.
Written to be compliant with [TOML v1.0.0](https://toml.io/en/v1.0.0).

## Usage

A toml file [`sample.toml`](sample.toml) is provided along with a sample C program [`main.c`](main.c).
The only header that you need to include is `src/toml.h`.
An example [Makefile](Makefile) is also provided.

## Tests

The test suite also includes the [official compliance tests](https://github.com/toml-lang/toml-test).

```bash
$ ./tests/run_tests.sh          # this runs the ctoml tests
$ ./tests/run_toml_tests.sh     # this runs the toml-test suite tests
```

### Current Status

**ctoml**

```
Running ctoml tests
==================
...
[REPORT]: Passed 52 tests | Failed 3 tests | Percent: 94.54%
```

**toml-test**

```
Running toml-test tests
==================
...
[REPORT]: Passed 390 tests | Failed 148 tests | Percent: 72.49%
```

## Limitations

Aside from the usual bug fixes, the following are *major* features that will be added soon:

- [ ] support for unicode escaping
- [ ] millisecond precision
- [ ] supporting `\r\n` as newline

There are also a few other *minor* issues that will eventually be addressed as well:

- [ ] do not allow trailing commas in inline tables
- [ ] print line and column correctly on error
- [ ] allow empty quoted keys
