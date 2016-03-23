#include "ParserHelpers.h"
// Returns a random color
Scalar getRandomColor(void) {
  int r = rand() % 255;
    r = (r + 200) / 2;
    int g = rand() % 255;
    g = (g + 200) / 2;
    int b = rand() % 255;
    b = (b + 200) / 2;
    return Scalar(r,g,b);
} 