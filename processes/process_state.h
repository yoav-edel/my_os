//
// Created by Yoav on 3/16/2025.
//

#ifndef MYKERNEL_PROCESS_STATE_H
#define MYKERNEL_PROCESS_STATE_H
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} process_state_t;
#endif //MYKERNEL_PROCESS_STATE_H
