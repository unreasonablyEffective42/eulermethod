#include "tinyexpr.h"
#include <iostream>

int main() {
  te_parser tep;
  std::string expr;
  std::cin >> expr;
  auto response = tep.evaluate(expr);
  std::cout << response;
}
