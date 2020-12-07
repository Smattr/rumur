#include <cstddef>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <rumur/Comment.h>
#include <sstream>
#include <string>
#include <vector>

namespace rumur {

namespace {

/// a file interface that supports lookahead
class File {

 private:
  uint64_t lineno = 1;
  uint64_t colno = 1;

  std::istream &in;
  std::string buffered;

 public:
  explicit File(std::istream &in_): in(in_) { }

  /// read a new character from the file
  int getchar() {

    // do we need to read a new character from the stream?
    if (buffered.empty())
      (void)peek();

    // did peek() fail?
    if (buffered.empty())
      return EOF;

    char c = buffered[0];
    buffered = buffered.substr(1);

    if (c == '\n') {
      ++lineno;
      colno = 1;
    } else {
      ++colno;
    }

    return static_cast<int>(c);
  }

  /// read the next `count` characters
  std::string read(size_t count) {
    std::string result;
    for (size_t i = 0; !eof() && i < count; ++i)
      result += getchar();
    return result;
  }

  /// lookahead at the next `count` characters while retaining them
  std::string peek(size_t count = 1) {

    // buffer enough to support our lookahead
    while (buffered.size() < count && in) {
      char c;
      if (in.read(&c, sizeof(c)))
        buffered += c;
    }

    return buffered.substr(0, count);
  }

  /// are the upcoming characters the given string?
  bool next_is(const std::string &expectation) {
    return peek(expectation.size()) == expectation;
  }

  /// have we reached the end of the file?
  bool eof() {

    // do we have known remaining characters?
    if (!buffered.empty())
      return false;

    // ensure buffer is populated or we have triggered EOF on the stream
    (void)peek();

    return buffered.empty() && in.eof();
  }

  uint64_t line() const {
    return lineno;
  }

  uint64_t col() const {
    return colno;
  }

  position pos() const {
    return position{nullptr, static_cast<position::counter_type>(line()),
      static_cast<position::counter_type>(col())};
  }
};

}

std::vector<Comment> parse_comments(std::istream &input) {

  std::vector<Comment> result;

  for (File in(input); !in.eof();) {

    // string?
    {
      bool is_quote = in.next_is("\"");
      bool is_smart_quote = in.next_is("“");
      if (is_quote || is_smart_quote) {
        // discard the quote starter
        (void)in.read(strlen(is_quote ? "\"" : "“"));
        // swallow the quote itself
        while (!in.eof()) {
          if (in.next_is("\\\"")) {
            (void)in.read(strlen("\\\""));
          } else if (in.next_is("\\”")) {
            (void)in.read(strlen("\\”"));
          } else if (in.next_is("\"")) {
            (void)in.read(strlen("\""));
            break;
          } else if (in.next_is("”")) {
            (void)in.read(strlen("”"));
            break;
          } else {
            (void)in.getchar();
          }
        }
        continue;
      }
    }

    // single line comment?
    if (in.next_is("//")) {
      position begin = in.pos();
      // discard the comment starter
      (void)in.read(strlen("//"));
      // consume the comment body
      std::ostringstream content;
      while (!in.eof() && !in.next_is("\n"))
        content << in.getchar();
      result.push_back(Comment{content.str(), false, location{begin, in.pos()}});
      continue;

    // multiline comment?
    } else if (in.next_is("/*")) {
      position begin = in.pos();
      // discard the comment starter
      (void)in.read(strlen("/*"));
      // consume the comment body;
      std::ostringstream content;
      while (!in.eof()) {
        if (in.next_is("*/")) {
          (void)in.read(strlen("*/"));
          break;
        }
        content << in.getchar();
      }
      result.push_back(Comment{content.str(), true, location{begin, in.pos()}});

    // otherwise, something irrelevant
    } else {
      (void)in.getchar();

    }
  }

  return result;
}

}
