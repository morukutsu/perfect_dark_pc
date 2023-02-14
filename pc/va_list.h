#ifndef _VA_LIST_H_
#define _VA_LIST_H_

/*
    Before including headers such as <stdio.h>, it seems va_list needs to be declared.
    Include this file before the system header if the error happens.
    TODO: linux, win x msvc support
*/
typedef char* __gnuc_va_list;

#endif