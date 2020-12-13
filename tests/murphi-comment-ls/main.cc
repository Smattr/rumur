#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>

int main(int argc, char **argv) {

  if (argc > 2 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
    std::cerr << "Murphi comment lister\n"
              << " usage: " << argv[0] << " filename\n";
    return EXIT_FAILURE;
  }

  std::vector<rumur::Comment> comments;

  if (argc > 1) {

    std::ifstream in(argv[1]);
    if (!in) {
      std::cerr << "failed to open " << argv[1] << "\n";
      return EXIT_FAILURE;
    }

    comments = rumur::parse_comments(in);

  } else {
    comments = rumur::parse_comments(std::cin);
  }

  for (const rumur::Comment &c : comments)
    std::cout << c.loc << ": " << c.content << "\n";

  return EXIT_SUCCESS;
}
