#ifndef COUNTS_H
#define COUNTS_H

#ifdef COUNT

extern void doCount(int a);
extern void doUnCount(int a);
extern void initCounts (int n);
extern char *getCountString(int a);
extern char *getMaxCountString();

#define initCounts(a) doInitCounts(a)
#define count(a) doCount(a);
#define unCount(a) doUnCount(a);
#define countString(a) getCountString(a)
#define maxCountString() getMaxCountString()
#else

#define initCounts(a)
#define count(a)
#define unCount(a)
#define countString(a) "-1 (not recorded)"
#define maxCountString() "-1 (not recorded)"


#endif

#endif
