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

    int err = p.parse();

    return err == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
