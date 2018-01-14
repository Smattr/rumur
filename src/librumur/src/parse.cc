#include <cassert>
#include <iostream>
#include "location.hh"
#include "parser.yy.hh"
#include <rumur/except.h>
#include <rumur/Indexer.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/parse.h>
#include <rumur/scanner.h>
#include <rumur/Symtab.h>

namespace rumur {

Model *parse(std::istream *input) {

    assert(input != nullptr);

    // Setup the parser
    Symtab symtab;
    symtab.open_scope();
    scanner s(input);
    Model *m = nullptr;
    Indexer indexer;
    parser p(s, m, &symtab, indexer);

    // Parse the input model
    int err = p.parse();
    if (err != 0)
        throw RumurError("parsing failed", location());

    /* Validate that the model makes sense. E.g. constant definitions are
     * actually constant.
     */
    assert(m != nullptr);
    m->validate();

    return m;
}

}
