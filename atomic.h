/* atomic.h
 * Atomic operations.
 *
 * Since the only true atomic write operations on this chip are sbi/cbi,
 * which only work on IO registers, we have to make everything else
 * atomic with critical sections.
 */

#ifndef __ATOMIC_H
#define __ATOMIC_H

/* Use like this:
 *  CRIT_START {
 *      ...
 *  } CRIT_END;
 */

#define CRIT_START ({ \
    byte __sreg = SREG; \
    cli();
#define CRIT_END \
    SREG = __sreg; \
})

#endif
