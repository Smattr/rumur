#pragma once

#include <cstddef>
#include <cassert>
#include <memory>
#include <rumur/rumur.h>
#include "Stage.h"
#include <vector>

class Pipeline {

 private:
  std::vector<Stage*> stages;

  std::vector<std::shared_ptr<Stage>> managed;

 public:
  void add_stage(Stage &s);

  template<typename T>
  void make_stage() {
    assert(!stages.empty() && "make_stage() on an empty pipeline");

    auto s = std::make_shared<T>(*stages[0]);
    managed.push_back(s);

    add_stage(*s);
  }

  void process(const rumur::Node &n);

  void finalise();
};
