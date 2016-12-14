//#include "xtimer/implementation.h"
#include "xtimer.h"
uint32_t _xtimer_lltimer_now(void)
{
#if XTIMER_SHIFT
    //return XTIMER_TICKS_TO_USEC((uint32_t)timer_read(XTIMER_DEV));
    return (uint32_t)timer_read(0) >> -0;
#else
    //return timer_read(XTIMER_DEV);
    return timer_read(TIMER_DEV(0));
#endif
}

uint32_t _xtimer_lltimer_mask(uint32_t XDATA val)
{
    //return val & ~XTIMER_MASK_SHIFTED;
    return val & ~(0 << -0);
}

uint32_t xtimer_now(void)
{
#if XTIMER_MASK
    uint32_t latched_high_cnt, now;

    /* _high_cnt can change at any time, so check the value before
     * and after reading the low-level timer. If it hasn't changed,
     * then it can be safely applied to the timer count. */

    do {
        latched_high_cnt = _xtimer_high_cnt;
        now = _xtimer_lltimer_now();
    } while (_xtimer_high_cnt != latched_high_cnt);

    return latched_high_cnt | now;
#else
    return _xtimer_lltimer_now();
#endif
}

void xtimer_spin(uint32_t XDATA offset) {
    uint32_t start = _xtimer_lltimer_now();
#if XTIMER_MASK
    offset = _xtimer_lltimer_mask(offset);
    while (_xtimer_lltimer_mask(_xtimer_lltimer_now() - start) < offset);
#else
    while ((_xtimer_lltimer_now() - start) < offset);
#endif
}

void xtimer_usleep(uint32_t XDATA microseconds)
{
    _xtimer_sleep(microseconds, 0);
}

void xtimer_nanosleep(uint32_t XDATA nanoseconds)
{
    _xtimer_sleep(nanoseconds / USEC_IN_NS, 0);
}

