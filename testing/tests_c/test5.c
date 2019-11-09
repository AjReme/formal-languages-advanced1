#include <stdlib.h>
#include "test_func.h"


int main() {
    exit(calculate("tests/test5.txt") != -10);
}
