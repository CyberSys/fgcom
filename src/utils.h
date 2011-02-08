
#ifndef __UTILS_H__
#define __UTILS_H__

/* Initialize the file parser. */
int parser_init(const char *filename);

/* Exits parser. */
void parser_exit(void);

int parser_get_next_value(double *value);

#endif
