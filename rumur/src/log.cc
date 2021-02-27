#include "log.h"
#include "options.h"
#include <cstddef>
#include <iostream>
#include <streambuf>

// A buffer that discards data written to it
namespace {
class NullBuf : public std::streambuf {
public:
  int overflow(int c) { return c; }
};
} // namespace

// A stream that discards data written to it
NullBuf null_buf;
std::ostream null(&null_buf);

std::ostream *debug = &null;
std::ostream *info = &null;
std::ostream *warn = &std::cerr;

void set_log_level(LogLevel level) {

  switch (level) {

  case LogLevel::SILENT:
    debug = &null;
    info = &null;
    warn = &null;
    break;

  case LogLevel::WARNINGS:
    debug = &null;
    info = &null;
    warn = &std::cerr;
    break;

  case LogLevel::INFO:
    debug = &null;
    info = &std::cerr;
    warn = &std::cerr;
    break;

  case LogLevel::DEBUG:
    debug = &std::cerr;
    info = &std::cerr;
    warn = &std::cerr;
    break;
  }
}
