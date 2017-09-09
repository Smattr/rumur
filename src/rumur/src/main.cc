#include <cassert>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include <unistd.h>

using namespace rumur;
using namespace std;

static istream *in;
static string *out;
static OutputOptions output_options = {
    .overflow_checks = true,
};

static void parse_args(int argc, char **argv) {

    for (;;) {
        static struct option options[] = {
            { "help", no_argument, 0, '?' },
            { "output", required_argument, 0, 'o' },
            { 0, 0, 0, 0 },
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "o:?", options, &option_index);

        if (c == -1)
            break;

        switch (c) {

            case 'o':
                if (out != nullptr)
                    delete out;
                out = new string(optarg);
                break;

            case '?':
                cerr << "usage: " << argv[0] << " --output DIR [FILE]\n";
                exit(EXIT_FAILURE);

            default:
                cerr << "unexpected error\n";
                exit(EXIT_FAILURE);

        }
    }

    if (optind == argc - 1) {
        ifstream *inf = new ifstream(argv[optind]);
        if (!inf->is_open()) {
            cerr << "failed to open " << argv[optind] << "\n";
            exit(EXIT_FAILURE);
        }
        in = inf;
    } else {
        in = &cin;
    }

    if (out == nullptr) {
        cerr << "output directory is required\n";
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {

    // Parse command line options
    parse_args(argc, argv);

    // Parse input model
    Model *m;
    try {
        m = parse(in);
    } catch (RumurError &e) {
        cerr << e.loc << ":" << e.what() << "\n";
        return EXIT_FAILURE;
    }

    assert(out != nullptr);
    assert(m != nullptr);
    if (output_includes(*out) != 0)
        return EXIT_FAILURE;
    if (output_checker(*out + "/checker.c", *m, output_options) != 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
