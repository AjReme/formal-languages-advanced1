#include <stdlib.h>
#include "test_func.h"


int main() {
    exit(calculate("tests/test4.txt") != 4);
}
