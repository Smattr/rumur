#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <rumur/Comment.h>
#include <sstream>
#include <string>
#include <vector>

using namespace rumur;

namespace {

/// a file interface that supports lookahead
class File {

private:
  position pos;

  std::istream &in;
  std::string buffered;

public:
  explicit File(std::istream &in_) : pos(nullptr, 1, 1), in(in_) {}

  /// read a new character from the file
  char getchar() {

    assert(!buffered.empty());

    char c = buffered[0];
    buffered = buffered.substr(1);

    if (c == '\n') {
      pos.lines();
    } else {
      pos.columns();
    }

    return c;
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

  position get_position() const { return pos; }
};

} // namespace

std::vector<Comment> rumur::parse_comments(std::istream &input) {

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

    if (in.next_is("--")) { // single line comment?
      position begin = in.get_position();
      // discard the comment starter
      (void)in.read(strlen("--"));
      // consume the comment body
      std::ostringstream content;
      while (!in.eof() && !in.next_is("\n"))
        content << in.getchar();
      result.push_back(
          Comment{content.str(), false, location(begin, in.get_position())});
      continue;

    } else if (in.next_is("/*")) { // multiline comment?
      position begin = in.get_position();
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
      result.push_back(
          Comment{content.str(), true, location(begin, in.get_position())});

    } else { // otherwise, something irrelevant
      (void)in.getchar();
    }
  }

  return result;
}
