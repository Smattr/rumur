#include <cstddef>
#include <fstream>
#include <iostream>
#include "generate.h"
#include "options.h"
#include "resources.h"
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

static std::ostream &operator<<(std::ostream &out, deadlock_detection_t d) {
  switch (d) {

    case DEADLOCK_DETECTION_OFF:
      out << "DEADLOCK_DETECTION_OFF";
      break;

    case DEADLOCK_DETECTION_STUCK:
      out << "DEADLOCK_DETECTION_STUCK";
      break;

    case DEADLOCK_DETECTION_STUTTERING:
      out << "DEADLOCK_DETECTION_STUTTERING";
      break;

  }

  return out;
}

static std::ostream &operator<<(std::ostream &out, symmetry_reduction_t s) {
  switch (s) {

    case SYMMETRY_REDUCTION_OFF:
      out << "SYMMETRY_REDUCTION_OFF";
      break;

    case SYMMETRY_REDUCTION_HEURISTIC:
      out << "SYMMETRY_REDUCTION_HEURISTIC";
      break;

    case SYMMETRY_REDUCTION_EXHAUSTIVE:
      out << "SYMMETRY_REDUCTION_EXHAUSTIVE";
      break;

  }

  return out;
}

int output_checker(const std::string &path, const Model &model) {

  std::ofstream out(path);
  if (!out)
    return -1;

  if (options.log_level < DEBUG)
    out << "#define NDEBUG 1\n\n";

  out

    // #includes
    << std::string((const char*)resources_includes_c, (size_t)resources_includes_c_len)
    << "\n"

    // Settings that are used in header.c
    << "enum { SET_CAPACITY = " << options.set_capacity << "ul };\n\n"
    << "enum { SET_EXPAND_THRESHOLD = " << options.set_expand_threshold << " };\n\n"
    << "static const enum { OFF, ON, AUTO } COLOR = " << (options.color == OFF ? "OFF" :
      options.color == ON ? "ON" : "AUTO") << ";\n\n"
    << "enum trace_category_t {\n"
    << "  TC_HANDLE_READS       = " << TC_HANDLE_READS << ",\n"
    << "  TC_HANDLE_WRITES      = " << TC_HANDLE_WRITES << ",\n"
    << "  TC_QUEUE              = " << TC_QUEUE << ",\n"
    << "  TC_SET                = " << TC_SET << ",\n"
    << "  TC_SYMMETRY_REDUCTION = " << TC_SYMMETRY_REDUCTION << ",\n"
    << "};\n"
    << "static const uint64_t TRACES_ENABLED = UINT64_C(" << options.traces << ");\n\n"
    << "static const enum {\n"
    << "  DEADLOCK_DETECTION_OFF,\n"
    << "  DEADLOCK_DETECTION_STUCK,\n"
    << "  DEADLOCK_DETECTION_STUTTERING,\n"
    << "} DEADLOCK_DETECTION = " << options.deadlock_detection << ";\n\n"
    << "static const enum {\n"
    << "  SYMMETRY_REDUCTION_OFF,\n"
    << "  SYMMETRY_REDUCTION_HEURISTIC,\n"
    << "  SYMMETRY_REDUCTION_EXHAUSTIVE,\n"
    << "} SYMMETRY_REDUCTION = " << options.symmetry_reduction << ";\n\n"
    << "enum { SANDBOX_ENABLED = " << options.sandbox_enabled << " };\n\n"
    << "enum { MAX_ERRORS = " << options.max_errors << "ul };\n\n"
    << "enum { THREADS = " << options.threads << "ul };\n\n"
    << "enum { STATE_SIZE_BITS = " << model.size_bits() << "ul };\n\n"
    << "enum { ASSUMPTION_COUNT = " << model.assumption_count() << "ul };\n\n"
    << "#define CEX_OFF 0\n"
    << "#define DIFF 1\n"
    << "#define FULL 2\n"
    << "#define COUNTEREXAMPLE_TRACE " << (options.counterexample_trace == CEX_OFF
      ? "CEX_OFF" : (options.counterexample_trace == DIFF ? "DIFF" : "FULL")) << "\n\n"
    << "enum { MACHINE_READABLE_OUTPUT = " << options.machine_readable_output
      << " };\n\n"

    // Static boiler plate code
    << std::string((const char*)resources_header_c, (size_t)resources_header_c_len)
    << "\n";

  // the model itself
  generate_model(out, model);

  return 0;
}
