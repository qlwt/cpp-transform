#include "tr/args.hpp"
#include "tr/bmp.hpp"

#pragma once

namespace tr {

void pattern_fill_precise(tr::img::BMP *target, double time,  tr::ArgConfig *config);
void pattern_fill_batch(tr::img::BMP *target, double time,  tr::ArgConfig *config);

}
