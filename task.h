/* task.h
 * Modules which need to be run from the main loop.
 */

#ifndef __TASK_H
#define __TASK_H

/* A wchan (wait channel) is something a task is waiting for, before it
 * can be run again. It is either a time (in millis), or something else.
 *
 *  RUN     0               Task is runnable now.
 *  TIME    type == 0       Task is runnable at the given time.
 *  IRQ     type == 0x8000  Reserved for hardware IRQs.
 *  SWI     type == 0x8001  Task is waiting for a call to swi().
 *  STOP    -1              Task is stopped.
 */

/* XXX This ought to be a union, but g++ won't play, so just use bitops
 * instead. */
typedef int32_t wchan;

#define WCHAN_TYPE(w)   ((w) & 0x80000000 ? ((w) >> 16) & 0xffff : 0x0000)
#define WCHAN_VALUE(w)  (w & 0xffff)

#define __TASK_TYP(t, v) (((int32_t)t) << 16 | (v))

#define TASK_TYP_TIME   0x0000
#define TASK_RUN        (0L)
#define TASK_TIME(n, x) ((n) + (x))
#define TASK_DELAY(x)   ((int32_t)millis() + (x))
#define TASK_TYP_IRQ    0x8000
#define TASK_IRQ(x)     __TASK_TYP(TASK_TYP_IRQ, (x))
#define TASK_TYP_SWI    0x8001
#define TASK_SWI(x)     __TASK_TYP(TASK_TYP_SWI, (x))
#define TASK_TYP_STOP   0xffff
#define TASK_STOP       (-1L)

typedef struct task {
    const char  *name;
    wchan       when;

    wchan       (*setup)    (void);
    wchan       (*run)      (wchan why);
    wchan       (*reset)    (void);
} task;

extern task *all_tasks[];

/* We currently have 8 swi lines. */
#define SWI_NTX     (1<<0)
#define SWI_GPS     (1<<1)
#define SWI_TEMP    (1<<2)

void    swi     (byte i);

#endif
