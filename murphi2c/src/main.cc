#include <cassert>
#include <cstddef>
#include "check.h"
#include "compares_complex_values.h"
#include <cstdlib>
#include <fstream>
#include "generate_c.h"
#include "generate_h.h"
#include <getopt.h>
#include "../../common/help.h"
#include <iostream>
#include <memory>
#include "name_rules.h"
#include "resources.h"
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static std::string in_filename = "<stdin>";
static std::shared_ptr<std::istream> in;
static std::shared_ptr<std::ostream> out;

// output C source? (as opposed to C header)
static bool source = true;

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option options[] = {
      { "header",  no_argument,       0, 128 },
      { "help",    no_argument,       0, 'h' },
      { "output",  required_argument, 0, 'o' },
      { "source",  no_argument,       0, 129 },
      { "version", no_argument,       0, 130 },
      { 0, 0, 0, 0 },
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "ho:", options, &option_index);

    if (c == -1)
      break;

    switch (c) {

      case '?':
        std::cerr << "run `" << argv[0] << " --help` to see available options\n";
        exit(EXIT_SUCCESS);

      case 128: // --header
        source = false;
        break;

      case 'h': // --help
        help(doc_murphi2c_1, doc_murphi2c_1_len);
        exit(EXIT_SUCCESS);

      case 'o': {
        auto o = std::make_shared<std::ofstream>(optarg);
        if (!o->is_open()) {
          std::cerr << "failed to open " << optarg << "\n";
          exit(EXIT_FAILURE);
        }
        out = o;
        break;
      }

      case 129: // --source
        source = true;
        break;

      case 130: // --version
        std::cout << "Rumur version " << rumur::get_version() << "\n";
        exit(EXIT_SUCCESS);

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

    in_filename = argv[optind];

    auto i = std::make_shared<std::ifstream>(in_filename);
    if (!i->is_open()) {
      std::cerr << "failed to open " << in_filename << "\n";
      exit(EXIT_FAILURE);
    }
    in = i;
  }
}

int main(int argc, char **argv) {

  // parse command line options
  parse_args(argc, argv);

  // parse input model
  rumur::Ptr<rumur::Model> m;
  try {
    m = rumur::parse(in == nullptr ? std::cin : *in);
    resolve_symbols(*m);
    validate(*m);
  } catch (rumur::Error &e) {
    std::cerr << e.loc << ":" << e.what() << "\n";
    return EXIT_FAILURE;
  }

  assert(m != nullptr);

  // validate that this model is OK to translate
  if (!check(*m))
    return EXIT_FAILURE;

  // name any rules that are unnamed, so they get valid C symbols
  name_rules(*m);

  // Determine if we have any == or != involving records or arrays, in which
  // case we will need to pack structs. See generate_c() for why.
  bool pack = compares_complex_values(*m);

  // output code
  if (source) {
    generate_c(*m, pack, out == nullptr ? std::cout : *out);
  } else {
    generate_h(*m, pack, out == nullptr ? std::cout : *out);
  }

  return EXIT_SUCCESS;
}
