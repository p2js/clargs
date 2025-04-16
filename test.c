#include <stdio.h>

#include "CLargs.h"

const CL_Schema schema = CL_DEFINESCHEMA(
    OPTION_HELP(),
    OPTION_BOOLEAN("verbose", 'v', "Enable verbose output"),
    OPTION_BOOLEAN("werbose", 'w', "Enable werbose output"),
    OPTION_INT("diceValue", 0, "Supply a dice value", -1, 6),
    OPTION_DOUBLE("randomValue", 0, "supply a random value", 1.23, 4.56),
    OPTION_OPTIONAL("smartMode", 0, "Enable smart mode, optionally with an additional mode"),
    OPTION_STRING("mode", 0, "Choose a mode"),
    OPTION_ONEOF("rouletteColor", 'r', "Select a roulette color", "red", "black"));

int main(int argc, char* argv[]) {
    CL_Args args = CL_parse(argc, argv, schema);
    printf("Verbose Mode: %s\n", CL_flag("verbose", args).boolean ? "on" : "off");
    printf("Werbose Mode: %s\n", CL_flag("werbose", args).boolean ? "on" : "off");
    printf("Dice Value: %d\n", CL_flag("diceValue", args).integer);
    printf("Random Value: %f\n", CL_flag("randomValue", args).number);
    printf("Smart Mode: %s\n", CL_flag("smartMode", args).string);
    printf("Mode: %s\n", CL_flag("mode", args).string);
    printf("Roulette Color: %s\n", CL_flag("rouletteColor", args).string);

    CL_freeArgs(args);
}