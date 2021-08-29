#include <argp.h>

struct argp_option options[3] = { 
        { "verbose", 'v', 0, 0, "Produce verbose output.", 0 }, 
        { "output", 'o', "FILE", 0, "Output to FILE instead of standard output", 0 }, 
        { 0 }
    };

struct arguments {
    char* arg;
    int verbose;
    char* output_file;
};

error_t parse_opt (int key, char *arg, struct argp_state *state) { 
    struct arguments *arguments = state->input;
    switch (key) {
        case 'v':
            arguments->verbose++;
            break;
        case 'o':
            arguments->output_file = arg;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1)
                argp_usage(state);
            arguments->arg = arg;
        case ARGP_KEY_END:
            printf("%d\n", state->arg_num);
            // if (state->arg_num == 0)
            //     argp_usage (state);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp argp = {options, parse_opt, "ARG", "literally does nothing."};

int main(int argc, char *argv[]) {
    struct arguments a;
    a.arg = "";
    a.output_file = "out.file";
    a.verbose = 0;

    argp_parse(&argp, argc, argv, 0, 0, &a);

    printf("passed arg: %s\n", a.arg);
    printf("output file: %s\n", a.output_file);
    printf("verbose: %d\n", a.verbose);

    return 0;
}

