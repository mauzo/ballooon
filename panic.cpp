/* panic.cpp
 * Handle can't-happen situations with longjmp.
 */

#include "ballooon.h"

jmp_buf         panic_jb;

static Fstr     panic_msg;

void
panic (Fstr msg)
{
    panic_msg = msg;
    longjmp(panic_jb, 1);
}

void
panic_catch (void)
{
    task    **t;

    warnx(strFdup(F("\r\nPANIC: ")));
    warnx(strFdup(panic_msg));
    warnx(strFdup(F("\r\n")));

    for (t = all_tasks; *t; t++)
        (*t)->reset();
}
