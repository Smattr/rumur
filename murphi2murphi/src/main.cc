#include "../../common/help.h"
#include "DecomposeComplexComparisons.h"
#include "ExplicitSemicolons.h"
#include "Pipeline.h"
#include "Printer.h"
#include "RemoveLiveness.h"
#include "Stage.h"
#include "SwitchToIf.h"
#include "ToAscii.h"
#include "options.h"
#include "resources.h"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <sstream>
#include <sys/stat.h>

using namespace rumur;

static std::shared_ptr<std::istream> in;
static std::shared_ptr<std::istream> in_replay;
static std::shared_ptr<std::ostream> out;

// buffer the contents of stdin so we can read it twice
static void buffer_stdin() {

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
        // clang-format off
        { "decompose-complex-comparisons",    no_argument,       0, 128 },
        { "explicit-semicolons",              no_argument,       0, 129 },
        { "help",                             no_argument,       0, 'h' },
        { "no-decompose-complex-comparisons", no_argument,       0, 130 },
        { "no-explicit-semicolons",           no_argument,       0, 131 },
        { "no-remove-liveness",               no_argument,       0, 132 },
        { "no-switch-to-if",                  no_argument,       0, 133 },
        { "no-to-ascii",                      no_argument,       0, 134 },
        { "output",                           required_argument, 0, 'o' },
        { "remove-liveness",                  no_argument,       0, 135 },
        { "switch-to-if",                     no_argument,       0, 136 },
        { "to-ascii",                         no_argument,       0, 137 },
        { "version",                          no_argument,       0, 138 },
        { 0, 0, 0, 0 },
        // clang-format on
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "ho:", opts, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {

    case 128: // --decompose-complex-comparisons
      options.decompose_complex_comparisons = true;
      break;

    case 129: // --explicit-semicolons
      options.explicit_semicolons = true;
      break;

    case '?':
      std::cerr << "run `" << argv[0] << " --help` to see available options\n";
      exit(EXIT_SUCCESS);

    case 'h': // --help
      help(doc_murphi2murphi_1, doc_murphi2murphi_1_len);
      exit(EXIT_SUCCESS);

    case 130: // --no-decompose-complex-comparisons
      options.decompose_complex_comparisons = false;
      break;

    case 131: // --no-explicit-semicolons
      options.explicit_semicolons = false;
      break;

    case 132: // --no-remove-liveness
      options.remove_liveness = false;
      break;

    case 133: // --no-switch-to-if
      options.switch_to_if = false;
      break;

    case 134: // --no-to-ascii
      options.to_ascii = false;
      break;

    case 'o': { // --output
      auto o = std::make_shared<std::ofstream>(optarg);
      if (!o->is_open()) {
        std::cerr << "failed to open " << optarg << '\n';
        exit(EXIT_FAILURE);
      }
      out = o;
      break;
    }

    case 135: // --remove-liveness
      options.remove_liveness = true;
      break;

    case 136: // --switch-to-if
      options.switch_to_if = true;
      break;

    case 137: // --to-ascii
      options.to_ascii = true;
      break;

    case 138: // --version
      std::cout << "Murphi2Murphi version " << get_version() << '\n';
      exit(EXIT_SUCCESS);

    default:
      std::cerr << "unexpected error\n";
      exit(EXIT_FAILURE);
    }
  }

  if (optind == argc - 1) {
    struct stat buf;
    if (stat(argv[optind], &buf) < 0) {
      std::cerr << "failed to open " << argv[optind] << ": " << strerror(errno)
                << '\n';
      exit(EXIT_FAILURE);
    }

    if (S_ISDIR(buf.st_mode)) {
      std::cerr << "failed to open " << argv[optind]
                << ": this is a directory\n";
      exit(EXIT_FAILURE);
    }

    auto i = std::make_shared<std::ifstream>(argv[optind]);
    if (!i->is_open()) {
      std::cerr << "failed to open " << argv[optind] << '\n';
      exit(EXIT_FAILURE);
    }
    in = i;

    // open the input again that we need for replay during XML output
    auto i2 = std::make_shared<std::ifstream>(argv[optind]);
    if (!i2->is_open()) {
      std::cerr << "failed to open " << argv[optind] << '\n';
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

  // parse input model
  Ptr<Model> m;
  try {
    m = parse_model(*in);
  } catch (Error &e) {
    std::cerr << e.loc << ":" << e.what() << '\n';
    return EXIT_FAILURE;
  }

  assert(m != nullptr);

  // assign unique identifiers to AST nodes
  m->reindex();

  // resolve symbolic references and validate the model
  try {
    resolve_symbols(*m);
    validate(*m);
  } catch (Error &e) {
    std::cerr << e.loc << ":" << e.what() << '\n';
    return EXIT_FAILURE;
  }

  // create a pipeline that we will incrementally populate
  Pipeline pipe;

  // add output generator
  Printer p(*in_replay, out == nullptr ? std::cout : *out);
  pipe.add_stage(p);

  // are we adding semi-colons?
  if (options.explicit_semicolons)
    pipe.make_stage<ExplicitSemicolons>();

  // are we removing liveness?
  if (options.remove_liveness)
    pipe.make_stage<RemoveLiveness>();

  // are we transforming switches to ifs?
  if (options.switch_to_if)
    pipe.make_stage<SwitchToIf>();

  // are we removing unicode operators?
  if (options.to_ascii)
    pipe.make_stage<ToAscii>();

  // are we decomposing complex comparisons?
  if (options.decompose_complex_comparisons)
    pipe.make_stage<DecomposeComplexComparisons>();

  try {
    // now we can run the pipeline
    pipe.process(*m);

    // note that we are done
    pipe.finalise();

  } catch (Error &e) {
    std::cerr << e.loc << ":" << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
