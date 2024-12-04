//
// Created by Yoav on 11/26/2024.
//

#include <assert.h>
#include "interupts_handler.h"
#include "../drivers/screen.h"

//currently supports only CPU exceptions
void isr_handler(registers_t *regs) {
    assert(regs->int_no < CPU_EXCEPTIONS);
    put_char('\n'); // New line for better visibility
    put_string("Exception: ");
    put_string(exception_messages[regs->int_no]);
    put_char('\n');
}
