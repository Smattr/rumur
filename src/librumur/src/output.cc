#include <iostream>
#include <fstream>
#include <rumur/Model.h>
#include <rumur/output.h>
#include "resources_header.h"
#include "resources_includes.h"
#include <string>

using namespace rumur;
using namespace std;

int rumur::output_checker(const string &path, const Model &model) {
    ofstream out(path);
    if (!out)
        return -1;

#define WRITE(resource) \
    do { \
        for (unsigned int i = 0; i < resources_##resource##_len; i++) { \
            out << resources_##resource[i]; \
        } \
        out << "\n"; \
    } while (0)

    WRITE(includes_cc);
    WRITE(header_cc);

#undef WRITE

    return 0;

}
