/* panic.h
 * Handle can't-happen problems.
 */

#ifndef __PANIC_H
#define __PANIC_H

extern jmp_buf  panic_jb;

void        panic           (Fstr msg);
void        panic_catch     (void);

/* This has to be a macro, because setjmp needs to be called directly
 * from loop() or it doesn't work. The do..while(0) is a standard trick
 * to keep the contents of the macro self-contained.
 *
 * Calling this macro requires <setjmp.h>.
 */
#define PANIC_CATCH \
    do { \
        if (setjmp(panic_jb)) \
            panic_catch(); \
    } while (0)

#endif
