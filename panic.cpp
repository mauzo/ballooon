/* panic.cpp
 * Handle can't-happen situations with longjmp.
 */

#include "ballooon.h"

jmp_buf         panic_jb;

static fstr     panic_msg;

EXT_C void
panic (fstr msg)
{
    panic_msg = msg;
    longjmp(panic_jb, 1);
}

void
panic_catch (void)
{
    task    **t;

    warnx("\r\nPANIC: ");
    warn(panic_msg);

    for (t = all_tasks; *t; t++)
        (*t)->reset();
}
