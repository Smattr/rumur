#include <algorithm>
#include <cassert>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include <unistd.h>

static std::istream *in;
static std::string *out;
static rumur::OutputOptions output_options = {
  .overflow_checks = true,
  .threads = 0,
  .debug = false,
  .set_capacity = 8 * 1024 * 1024,
  .set_expand_threshold = 65,
  .tbb = false,
};

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option options[] = {
      { "debug", no_argument, 0, 'd' },
      { "help", no_argument, 0, '?' },
      { "output", required_argument, 0, 'o' },
      { "set-capacity", required_argument, 0, 's' },
      { "set-expand-threshold", required_argument, 0, 'e' },
      { "threads", required_argument, 0, 't' },
      { "tbb", no_argument, 0, 128 },
      { 0, 0, 0, 0 },
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "de:o:s:t:?", options, &option_index);

    if (c == -1)
      break;

    switch (c) {

      case 'd':
        output_options.debug = true;
        break;

      case 'e':
        try {
          output_options.set_expand_threshold = std::stoul(optarg);
        } catch (std::exception) {
          std::cerr << "invalid --set-expand-threshold argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        if (output_options.set_expand_threshold < 1 ||
            output_options.set_expand_threshold > 100) {
          std::cerr << "invalid --set-expand-threshold argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 'o':
        if (out != nullptr)
          delete out;
        out = new std::string(optarg);
        break;

      case 's':
        try {
          output_options.set_capacity = std::stoul(optarg);
        } catch (std::exception) {
          std::cerr << "invalid --set-capacity argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 't':
        try {
          output_options.threads = std::stoul(optarg);
        } catch (std::exception) {
          std::cerr << "invalid --threads argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case '?':
        std::cerr
          << "usage: " << argv[0] << " [options...] --output FILE [FILE]\n"
          << "\n"
          << "options:\n\n"
          << " --debug | -d\n"
          << "     Enable debugging options in the generated checker. This\n"
          << "     includes enabling runtime assertions.\n\n"
          << " --help\n"
          << "     Display the present information.\n\n"
          << " --set-capacity SIZE | -s SIZE\n"
          << "     Specify a static size (in bytes) of the seen state set. When\n"
          << "     this size is reached, the checker will abort if it has not\n"
          << "     yet covered the state space. A size of 0 indicates a\n"
          << "     dynamically expanding seen state set which is the default.\n\n"
          << " --set-expand-threshold PERCENT | -e PERCENT\n"
          << "     Expand the state set when its occupancy exceeds this\n"
          << "     percentage. Default 65, valid values 1 - 100.\n\n"
          << " --threads COUNT | -t COUNT\n"
          << "     Specify the number of threads the checker should use. If you\n"
          << "     do not specify this parameter or pass 0, the number of\n"
          << "     threads will be chosen based on the available hardware\n"
          << "     threads on the platform on which you generate the model.\n\n"
          << " --tbb\n"
          << "     Use Intel Thread Building Blocks. When using a dynamically\n"
          << "     expanding state set, Intel TBB's concurrent_unordered_set\n"
          << "     will be used to store states which can potentially result in\n"
          << "     a speed up. This requires you to have the TBB headers and\n"
          << "     library available when building the checker and will only be\n"
          << "     used by a multithreaded checker.\n";
        exit(EXIT_FAILURE);

      case 128:
        output_options.tbb = true;
        break;

      default:
        std::cerr << "unexpected error\n";
        exit(EXIT_FAILURE);

    }
  }

  if (optind == argc - 1) {
    auto inf = new std::ifstream(argv[optind]);
    if (!inf->is_open()) {
      std::cerr << "failed to open " << argv[optind] << "\n";
      exit(EXIT_FAILURE);
    }
    in = inf;
  } else {
    in = &std::cin;
  }

  if (out == nullptr) {
    std::cerr << "output file is required\n";
    exit(EXIT_FAILURE);
  }

  if (output_options.threads == 0) {
    // automatic
    long r = sysconf(_SC_NPROCESSORS_ONLN);
    if (r < 1) {
      output_options.threads = 1;
    } else {
      output_options.threads = static_cast<unsigned long>(r);
    }
  }
}

template<typename T, typename U>
static bool contains(const T &container, U predicate) {
  return std::find_if(container.begin(), container.end(), predicate)
    != container.end();
}

static bool has_start_state(const rumur::Rule *r) {
  if (dynamic_cast<const rumur::StartState*>(r))
    return true;
  if (auto s = dynamic_cast<const rumur::Ruleset*>(r))
    return contains(s->rules, has_start_state);
  return false;
}

static void validate(const rumur::Model &m) {

  // Check whether we have a start state.
  if (!contains(m.rules, has_start_state))
    std::cerr << "warning: model has no start state\n";

}

int main(int argc, char **argv) {

  // Parse command line options
  parse_args(argc, argv);

  // Parse input model
  rumur::Model *m;
  try {
    m = rumur::parse(in);
  } catch (rumur::Error &e) {
    std::cerr << e.loc << ":" << e.what() << "\n";
    return EXIT_FAILURE;
  }

  assert(m != nullptr);
  validate(*m);

  assert(out != nullptr);
  if (rumur::output_checker(*out, *m, output_options) != 0)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
