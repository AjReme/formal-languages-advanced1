#include <stdlib.h>
#include "test_func.h"


int main() {
    exit(calculate("tests/test6.txt") != 17);
}
