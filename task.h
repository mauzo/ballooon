/* task.h
 * Modules which need to be run from the main loop.
 */

#ifndef __TASK_H
#define __TASK_H

#define TASK_INACTIVE   (-1L)
#define TASK_START      (0L)

struct task {
    const char  *name;
    long        when;

    void        (*setup)(void);
    void        (*run)(long now);
    void        (*reset)(void);
};

extern task *all_tasks[];

#endif
