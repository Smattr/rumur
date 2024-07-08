#include "../../common/environ.h"
#include "../../common/help.h"
#include "ValueType.h"
#include "generate.h"
#include "has-start-state.h"
#include "log.h"
#include "optimise-field-ordering.h"
#include "options.h"
#include "resources.h"
#include "smt/except.h"
#include "smt/simplify.h"
#include "utils.h"
#include <algorithm>
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
#include <spawn.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

using namespace rumur;

static std::shared_ptr<std::istream> in;
static std::shared_ptr<std::string> out;

static unsigned string_to_percentage(const std::string &s) {
  int p;
  try {
    p = std::stoi(s);
    if (p < 1 || p > 100)
      throw std::invalid_argument("");
  } catch (std::out_of_range &) {
    throw std::invalid_argument("");
  }
  return (unsigned)p;
}

static void parse_args(int argc, char **argv) {

  for (;;) {
    enum {
      OPT_BOUND = 128,
      OPT_COLOUR,
      OPT_COUNTEREXAMPLE_TRACE,
      OPT_DEADLOCK_DETECTION,
      OPT_MAX_ERRORS,
      OPT_MONOPOLISE,
      OPT_OUTPUT_FORMAT,
      OPT_PACK_STATE,
      OPT_POINTER_BITS,
      OPT_REORDER_FIELDS,
      OPT_SANDBOX,
      OPT_SCALARSET_SCHEDULES,
      OPT_SMT_ARG,
      OPT_SMT_BITVECTORS,
      OPT_SMT_BUDGET,
      OPT_SMT_PATH,
      OPT_SMT_PRELUDE,
      OPT_SMT_SIMPLIFICATION,
      OPT_SYMMETRY_REDUCTION,
      OPT_TRACE,
      OPT_VALUE_TYPE,
      OPT_VERSION,
    };

    static struct option opts[] = {
        {"bound", required_argument, 0, OPT_BOUND},
        {"color", required_argument, 0, OPT_COLOUR},
        {"colour", required_argument, 0, OPT_COLOUR},
        {"counterexample-trace", required_argument, 0,
         OPT_COUNTEREXAMPLE_TRACE},
        {"deadlock-detection", required_argument, 0, OPT_DEADLOCK_DETECTION},
        {"debug", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"max-errors", required_argument, 0, OPT_MAX_ERRORS},
        {"monopolise", no_argument, 0, OPT_MONOPOLISE},
        {"monopolize", no_argument, 0, OPT_MONOPOLISE},
        {"output", required_argument, 0, 'o'},
        {"output-format", required_argument, 0, OPT_OUTPUT_FORMAT},
        {"pack-state", required_argument, 0, OPT_PACK_STATE},
        {"pointer-bits", required_argument, 0, OPT_POINTER_BITS},
        {"quiet", no_argument, 0, 'q'},
        {"reorder-fields", required_argument, 0, OPT_REORDER_FIELDS},
        {"sandbox", required_argument, 0, OPT_SANDBOX},
        {"scalarset-schedules", required_argument, 0, OPT_SCALARSET_SCHEDULES},
        {"set-capacity", required_argument, 0, 's'},
        {"set-expand-threshold", required_argument, 0, 'e'},
        {"smt-arg", required_argument, 0, OPT_SMT_ARG},
        {"smt-bitvectors", required_argument, 0, OPT_SMT_BITVECTORS},
        {"smt-budget", required_argument, 0, OPT_SMT_BUDGET},
        {"smt-path", required_argument, 0, OPT_SMT_PATH},
        {"smt-prelude", required_argument, 0, OPT_SMT_PRELUDE},
        {"smt-simplification", required_argument, 0, OPT_SMT_SIMPLIFICATION},
        {"symmetry-reduction", required_argument, 0, OPT_SYMMETRY_REDUCTION},
        {"threads", required_argument, 0, 't'},
        {"trace", required_argument, 0, OPT_TRACE},
        {"value-type", required_argument, 0, OPT_VALUE_TYPE},
        {"verbose", no_argument, 0, 'v'},
        {"version", no_argument, 0, OPT_VERSION},
        {0, 0, 0, 0},
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "de:ho:qs:t:v", opts, &option_index);

    if (c == -1)
      break;

    switch (c) {

    case 'd': // --debug
      options.log_level = LogLevel::DEBUG;
      set_log_level(options.log_level);
      break;

    case 'e': { // --set-expand-threshold ...
      bool valid = true;
      try {
        options.set_expand_threshold = string_to_percentage(optarg);
      } catch (std::invalid_argument &) {
        valid = false;
      }
      if (!valid) {
        std::cerr << "invalid --set-expand-threshold argument \"" << optarg
                  << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;
    }

    case 'h': // --help
      help(doc_rumur_1, doc_rumur_1_len);
      exit(EXIT_SUCCESS);

    case 'o': // --output ...
      out = std::make_shared<std::string>(optarg);
      break;

    case 'q': // --quiet
      options.log_level = LogLevel::SILENT;
      set_log_level(options.log_level);
      break;

    case 's': { // --set-capacity ...
      bool valid = true;
      try {
        options.set_capacity = optarg;
        if (options.set_capacity <= 0)
          valid = false;
      } catch (std::invalid_argument &) {
        valid = false;
      }
      if (!valid) {
        std::cerr << "invalid --set-capacity argument \"" << optarg << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;
    }

    case 't': { // --threads ...
      bool valid = true;
      try {
        options.threads = optarg;
        if (options.threads < 0)
          valid = false;
      } catch (std::invalid_argument &) {
        valid = false;
      }
      if (!valid) {
        std::cerr << "invalid --threads argument \"" << optarg << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;
    }

    case 'v': // --verbose
      options.log_level = LogLevel::INFO;
      set_log_level(options.log_level);
      break;

    case '?':
      std::cerr << "run `" << argv[0] << " --help` to see available options\n";
      exit(EXIT_FAILURE);

    case OPT_COLOUR: // --colour ...
      if (strcmp(optarg, "auto") == 0) {
        options.color = Color::AUTO;
      } else if (strcmp(optarg, "on") == 0) {
        if (options.machine_readable_output) {
          std::cerr << "colour is not supported in combination with "
                    << "--output-format \"machine readable\"\n";
          exit(EXIT_FAILURE);
        }
        options.color = Color::ON;
      } else if (strcmp(optarg, "off") == 0) {
        options.color = Color::OFF;
      } else {
        std::cerr << "invalid --colour argument \"" << optarg << "\"\n"
                  << "valid arguments are \"auto\", \"off\", and \"on\"\n";
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_TRACE: // --trace ...
      if (strcmp(optarg, "handle_reads") == 0) {
        options.traces |= TC_HANDLE_READS;
      } else if (strcmp(optarg, "handle_writes") == 0) {
        options.traces |= TC_HANDLE_WRITES;
      } else if (strcmp(optarg, "memory_usage") == 0) {
        options.traces |= TC_MEMORY_USAGE;
      } else if (strcmp(optarg, "queue") == 0) {
        options.traces |= TC_QUEUE;
      } else if (strcmp(optarg, "set") == 0) {
        options.traces |= TC_SET;
      } else if (strcmp(optarg, "symmetry_reduction") == 0) {
        options.traces |= TC_SYMMETRY_REDUCTION;
      } else if (strcmp(optarg, "all") == 0) {
        options.traces = uint64_t(-1);
      } else {
        std::cerr << "invalid --trace argument \"" << optarg << "\"\n"
                  << "valid arguments are \"handle_reads\", \"handle_writes\", "
                     "\"memory_usage\", \"queue\", \"set\", and "
                     "\"symmetry_reduction\"\n";
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_DEADLOCK_DETECTION: // --deadlock-detection ...
      if (strcmp(optarg, "off") == 0) {
        options.deadlock_detection = DeadlockDetection::OFF;
      } else if (strcmp(optarg, "stuck") == 0) {
        options.deadlock_detection = DeadlockDetection::STUCK;
      } else if (strcmp(optarg, "stuttering") == 0) {
        options.deadlock_detection = DeadlockDetection::STUTTERING;
      } else {
        std::cerr << "invalid argument to --deadlock-detection, \"" << optarg
                  << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_MONOPOLISE: { // --monopolise

      long pagesize = sysconf(_SC_PAGESIZE);
      if (pagesize < 0) {
        perror("failed to retrieve page size");
        exit(EXIT_FAILURE);
      }

      long physpages = sysconf(_SC_PHYS_PAGES);
      if (physpages < 0) {
        perror("failed to retrieve physical pages");
        exit(EXIT_FAILURE);
      }

      /* Allocate a set that will eventually cover all of memory upfront. Note
       * that this will never actually reach 100% occupancy because memory
       * also needs to contain our code and data as well as the OS.
       */
      options.set_capacity = size_t(pagesize) * size_t(physpages);

      // Never expand the set.
      options.set_expand_threshold = 100;

      break;
    }

    case OPT_PACK_STATE: // --pack-state ...
      if (strcmp(optarg, "on") == 0) {
        options.pack_state = true;
      } else if (strcmp(optarg, "off") == 0) {
        options.pack_state = false;
      } else {
        std::cerr << "invalid argument to --pack_state, \"" << optarg << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_POINTER_BITS: // --pointer-bits ...
      if (strcmp(optarg, "auto") == 0) {
        options.pointer_bits = 0;
      } else {
        bool valid = true;
        try {
          options.pointer_bits = optarg;
          if (options.pointer_bits <= 0)
            valid = false;
        } catch (std::invalid_argument &) {
          valid = false;
        }
        if (!valid) {
          std::cerr << "invalid --pointer-bits argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
      }
      break;

    case OPT_SYMMETRY_REDUCTION: // --symmetry-reduction ...
      if (strcmp(optarg, "off") == 0) {
        options.symmetry_reduction = SymmetryReduction::OFF;
      } else if (strcmp(optarg, "heuristic") == 0) {
        options.symmetry_reduction = SymmetryReduction::HEURISTIC;
      } else if (strcmp(optarg, "exhaustive") == 0) {
        options.symmetry_reduction = SymmetryReduction::EXHAUSTIVE;
      } else {
        std::cerr << "invalid argument to --symmetry-reduction, \"" << optarg
                  << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_SANDBOX: // --sandbox ...
      if (strcmp(optarg, "on") == 0) {
        options.sandbox_enabled = true;
      } else if (strcmp(optarg, "off") == 0) {
        options.sandbox_enabled = false;
      } else {
        std::cerr << "invalid argument to --sandbox, \"" << optarg << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_SCALARSET_SCHEDULES: // --scalarset-schedules ...
      if (strcmp(optarg, "on") == 0) {
        options.scalarset_schedules = true;
      } else if (strcmp(optarg, "off") == 0) {
        options.scalarset_schedules = false;
      } else {
        std::cerr << "invalid argument to --scalarset-schedules, \"" << optarg
                  << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_MAX_ERRORS: { // --max-errors ...
      bool valid = true;
      try {
        options.max_errors = optarg;
        if (options.max_errors <= 0)
          valid = false;
      } catch (std::invalid_argument &) {
        valid = false;
      }
      if (!valid) {
        std::cerr << "invalid --max-errors argument \"" << optarg << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;
    }

    case OPT_COUNTEREXAMPLE_TRACE: // --counterexample-trace ...
      if (strcmp(optarg, "full") == 0) {
        options.counterexample_trace = CounterexampleTrace::FULL;
      } else if (strcmp(optarg, "diff") == 0) {
        options.counterexample_trace = CounterexampleTrace::DIFF;
      } else if (strcmp(optarg, "off") == 0) {
        options.counterexample_trace = CounterexampleTrace::OFF;
      } else {
        std::cerr << "invalid argument to --counterexample-trace, \"" << optarg
                  << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_OUTPUT_FORMAT: // --output-format ...
      if (strcmp(optarg, "machine-readable") == 0) {
        options.machine_readable_output = true;
        // Disable colour that would interfere with XML
        options.color = Color::OFF;
      } else if (strcmp(optarg, "human-readable") == 0) {
        options.machine_readable_output = false;
      } else {
        std::cerr << "invalid argument to --output-format, \"" << optarg
                  << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_VERSION: // --version
      std::cout << "Rumur version " << get_version() << "\n";
      exit(EXIT_SUCCESS);

    case OPT_BOUND: { // --bound ...
      bool valid = true;
      try {
        options.bound = optarg;
        if (options.bound < 0)
          valid = false;
      } catch (std::invalid_argument &) {
        valid = false;
      }
      if (!valid) {
        std::cerr << "invalid --bound argument \"" << optarg << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;
    }

    case OPT_VALUE_TYPE: // --value-type ...
      options.value_type = optarg;
      break;

    case OPT_REORDER_FIELDS: // --reorder-fields ...
      if (strcmp(optarg, "on") == 0) {
        options.reorder_fields = true;
      } else if (strcmp(optarg, "off") == 0) {
        options.reorder_fields = false;
      } else {
        std::cerr << "invalid argument to --reorder-fields, \"" << optarg
                  << "\"\n";
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_SMT_ARG: // --smt-arg ...
      options.smt.args.emplace_back(optarg);
      if (options.smt.simplification == SmtSimplification::AUTO) {
        options.smt.simplification = SmtSimplification::ON;
      }
      break;

    case OPT_SMT_BITVECTORS: // --smt-bitvectors ...
      if (strcmp(optarg, "on") == 0) {
        options.smt.use_bitvectors = true;
      } else if (strcmp(optarg, "off") == 0) {
        options.smt.use_bitvectors = false;
      } else {
        std::cerr << "invalid argument to --smt-bitvectors, \"" << optarg
                  << "\"\n";
        exit(EXIT_FAILURE);
      }
      if (options.smt.simplification == SmtSimplification::AUTO) {
        options.smt.simplification = SmtSimplification::ON;
      }
      break;

    case OPT_SMT_BUDGET: { // --smt-budget ...
      bool valid = true;
      try {
        options.smt.budget = optarg;
        if (options.smt.budget < 0)
          valid = false;
      } catch (std::invalid_argument &) {
        valid = false;
      }
      if (!valid) {
        std::cerr << "invalid --smt-budget, \"" << optarg << "\"\n";
        exit(EXIT_FAILURE);
      }
      if (options.smt.simplification == SmtSimplification::AUTO) {
        options.smt.simplification = SmtSimplification::ON;
      }
      break;
    }

    case OPT_SMT_PATH: // --smt-path ...
      options.smt.path = optarg;
      if (options.smt.simplification == SmtSimplification::AUTO) {
        options.smt.simplification = SmtSimplification::ON;
      }
      break;

    case OPT_SMT_PRELUDE: // --smt-prelude ...
      options.smt.prelude.emplace_back(optarg);
      if (options.smt.simplification == SmtSimplification::AUTO) {
        options.smt.simplification = SmtSimplification::ON;
      }
      break;

    case OPT_SMT_SIMPLIFICATION: // --smt-simplification ...
      if (strcmp(optarg, "on") == 0) {
        options.smt.simplification = SmtSimplification::ON;
      } else if (strcmp(optarg, "off") == 0) {
        options.smt.simplification = SmtSimplification::OFF;
      } else {
        std::cerr << "invalid argument to --smt-simplification, \"" << optarg
                  << "\"\n";
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
                << "\n";
      exit(EXIT_FAILURE);
    }

    if (S_ISDIR(buf.st_mode)) {
      std::cerr << "failed to open " << argv[optind]
                << ": this is a directory\n";
      exit(EXIT_FAILURE);
    }

    auto inf = std::make_shared<std::ifstream>(argv[optind]);
    if (!inf->is_open()) {
      std::cerr << "failed to open " << argv[optind] << "\n";
      exit(EXIT_FAILURE);
    }
    input_filename = argv[optind];
    in = inf;
  }

  if (out == nullptr) {
    std::cerr << "output file is required\n";
    exit(EXIT_FAILURE);
  }

  if (options.threads == 0) {
    // automatic
    long r = sysconf(_SC_NPROCESSORS_ONLN);
    if (r < 1) {
      options.threads = 1;
    } else {
      options.threads = r;
    }
  }

  if (options.smt.simplification == SmtSimplification::ON &&
      options.smt.path == "") {
    *warn << "SMT simplification was enabled but no path was provided to the "
          << "solver (--smt-path ...), so it will be disabled\n";
    options.smt.simplification = SmtSimplification::OFF;
  }
}

static bool use_colors() {
  return options.color == Color::ON ||
         (options.color == Color::AUTO && isatty(STDERR_FILENO));
}

static std::string bold() {
  if (use_colors())
    return "\033[1m";
  return "";
}

static std::string green() {
  if (use_colors())
    return "\033[32m";
  return "";
}

static std::string red() {
  if (use_colors())
    return "\033[31m";
  return "";
}

static std::string reset() {
  if (use_colors())
    return "\033[0m";
  return "";
}

static std::string white() {
  if (use_colors())
    return "\033[37m";
  return "";
}

static void print_location(const std::string &file, const location &location) {

  // don't try to open stdin (see default in options.cc)
  if (file == "<stdin>")
    return;

  std::ifstream f(file);
  if (!f.is_open()) {
    // ignore failure here as it's non-critical
    return;
  }

  // the type of position.line and position.column changes across Bison
  // releases, so avoid some -Wsign-compare warnings by casting them in advance
  auto loc_line = static_cast<unsigned long>(location.begin.line);
  auto loc_col = static_cast<unsigned long>(location.begin.column);

  std::string line;
  unsigned long lineno = 0;
  while (lineno < loc_line) {
    if (!std::getline(f, line))
      return;
    lineno++;
  }

  // print the line, and construct an underline indicating the column location
  std::ostringstream buf;
  unsigned long col = 1;
  for (const char &c : line) {
    if (col == loc_col) {
      buf << green() << bold() << "^" << reset();
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
  std::cerr << "\n";

  std::cerr << buf.str() << "\n";
}

int main(int argc, char **argv) {

  // Parse command line options
  parse_args(argc, argv);

  // Parse input model
  *debug << "parsing input model...\n";
  Ptr<Model> m;
  try {
    m = parse_model(in == nullptr ? std::cin : *in);
  } catch (Error &e) {
    std::cerr << white() << bold() << input_filename << ":" << e.loc << ":"
              << reset() << " " << red() << bold() << "error:" << reset() << " "
              << white() << bold() << e.what() << reset() << "\n";
    print_location(input_filename, e.loc);
    return EXIT_FAILURE;
  }

  // We no longer need the input file. Close it here to avoid having to think
  // about close-on-exec when running the compiler or an SMT solver.
  in = nullptr;

  assert(m != nullptr);

  /* Re-index the model (assign unique identifiers to each node that are used in
   * generation of the verifier).
   */
  *debug << "re-indexing...\n";
  m->reindex();

  // resolve symbolic references and validate the model
  try {
    *debug << "resolving symbols...\n";
    resolve_symbols(*m);
    *debug << "validating AST...\n";
    validate(*m);
  } catch (Error &e) {
    std::cerr << white() << bold() << input_filename << ":" << e.loc << ":"
              << reset() << " " << red() << bold() << "error:" << reset() << " "
              << white() << bold() << e.what() << reset() << "\n";
    print_location(input_filename, e.loc);
    return EXIT_FAILURE;
  }

  // Check whether we have a start state.
  if (!has_start_state(*m))
    *warn << "warning: model has no start state\n";

  // run SMT simplification if the user enabled it
  if (options.smt.simplification == SmtSimplification::ON) {
    *debug << "SMT simplification...\n";
    try {
      smt::simplify(*m);
    } catch (smt::BudgetExhausted &) {
      *info << "SMT solver budget (" << options.smt.budget << "ms) exhausted\n";
    } catch (smt::Unsupported &e) {
      if (e.expr != nullptr)
        *info << e.expr->loc << ": ";
      *info << e.what() << "\n";
    }
  }

  // re-order fields to optimise access to them
  if (options.reorder_fields) {
    *debug << "optimising field ordering...\n";
    optimise_field_ordering(*m);
  }

  // get value_t to use in the checker
  *debug << "determining value_t type...\n";
  std::pair<ValueType, ValueType> value_types;
  try {
    value_types = get_value_type(options.value_type, *m);
  } catch (std::runtime_error &e) {
    std::cerr << "invalid --value-type " << options.value_type << ": "
              << e.what() << "\n";
    return EXIT_FAILURE;
  }

  *debug << "generating verifier...\n";
  assert(out != nullptr);
  if (output_checker(*out, *m, value_types) != 0)
    return EXIT_FAILURE;

#ifndef __AFL_COMPILER
#define __AFL_COMPILER 0
#endif

  if (__AFL_COMPILER) { // extra steps for when we're being fuzzed

    // find the C compiler
    const char *cc = getenv("CC");
    if (cc == nullptr)
      cc = "cc";

    // setup an argument vector for calling the C compiler
    const char *args[] = {cc,
                          "-std=c11",
                          "-x",
                          "c",
                          "-o",
                          "/dev/null",
                          "-march=native",
                          "-Werror=format",
                          "-Werror=sign-compare",
                          "-Werror=type-limits",
                          out->c_str(),
#ifdef __x86_64__
                          "-mcx16",
#endif
                          "-lpthread"};

    // start the C compiler
    int r = posix_spawnp(nullptr, args[0], nullptr, nullptr,
                         const_cast<char *const *>(args), get_environ());
    if (r != 0) {
      std::cerr << "posix_spawnp failed: " << strerror(r) << "\n";
      abort();
    }

    // wait for it to finish
    int stat_loc;
    pid_t pid = wait(&stat_loc);
    if (pid == -1) {
      std::cerr << "wait failed\n";
      abort();
    }

    // if it terminated abnormally, pass an error up to the fuzzer
    if (WIFSIGNALED(stat_loc) || WIFSTOPPED(stat_loc)) {
      std::cerr << "subprocess either signalled or stopped\n";
      abort();
    }

    assert(WIFEXITED(stat_loc));
  }

  return EXIT_SUCCESS;
}
