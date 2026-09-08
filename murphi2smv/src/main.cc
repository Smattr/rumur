#include "../../common/help.h"
#include "codegen.h"
#include "pick_numeric_type.h"
#include "resources.h"
#include <cassert>
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

static const char *in_filename = "<stdin>";
static std::shared_ptr<std::istream> in;
static std::string out_filename{"-"};
static std::shared_ptr<std::ostream> out;

std::string numeric_type;

/// use colour in error messages?
static enum { AUTO, ON, OFF } color = AUTO;

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option options[] = {
        // clang-format off
        { "color",        required_argument, 0, 129 },
        { "colour",       required_argument, 0, 129 },
        { "help",         no_argument,       0, 'h' },
        { "numeric-type", required_argument, 0, 'n' },
        { "output",       required_argument, 0, 'o' },
        { "version",      no_argument,       0, 128 },
        { 0, 0, 0, 0 },
        // clang-format on
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "hn:o:", options, &option_index);

    if (c == -1)
      break;

    switch (c) {

    case '?':
      std::cerr << "run `" << argv[0] << " --help` to see available options\n";
      exit(EXIT_SUCCESS);

    case 'h': // --help
      help(doc_murphi2smv_1, doc_murphi2smv_1_len);
      exit(EXIT_SUCCESS);

    case 'n': // --numeric-type
      if (optarg != std::string{"integer"} && optarg != std::string{"word"}) {
        std::cerr << "invalid argument to --numeric-type " << optarg << '\n';
        exit(EXIT_FAILURE);
      }
      numeric_type = optarg;
      break;

    case 'o': // --output
      out_filename = optarg;
      break;

    case 128: // --version
      std::cout << "Murphi2SMV version " << rumur_get_version() << '\n';
      exit(EXIT_SUCCESS);

    case 129: // --color, --colour
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
    if (!i->is_open()) {
      std::cerr << "failed to open " << in_filename << '\n';
      exit(EXIT_FAILURE);
    }
    in = i;
  }
}

static std::shared_ptr<std::istream> make_stdin_buf() {

  // read stdin into memory
  auto buffer = std::make_shared<std::stringstream>();
  *buffer << std::cin.rdbuf();

  return buffer;
}

static std::ostream &output() { return out == nullptr ? std::cout : *out; }

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

  // if we are reading from stdin, buffer it so that we can seek it
  if (in == nullptr)
    in = make_stdin_buf();

  // parse input
  rumur::Ptr<rumur::Node> parsed;
  try {
    parsed = rumur::parse(*in);
  } catch (rumur::Error &e) {
    std::cerr << white() << bold() << in_filename << ':' << e.loc << ':'
              << reset() << ' ' << red() << bold() << "error:" << reset() << ' '
              << white() << bold() << e.what() << reset() << '\n';
    in->seekg(0);
    print_location(*in, e.loc);
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
      std::cerr << white() << bold() << in_filename << ':' << e.loc << ':'
                << reset() << ' ' << red() << bold() << "error:" << reset()
                << ' ' << white() << bold() << e.what() << reset() << '\n';
      in->seekg(0);
      print_location(*in, e.loc);
      return EXIT_FAILURE;
    }
  }

  // if the user did not select a numeric type, select one for them
  if (numeric_type == "")
    numeric_type = pick_numeric_type(*parsed);

  // parse comments from the source code
  in->seekg(0);
  std::vector<rumur::Comment> comments = rumur::parse_comments(*in);

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

  // generate SMV source code
  codegen(*parsed, comments, output());

  return EXIT_SUCCESS;
}
