#pragma once

#include <functional>
#include "State.cc"
#include <string>

struct StartState {
    std::string name;
    std::function<void(State&)> body;
};

struct Invariant {
    std::string name;
    std::function<bool(const State&)> guard;
};

struct Rule {
    std::string name;
    std::function<bool(const State&)> guard;
    std::function<void(State&)> body;
};
