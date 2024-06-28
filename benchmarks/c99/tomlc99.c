#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "../../../tomlc99/toml.h"

static void error(const char* msg, const char* msg1)
{
    fprintf(stderr, "ERROR: %s%s\n", msg, msg1?msg1:"");
    exit(1);
}

int main(int argc, char* argv[], char** envp)
{
    char* file = NULL;
    if( argc > 1 )
        file = argv[1];
    FILE* fp;
    char errbuf[200];

    // 1. Read and parse toml file
    fp = fopen(file, "r");
    if (!fp) {
        error("cannot open - ", strerror(errno));
    }

    toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!conf) {
        error("cannot parse - ", errbuf);
    }

    toml_free(conf);
    return 0;
}
