/* panic.cpp
 * Handle can't-happen situations with longjmp.
 */

#include "ballooon.h"

jmp_buf panic_jb;
void    (*panic_handler)(void);

void
panic (const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    pad_vform(msg, ap);
    va_end(ap);

    warnx(WPANIC, sF("\r\nPANIC: "));
    warn(WPANIC, pad);

    if (panic_handler)
        panic_handler();
    else
        /* nothing else we can do... */
        while (1) ;
}

void
panic_in_loop (void)
{
    task    **t;

    for (t = all_tasks; *t; t++)
        (*t)->reset();

    longjmp(panic_jb, 0);
}

void
panic_in_setup (void)
{
    warn(WPANIC, sF("Panic in setup, cannot continue!"));
    while (1) ;
}

