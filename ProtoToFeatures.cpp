#include "DenseTrack.h"
#include "Initialize.h"
#include "Descriptors.h"
#include "Util.h"
#include "ParserHelpers.h"
#include "dump.pb.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>

// Read everything in proto
// Training and test splits
// Build codebooks
// Write features