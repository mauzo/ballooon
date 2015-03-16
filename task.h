/* task.h
 * Modules which need to be run from the main loop.
 */

#ifndef __TASK_H
#define __TASK_H

struct task {
    const char  *name;
    boolean     active;
    long        when;

    void        (*setup)(void);
    void        (*run)(void);
    void        (*reset)(void);
};

extern task *all_tasks[];

#endif
