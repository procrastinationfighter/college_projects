// File downloaded from Moodle: https://moodle.mimuw.edu.pl/course/view.php?id=561
#ifndef _ERR_
#define _ERR_

/* Writes and error and terminates program. */
extern void syserr(const char *fmt, ...);

/* wypisuje informacje o błędzie i kończy działanie */
extern void fatal(const char *fmt, ...);

#endif
