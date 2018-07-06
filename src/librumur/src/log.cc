#include <iostream>
#include <rumur/log.h>
#include <streambuf>

// Define an output stream buffer that discards everything written to it.
namespace {
  class NullBuffer : public std::streambuf {
   public:
    int overflow(int c = EOF) final {
      return c;
    }
  };
}

// Use the above buffer to define a stream that discards its input.
static NullBuffer null_buffer;
static std::ostream null_stream(&null_buffer);

namespace rumur {

Log::Log(std::ostream &out, Level level):
  warn(level >= WARNINGS ? out : null_stream),
  info(level >= INFORMATIONAL ? out : null_stream),
  debug(level >= DEBUG ? out : null_stream) { }

Log null_log(null_stream, Log::WARNINGS);

}
