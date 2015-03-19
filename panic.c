/* panic.cpp
 * Handle can't-happen situations with longjmp.
 */

#include "ballooon.h"

jmp_buf panic_jb;

static const char *panic_msg;

void
panic (const char *msg)
{
    panic_msg = msg;
    longjmp(panic_jb, 1);
}

void
panic_catch (void)
{
    task    **t;

    warnx(WPANIC, sF("\r\nPANIC: "));
    warn(WPANIC, panic_msg);

    for (t = all_tasks; *t; t++)
        (*t)->reset();
}
