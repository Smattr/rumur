// a stage for removing extended unicode characters like ≔

#pragma once

#include <cstddef>
#include "Stage.h"
#include <string>

class ToAscii : public IntermediateStage {

 private:
  // states for write() state machine
  enum State {
    IDLE,
    IDLE_DASH, // saw '-'
    IDLE_SLASH, // saw '/'
    IN_STRING, // within a string
    IN_STRING_SLASH, // within string and saw a '\'
    IN_LINE_COMMENT, // within a -- comment
    IN_MULTILINE_COMMENT, // within a /* */ comment
    IN_MULTILINE_COMMENT_STAR, // within a /* */ comment and saw '*'
  };

  State state = IDLE;

  // do we need to insert a space if we do not see one next?
  bool pending_space = false;

 public:
  explicit ToAscii(Stage &next_);

  void write(const std::string &c) final;

  virtual ~ToAscii() = default;
};
