//
// Created by Yoav on 3/16/2025.
//

#include "pid.h"

static pid_t next_pid = 1;

// todo implement smart pid allocation
pid_t pid_alloc() {
    return next_pid++;
}

void free_pid(pid_t pid) {

}