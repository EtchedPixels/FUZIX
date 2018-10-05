#ifndef __READLINE_H
#define __READLINE_H

/* Standard readline API */
extern char *readline(const char *__prompt);

/* Fuzix API's */
extern int rl_edit (int __fd, int __ofd, const char *__prompt, char *__input,
                    size_t __len);
extern void rl_hinit(char *__buffer, size_t __len);

#endif
