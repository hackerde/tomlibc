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

The code used to perform the benchmarks are included in the `benchmarks` directory.
However, the libraries are not included and there are no compilation instructions.
The reader may try and reproduce these results after following the steps to install
the libraries appropriately with the provided code files.
