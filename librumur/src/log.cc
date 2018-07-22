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

Log::Log(std::ostream &stream, Level level_):
  out(&stream), level(level_) {
  update_streams();
}

void Log::set_level(Log::Level level_) {
  level = level_;
  update_streams();
}

void Log::set_stream(std::ostream &stream) {
  out = &stream;
  update_streams();
}

void Log::update_streams(void) {
  warn = level >= WARNING ? out : &null_stream;
  info = level >= INFO ? out : &null_stream;
  debug = level >= DEBUG ? out : &null_stream;
}

Log log;

}
