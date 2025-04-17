# CLargs

A command-line interface argument parsing library for C/C++.

It features an optional schema definition, multiple choices for option parsing, errors and a programmatically generated help menu.

## Usage

CLargs is designed to be flexible: You can supply a schema for your program's options, or (for more simple cases) call the argument parser without supplying one.

### Examples

For example usage of all the features described in the below documentation, see the two source files in `examples` - `noschema.c` for an example of getting arguments without a schema, and `arithmetic.c` for a more full-fledged example with a schema and overridden help message.

### Defining a schema

You can define an option schema describing your options with the `CL_DEFINESCHEMA` macro, which will format them to be received by the parser.

There are several different types of options:
- `OPTION_BOOLEAN(name, abbr, description)` describes a boolean flag, which is either included (true) or not (false).
- `OPTION_INT(name, abbr, description, min, max, default)` describes a flag that takes an integer value, optionally from `min` to `max` (setting both as 0 will accept any signed 32-bit integer). Will be set to `default` if the flag is not passed.
- `OPTION_DOUBLE(name, abbr, description, min, max, default)` describes a flag that takes a double precision floating-point value, optionally from `min` to `max` (setting both as 0 will accept all options in the range of doubles). Will be set to `default` if the flag is not passed.
- `OPTION_STRING(name, abbr, description)` describes a flag that takes a string value.
- `OPTION_OPTIONAL(name, abbr, description)` describes a flag that may optionally be followed by a string value.
- `OPTION_ONEOF(name, abbr, description, ...)` describes a flag that takes a string value that may be one of the provided possibilities (max 15 by default, but this can be changed in the source code). Will be set to the first provided possibility if the flag is not passed.
- `OPTION_HELP()` describes the `--help` flag, which can be optionally added to your schema to display all the usage information and description.

The `abbr` field defines an optional single-character flag name to be used as short form (eg `OPTION_BOOLEAN("verbose", 'v', ...)` can be toggled by including either `--verbose` or `-v`). Use the value 0 if you don't want the flag to have an abbreviation.


### Parsing/retrieving options and values

The options and values given when the program is started can be parsed and collected into a `CL_Args` object using `CL_parse(argc, argv, schema)`, which must be freed at the end of the program using `CL_free(args)`. 

The integer options accept different formats for integer values, namely starting with `0b`, `0o` or`0x` for binary, octal and hexadecimal values respectively.

Boolean short-form options can be grouped, eg `-l` and `-a` can become `-la`.

*Note: Without a schema, all options will be collected and their following value (if present) will be treated as a string. Short-form options will be ignored and treated as values.*

Specific options can be grabbed from a `CL_Args` object using `CL_flag(flagname, args)` - This will be a union of all the possible types of value (boolean/string/integer/number), so it must be accessed according to the type defined in the schema.

### Custom parse error/help behaviour

When `OPTION_HELP()` is included in the parsing schema, invoking the program with `--help` will display a rudimentary menu of all the flag options, then exit prematurely. 
Additionally, parse errors will inform the end-user and also exit.

If you wish to override either of those behaviours, you may use the `CL_setParseErrorCallback` and `CL_setHelpCallback` functions. Note that you are expected to exit the program in the parse error function, but the help function may simply return a boolean value of `true` to exit.