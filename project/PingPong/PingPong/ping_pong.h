/*
 * Project: Ping pong game with a distributed embedded control system
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <avr/io.h>

#define F_CPU 4915200  /* Clock speed */

/* Utility macros */
#define __NOP() {__asm__ __volatile__ ("nop");}
#define NUMELTS(array) (sizeof(array)/sizeof(array[0]))
