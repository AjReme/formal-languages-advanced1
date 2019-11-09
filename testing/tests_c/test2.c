#include <stdlib.h>
#include "test_func.h"


int main() {
    exit(calculate("tests/test2.txt") != 25);
}
