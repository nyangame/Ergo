#include "test_framework.hpp"
#include <cstdio>

int main() {
    std::printf("=== Ergo Engine Tests ===\n\n");
    return test::run_all();
}
