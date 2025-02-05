#pragma once

#include <sys/stat.h>
#include "hal.h"
#include "uart.h"
int _fstat(int fd, struct stat *st);
void *_sbrk(int incr);
int _open(const char *path);
int _close(int fd);
int _isatty(int fd);
int _lseek(int fd, int ptr, int dir);
void _exit(int status);
void _kill(int pid, int sig);
int _getpid(void);
int _write(int fd, char *ptr, int len);
int _read(int fd, char *ptr, int len);
int _link(const char *a, const char *b);
int _unlink(const char *a);
int _stat(const char *path, struct stat *st);
int mkdir(const char *path, mode_t mode);
void _init(void);

