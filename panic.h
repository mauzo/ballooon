/* panic.h
 * Handle can't-happen problems.
 */

#ifndef __PANIC_H
#define __PANIC_H

extern jmp_buf  panic_jb;
extern void     (*panic_handler)(void);

void    panic           (const char *msg, ...);
void    panic_in_loop   (void);
void    panic_in_setup  (void);

#endif
