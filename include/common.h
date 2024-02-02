#pragma once

#include <cassert>

#define PORT 2333
const int TEST_CNT = 10;
const int BUFFER_SIZE = 1024;

#define ASSERT(expr, msg) assert((expr) && (msg));
