/* task.h
 * Modules which need to be run from the main loop.
 */

#ifndef __TASK_H
#define __TASK_H

/* A wchan (wait channel) is something a task is waiting for, before it
 * can be run again. It is either a time (in millis), or something else.
 *
 *  RUN     time == 0       Task is runnable now.
 *  TIME    time > 0        Task is runnable at .time.
 *  IRQ     type == 0x8000  Reserved for hardware IRQs.
 *  SWI     type == 0x8001  Task is waiting for a call to swi().
 *  STOP    time == -1      Task is stopped.
 */

typedef union {
    int32_t         time;
    struct {
        uint16_t    type;
        uint16_t    value;
    };
} wchan;

#define TASK_RUN        ((wchan){ .time = 0L })
#define TASK_TIME(n, x) ((wchan){ .time = (n).time + (x) })
#define TASK_DELAY(x)   ((wchan){ .time = (long)millis() + (x) })
#define TASK_TYP_IRQ    0x8000
#define TASK_IRQ(x)     ((wchan){ .type = TASK_TYP_IRQ, .value = (x) })
#define TASK_TYP_SWI    0x8001
#define TASK_SWI(x)     ((wchan){ .type = TASK_TYP_SWI, .value = (x) })
#define TASK_TYP_STOP   0xffff
#define TASK_STOP       ((wchan){ .time = -1L })

typedef struct task {
    const char  *name;
    wchan       when;

    wchan       (*setup)    (void);
    wchan       (*run)      (wchan why);
    void        (*reset)    (void);
} task;

extern task *all_tasks[];

/* We currently have 8 swi lines. */
#define SWI_NTX     0
#define SWI_GPS     1

void    swi     (byte i);

#endif
