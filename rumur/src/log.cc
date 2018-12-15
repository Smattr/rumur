#include <iostream>
#include "log.h"
#include "options.h"
#include <streambuf>

// A buffer that discards data written to it
namespace {
  class NullBuf : public std::streambuf {
   public:
    int overflow(int c) {
      return c;
    }
  };
}

// A stream that discards data written to it
NullBuf null_buf;
std::ostream null(&null_buf);

std::ostream *debug = &null;
std::ostream *info = &null;
std::ostream *warn = &std::cerr;

void set_log_level(log_level_t level) {

  switch (level) {

    case SILENT:
      debug = &null;
      info = &null;
      warn = &null;
      break;

    case WARNINGS:
      debug = &null;
      info = &null;
      warn = &std::cerr;
      break;

    case INFO:
      debug = &null;
      info = &std::cerr;
      warn = &std::cerr;
      break;

    case DEBUG:
      debug = &std::cerr;
      info = &std::cerr;
      warn = &std::cerr;
      break;

  }
}
