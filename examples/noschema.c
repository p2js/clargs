/**
 * noschema.c
 *
 * Example program for CLargs (parsing without schema)
 * Try calling the program with different options/values!
 */
#include <stdio.h>

#include "../CLargs.h"

int main(int argc, char* argv[]) {
    // Parse args without a schema
    CL_Args args = CL_parse(argc, argv, NULL);

    char* mainValue = CL_flag("main", args).string;

    if (mainValue[0] != '\0') {  // If the value has been set (flag has been passed a value)
        printf("value of --main: %s\n", mainValue);
    } else {
        printf("--main value not set\n");
    };

    printf("===All Options:===\n");
    for (uint32_t i = 0; i < args.option_count; i++) {
        CL_FlagOption flag = args.options[i];
        bool includeColon = flag.value.string[0] != 0;
        printf("%s%s%s\n", flag.flag, includeColon ? ": " : " (no value)", flag.value.string);
    }
    printf("===All Values: ===\n");
    for (uint32_t i = 0; i < args.value_count; i++) {
        printf("%s\n", args.values[i]);
    }
}