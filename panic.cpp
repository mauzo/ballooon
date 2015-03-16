/* panic.cpp
 * Handle can't-happen situations with longjmp.
 */

#include <Arduino.h>

#include <stdarg.h>

#include "task.h"

#include "builtins.h"
#include "debug.h"
#include "panic.h"

jmp_buf     panic_jb;

static char panic_msg[255];

void
panic (const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(panic_msg, sizeof panic_msg, fmt, ap);
    va_end(ap);

    longjmp(panic_jb, 1);
}

void
panic_catch (void)
{
    task    **t;

    warn("");
    warn("PANIC: %s", panic_msg);
    warn("");

    for (t = all_tasks; *t; t++)
        (*t)->reset();
}
