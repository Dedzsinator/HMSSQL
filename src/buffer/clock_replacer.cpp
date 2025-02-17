//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// clock_replacer.cpp
//
// Identification: src/buffer/clock_replacer.cpp
//
//
//===----------------------------------------------------------------------===//

#include "../include/buffer/clock_replacer.h"

namespace hmssql {

ClockReplacer::ClockReplacer(size_t num_pages) {}

ClockReplacer::~ClockReplacer() = default;

auto ClockReplacer::Victim(frame_id_t *frame_id) -> bool { return false; }

void ClockReplacer::Pin(frame_id_t frame_id) {}

void ClockReplacer::Unpin(frame_id_t frame_id) {}

auto ClockReplacer::Size() -> size_t { return 0; }

}  // namespace hmssql
