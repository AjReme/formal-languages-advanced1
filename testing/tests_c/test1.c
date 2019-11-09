#include <stdlib.h>
#include "test_func.h"


int main() {
    exit(calculate("tests/test1.txt") != 9);
}
