//
// Created by Yoav on 1/8/2025.
//

#ifndef MYKERNELPROJECT_SHELL_H
#define MYKERNELPROJECT_SHELL_H
#define MAX_INPUT_LENGTH 128
void shell();
void shell_welcome_message();
void shell_prompt();
void execute_command(const char *input);
#endif //MYKERNELPROJECT_SHELL_H
