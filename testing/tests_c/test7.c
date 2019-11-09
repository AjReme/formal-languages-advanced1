#include <stdlib.h>
#include "test_func.h"


int main() {
    exit(calculate("tests/test7.txt") != -2000000000);
}
