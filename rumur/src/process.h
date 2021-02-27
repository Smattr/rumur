#pragma once

#include <cstddef>
#include <string>
#include <vector>

/* Run an external process, pass it the given input on stdin and wait for it to
 * finish. Its stdout data is reported via the output parameter. Note that there
 * is currently no reporting of the exit status of the process.
 */
int run(const std::vector<std::string> &args, const std::string &input,
        std::string &output);
