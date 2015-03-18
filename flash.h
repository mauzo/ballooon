/* flash.h
 * Unify the flash and ram address spaces.
 *
 * We have 32k of flash and less than that of ram, so the whole lot can
 * be addressed with a single 16-bit pointer if we use the top bit to
 * indicate that an address is in flash. (GCC has a __memx address space
 * that works similarly and would be more convenient, but it uses 24-bit
 * pointers to allow for multiple 64k blocks of flash.)
 */

#ifndef __FLASH_H
#define __FLASH_H

/* This is &f, where f is in flash */
#define pF(f)   ((typeof(f)*)(((uintptr_t)&(f)) | 0x8000))

/* This returns the actual address in f */
#define aF(f)   ((typeof(*(f))*)((uintptr_t)(f) & 0x7fff))

/* Is this address in flash? */
#define isF(f)  ((uintptr_t)(f) & 0x8000)

/* This is *f, where f might point into flash. This doesn't work in C++,
 * because it doesn't support the __flash memory space. */
#ifndef __cplusplus
#  define dF(f) ({ \
    const __flash typeof(*(f)) *__f; \
    (isF(f) ? (__f = aF(f), *__f) : *(f)); \
})
#endif

/* Find the address of a member of s, maintaining the top bit (&s->m) */
#define mF(s, m) ((s) + offsetof(typeof(*(s)), m))

/* Put a constant string into flash and return its address. This is NOT
 * COMPATIBLE with the F() macro provided by the Arduino headers. Use
 * PROGMEM rather than __flash so these can be created in C++. */
#define sF(s) ({ \
    static const char __sF[sizeof(s)] PROGMEM = (s); \
    (const char *)pF(__sF); \
})

/* String copying functions which work on pF source addresses. */
#define strlcpyF(d, s, n) \
    (isF(s) ? strlcpy_P(d, aF(s), n) : strlcpy(d, s, n))
#define memcpyF(d, s, n) \
    (isF(s) ? memcpy_P(d, aF(s), n) : memcpy(d, s, n))

#endif
