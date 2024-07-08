#include "../../common/help.h"
#include "check.h"
#include "codegen.h"
#include "options.h"
#include "pick_numeric_type.h"
#include "resources.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <utility>
#include <vector>

// a pair of input streams
using dup_t =
    std::pair<std::shared_ptr<std::istream>, std::shared_ptr<std::istream>>;

static std::string in_filename = "<stdin>";
static dup_t in;
static std::string out_filename = "-";
static std::shared_ptr<std::ostream> out;

std::string module_name = "main";

std::string numeric_type;

static bool is_valid_numeric_type(const char *s) {
  assert(s != NULL);
  if (strcmp(s, "integer") == 0)
    return true;
  if (strncmp(s, "bv", strlen("bv")) != 0)
    return false;
  for (const char *p = s + strlen("bv"); *p != '\0'; ++p) {
    if (!isdigit(*p))
      return false;
  }
  return true;
}

verbosity_t verbosity = WARNINGS;

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option options[] = {
        // clang-format off
        { "help",         no_argument,       0, 'h' },
        { "module",       required_argument, 0, 'm' },
        { "numeric-type", required_argument, 0, 'n' },
        { "output",       required_argument, 0, 'o' },
        { "quiet",        no_argument,       0, 'q' },
        { "verbose",      no_argument,       0, 'v' },
        { "version",      no_argument,       0, 128 },
        { 0, 0, 0, 0 },
        // clange-format on
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "hm:n:o:qv", options, &option_index);

    if (c == -1)
      break;

    switch (c) {

    case '?':
      std::cerr << "run `" << argv[0] << " --help` to see available options\n";
      exit(EXIT_SUCCESS);

    case 'h': // --help
      help(doc_murphi2uclid_1, doc_murphi2uclid_1_len);
      exit(EXIT_SUCCESS);

    case 'm':
      module_name = optarg;
      break;

    case 'n': // --numeric-type
      if (!is_valid_numeric_type(optarg)) {
        std::cerr << "invalid argument to --numeric-type " << optarg << '\n';
        exit(EXIT_FAILURE);
      }
      numeric_type = optarg;
      break;

    case 'o':
      out_filename = optarg;
      break;

    case 'q': // --quiet
      verbosity = QUIET;
      break;

    case 'v': // --verbose
      verbosity = VERBOSE;
      break;

    case 128: // --version
      std::cout << "Murphi2Uclid version " << rumur::get_version() << '\n';
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

    in_filename = argv[optind];

    auto i = std::make_shared<std::ifstream>(in_filename);
    auto j = std::make_shared<std::ifstream>(in_filename);
    if (!i->is_open() || !j->is_open()) {
      std::cerr << "failed to open " << in_filename << '\n';
      exit(EXIT_FAILURE);
    }
    in = dup_t(i, j);
  }
}

static dup_t make_stdin_dup() {

  // read stdin into memory
  auto buffer = std::make_shared<std::stringstream>();
  *buffer << std::cin.rdbuf();

  // duplicate the buffer
  auto copy = std::make_shared<std::istringstream>(buffer->str());

  return dup_t(buffer, copy);
}

static std::ostream &output() {
  return out == nullptr ? std::cout : *out;
}

int main(int argc, char **argv) {

  // parse command line options
  parse_args(argc, argv);

  // if we are reading from stdin, duplicate it so that we can parse it both as
  // Murphi and for comments
  if (in.first == nullptr)
    in = make_stdin_dup();

  // parse input
  rumur::Ptr<rumur::Node> parsed;
  try {
    parsed = rumur::parse(*in.first);
  } catch (rumur::Error &e) {
    std::cerr << e.loc << ":" << e.what() << '\n';
    return EXIT_FAILURE;
  }

  assert(parsed != nullptr);

  // if we have a model, run full validation
  auto model = dynamic_cast<rumur::Model *>(parsed.get());
  if (model != nullptr) {

  // update unique identifiers within the model
  model->reindex();

  // check the model is valid
  try {
    resolve_symbols(*model);
    validate(*model);
  } catch (rumur::Error &e) {
    std::cerr << e.loc << ":" << e.what() << '\n';
    return EXIT_FAILURE;
  }
  }

  // name any rules that are unnamed, so they get valid Uclid5 symbols
  rumur::sanitise_rule_names(*parsed);

  // check this can be translated to Uclid5
  try {
    check(*parsed);
  } catch (rumur::Error &e) {
    std::cerr << e.loc << ":" << e.what() << '\n';
    return EXIT_FAILURE;
  }

  // if the user did not select a numeric type, select one for them
  if (numeric_type == "")
    numeric_type = pick_numeric_type(*parsed);

  // parse comments from the source code
  std::vector<rumur::Comment> comments = rumur::parse_comments(*in.second);

  // only *now* open the output file, to avoid creating an empty file if any of
  // the preceding steps fail
  if (out_filename != "-") {
    auto o = std::make_shared<std::ofstream>(out_filename);
    if (!o->is_open()) {
      std::cerr << "failed to open " << out_filename << '\n';
      exit(EXIT_FAILURE);
    }
    out = o;
  }

  // generate Uclid5 source code
  codegen(*parsed, comments, output());

  return EXIT_SUCCESS;
}
