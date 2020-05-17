#ifndef _COLORS_
#define _COLORS_

/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KBLK  "\x1B[30m"

#define FRED(x) string(KRED) + x + RST
#define FGRN(x) string(KGRN) + x + RST
#define FYEL(x) string(KYEL) + x + RST
#define FBLU(x) string(KBLU) + x + RST
#define FMAG(x) string(KMAG) + x + RST
#define FCYN(x) string(KCYN) + x + RST
#define FWHT(x) string(KWHT) + x + RST
#define FBLK(x) string(KBLK) + x + RST

#define BOLD(x) string("\x1B[1m") + x + RST
#define UNDL(x) string("\x1B[4m") + x + RST

#endif  /* _COLORS_ */