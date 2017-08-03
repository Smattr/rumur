#include <cassert>
#include <iostream>
#include "parser.yy.hh"
#include <rumur/Model.h>
#include <rumur/scanner.h>
#include <unistd.h>

using namespace rumur;
using namespace std;

int main(void) {

    scanner s(&cin);
    Model *m = nullptr;
    parser p(s, m);

    int err;
    try {
        err = p.parse();
    } catch (RumurError &e) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    if (err != 0)
        return EXIT_FAILURE;

    assert(m != nullptr);
    try {
        m->validate();
    } catch (RumurError &e) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
