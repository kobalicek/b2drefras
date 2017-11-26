#ifndef _PERFORMANCE_H
#define _PERFORMANCE_H

#include <stddef.h>
#include <stdint.h>

class Performance {
public:
  inline Performance() noexcept { reset(); }

  inline void reset() noexcept {
    tick = 0U;
    best = 0xFFFFFFFFU;
  }

  inline uint32_t start() noexcept { return (tick = getTickCount()); }
  inline uint32_t diff() const noexcept { return getTickCount() - tick; }

  inline uint32_t end() noexcept {
    tick = diff();
    if (best > tick)
      best = tick;
    return tick;
  }

  static uint32_t getTickCount() noexcept;

  uint32_t tick;
  uint32_t best;
};

#endif // _PERFORMANCE_H
