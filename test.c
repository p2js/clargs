#include <stdio.h>

#include "CLargs.h"

const CL_Schema schema = CL_DEFINESCHEMA(
    OPTION_HELP(),
    OPTION_BOOLEAN("verbose", "Enable verbose output"),
    OPTION_INT("randomvalue", "Supply a random value", -1, 6),
    OPTION_OPTIONAL("smartMode", "Enable smart mode, optionally with an additional mode"),
    OPTION_STRING("mode", "Choose a mode"),
    OPTION_ONEOF("rouletteColor", "Select a roulette color", "red", "black"));

int main(int argc, char* argv[]) {
    CL_Args args = CL_parse(argc, argv, schema);

    printf("Verbose Mode: %s\n", CL_flag("verbose", args).boolean ? "on" : "off");
    printf("Random Value: %d\n", CL_flag("randomvalue", args).number);
    printf("Smart Mode: %s\n", CL_flag("smartMode", args).string);
    printf("Mode: %s\n", CL_flag("mode", args).string);
    printf("Roulette Color: %s\n", CL_flag("rouletteColor", args).string);

    CL_freeArgs(args);
}