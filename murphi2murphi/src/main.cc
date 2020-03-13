#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "find-liveness.h"
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include "options.h"
#include "Printer.h"
#include <rumur/rumur.h>
#include <sstream>
#include <sys/stat.h>
#include <vector>

using namespace rumur;

static std::shared_ptr<std::istream> in;
static std::shared_ptr<std::istream> in_replay;
static std::shared_ptr<std::ostream> out;

// buffer the contents of stdin so we can read it twice
static void buffer_stdin(void) {

  // read in all of stdin
  std::ostringstream buf;
  buf << std::cin.rdbuf();
  buf.flush();

  // put this into two buffers we can read from
  in = std::make_shared<std::istringstream>(buf.str());
  in_replay = std::make_shared<std::istringstream>(buf.str());
}

static void parse_args(int argc, char **argv) {

  for (;;) {

    static struct option opts[] = {
      { "explicit-semicolons",    no_argument,       0, 128 },
      { "help",                   no_argument,       0, '?' },
      { "no-explicit-semicolons", no_argument,       0, 129 },
      { "no-remove-liveness",     no_argument,       0, 130 },
      { "output",                 required_argument, 0, 'o' },
      { "remove-liveness",        no_argument,       0, 131 },
      { 0, 0, 0, 0 },
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "h", opts, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {

      case 128: // --explicit-semicolons
        options.explicit_semicolons = true;
        break;

      case '?': // --help
        std::cerr << "TODO\n";
        exit(EXIT_SUCCESS);

      case 129: // --no-explicit-semicolons
        options.explicit_semicolons = false;
        break;

      case 130: // --no-remove-liveness
        options.remove_liveness = false;
        break;

      case 'o': { // --output
        auto o = std::make_shared<std::ofstream>(optarg);
        if (!o->is_open()) {
          std::cerr << "failed to open " << optarg << "\n";
          exit(EXIT_FAILURE);
        }
        out = o;
        break;
      }

      case 131: // --remove-liveness
        options.remove_liveness = true;
        break;

      default:
        std::cerr << "unexpected error\n";
        exit(EXIT_FAILURE);
    }
  }

  if (optind == argc - 1) {
    struct stat buf;
    if (stat(argv[optind], &buf) < 0) {
      std::cerr << "failed to open " << argv[optind] << ": " << strerror(errno) << "\n";
      exit(EXIT_FAILURE);
    }

    if (S_ISDIR(buf.st_mode)) {
      std::cerr << "failed to open " << argv[optind] << ": this is a directory\n";
      exit(EXIT_FAILURE);
    }

    auto i = std::make_shared<std::ifstream>(argv[optind]);
    if (!i->is_open()) {
      std::cerr << "failed to open " << argv[optind] << "\n";
      exit(EXIT_FAILURE);
    }
    in = i;

    // open the input again that we need for replay during XML output
    auto i2 = std::make_shared<std::ifstream>(argv[optind]);
    if (!i2->is_open()) {
      std::cerr << "failed to open " << argv[optind] << "\n";
      exit(EXIT_FAILURE);
    }
    in_replay = i2;
  } else {
    // we are going to read data from stdin
    buffer_stdin();
  }
}

int main(int argc, char **argv) {

  // parse command line options
  parse_args(argc, argv);

  assert(in != nullptr);

  // Parse input model
  Ptr<Model> m;
  try {
    m = parse(*in);
    resolve_symbols(*m);
    validate(*m);
  } catch (Error &e) {
    std::cerr << e.loc << ":" << e.what() << "\n";
    return EXIT_FAILURE;
  }

  assert(m != nullptr);

  // optionally remove liveness properties
  std::vector<location> liveness_ranges;
  if (options.remove_liveness) {
    liveness_ranges = find_liveness(*m);
  }

  {
    Printer p(*in_replay, out == nullptr ? std::cout : *out, liveness_ranges);
    p.dispatch(*m);
  }

  return EXIT_SUCCESS;
}
