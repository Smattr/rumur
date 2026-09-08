#include "../../common/help.h"
#include "check.h"
#include "compares_complex_values.h"
#include "generate_c.h"
#include "generate_h.h"
#include "options.h"
#include "resources.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include <vector>

// a pair of input streams
using dup_t =
    std::pair<std::shared_ptr<std::istream>, std::shared_ptr<std::istream>>;

static std::string in_filename = "<stdin>";
static dup_t in;
static std::shared_ptr<std::ostream> out;

// output C source? (as opposed to C header)
static bool source = true;

/// use colour in error messages?
static enum { AUTO, ON, OFF } color = AUTO;

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option options[] = {
        // clang-format off
        { "color",      required_argument, 0, 132 },
        { "colour",     required_argument, 0, 132 },
        { "header",     no_argument,       0, 128 },
        { "help",       no_argument,       0, 'h' },
        { "output",     required_argument, 0, 'o' },
        { "source",     no_argument,       0, 129 },
        { "value-type", required_argument, 0, 130 },
        { "version",    no_argument,       0, 131 },
        { 0, 0, 0, 0 },
        // clang-format on
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
        std::cerr << "failed to open " << optarg << '\n';
        exit(EXIT_FAILURE);
      }
      out = o;
      break;
    }

    case 129: // --source
      source = true;
      break;

    case 130: // --value-type
      // note that we just assume the type the user gave us exists
      value_type = optarg;
      break;

    case 131: // --version
      std::cout << "Murphi2C version " << rumur_get_version() << '\n';
      exit(EXIT_SUCCESS);

    case 132: // --color, --colour
      if (strcmp(optarg, "auto") == 0) {
        color = AUTO;
      } else if (strcmp(optarg, "on") == 0) {
        color = ON;
      } else if (strcmp(optarg, "off") == 0) {
        color = OFF;
      } else {
        std::cerr << "invalid --colour argument \"" << optarg << "\"\n"
                  << "valid arguments are \"auto\", \"off\", and \"on\"\n";
        exit(EXIT_FAILURE);
      }
      break;

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

static bool use_colors() {
  if (color == AUTO)
    color = isatty(STDERR_FILENO) ? ON : OFF;
  return color == ON;
}

static const char *bold() {
  if (use_colors())
    return "\033[1m";
  return "";
}

static const char *green() {
  if (use_colors())
    return "\033[32m";
  return "";
}

static const char *red() {
  if (use_colors())
    return "\033[31m";
  return "";
}

static const char *reset() {
  if (use_colors())
    return "\033[0m";
  return "";
}

static const char *white() {
  if (use_colors())
    return "\033[37m";
  return "";
}

static void print_location(std::istream &src, const rumur::location &location) {

  // the type of position.line and position.column changes across Bison
  // releases, so avoid some -Wsign-compare warnings by casting them in advance
  auto loc_line = static_cast<unsigned long>(location.begin.line);
  auto loc_col = static_cast<unsigned long>(location.begin.column);

  std::string line;
  unsigned long lineno = 0;
  while (lineno < loc_line) {
    if (!std::getline(src, line))
      return;
    lineno++;
  }

  // print the line, and construct an underline indicating the column location
  std::ostringstream buf;
  unsigned long col = 1;
  for (const char &c : line) {
    if (col == loc_col) {
      buf << green() << bold() << '^' << reset();
    } else if (col < loc_col) {
      if (c == '\t') {
        buf << '\t';
      } else {
        buf << ' ';
      }
    }
    std::cerr << c;
    col++;
  }
  std::cerr << '\n';

  std::cerr << buf.str() << '\n';
}

int main(int argc, char **argv) {

  // parse command line options
  parse_args(argc, argv);

  // if we are reading from stdin, duplicate it so that we can parse it both as
  // Murphi and for comments
  if (in.first == nullptr)
    in = make_stdin_dup();

  // parse input model
  rumur::Ptr<rumur::Model> m;
  try {
    m = rumur::parse_model(*in.first);
  } catch (rumur::Error &e) {
    std::cerr << white() << bold() << in_filename << ':' << e.loc << ':'
              << reset() << ' ' << red() << bold() << "error:" << reset() << ' '
              << white() << bold() << e.what() << reset() << '\n';
    print_location(*in.second, e.loc);
    return EXIT_FAILURE;
  }

  assert(m != nullptr);

  // update unique identifiers within the model
  m->reindex();

  // check the model is valid
  try {
    resolve_symbols(*m);
    validate(*m);
  } catch (rumur::Error &e) {
    std::cerr << white() << bold() << in_filename << ':' << e.loc << ':'
              << reset() << ' ' << red() << bold() << "error:" << reset() << ' '
              << white() << bold() << e.what() << reset() << '\n';
    print_location(*in.second, e.loc);
    return EXIT_FAILURE;
  }

  // validate that this model is OK to translate
  if (!check(*m))
    return EXIT_FAILURE;

  // name any rules that are unnamed, so they get valid C symbols
  rumur::sanitise_rule_names(*m);

  // Determine if we have any == or != involving records or arrays, in which
  // case we will need to pack structs. See generate_c() for why.
  bool pack = compares_complex_values(*m);

  // parse comments from the source code
  std::vector<rumur::Comment> comments = rumur::parse_comments(*in.second);

  // output code
  if (source) {
    generate_c(*m, comments, pack, out == nullptr ? std::cout : *out);
  } else {
    generate_h(*m, comments, pack, out == nullptr ? std::cout : *out);
  }

  return EXIT_SUCCESS;
}
