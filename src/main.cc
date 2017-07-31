#include <iostream>
#include "parser.yy.hh"
#include "Model.h"
#include "Scanner.h"
#include <unistd.h>

using namespace rumur;
using namespace std;

int main(void) {

    Scanner s(&cin);
    Model *m = nullptr;
    parser p(s, m);

    int err = p.parse();

    return err == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
