CC = gcc

CFLAGS +=  -Wall -O3  -m32 -DNDEBUG -fno-strict-aliasing
CPPFLLAGS +=
LDFLAGS +=  -m32 -lm -static
COUNT = NONE
VERSION = -5-0


CSRCS =  tts.c memutil.c clause.c assignment.c proposition.c list.c dimacs.c  \
	subsettree.c hashtable.c set.c intset.c ternarytree.c permute.c cputime.c \
	options.c

VISUALSTUDIOSRCS = tts-vs.sln tts-vs.vcproj

ALLSRCS = $(CSRCS) proof.c $(VISUALSTUDIOSRCS)

SCRIPTS = run-tts.sh


TSRCS = $(CSRCS)
THS =  $(TSRCS:%.c=%.h)
TOBJS =  $(TSRCS:%.c=%.o)

THSRCS = $(TSRCS)
THHS =  $(THSRCS:%.c=%.h)
THOBJS =  $(THSRCS:%.c=th%.o)

PSRCS = $(TSRCS) proof.c cbigopen.c
PHS = $(PSRCS:%.c=%.h)
POBJS = $(PSRCS:%.c=prf%.o)

ALLHS =  $(ALLSRCS:%.c=%.h)

main: tts$(VERSION)

all: tts$(VERSION) ttsp$(VERSION)

checkprt: $(CHECKOBJS)
	$(CC) -o checkprt $(CFLAGS) $(CHECKOBJS) $(LDFLAGS)

makentorder: makentorder.o
	$(CC) -o makentorder $(CFLAGS) makentorder.o $(LDFLAGS)

tts$(VERSION): $(TOBJS)
	$(CC) -o tts$(VERSION) $(CFLAGS) $(TOBJS) $(LDFLAGS)

ttsp$(VERSION): $(POBJS)
	$(CC) -o ttsp$(VERSION) $(CFLAGS) $(POBJS) $(LDFLAGS)

ttsth$(VERSION): $(THOBJS)
	$(CC) -DTHREADS -o ttsth$(VERSION) $(CFLAGS) $(THOBJS) $(LDFLAGS)

tts$(VERSION).tar: $(ALLSRCS) $(ALLHS) Makefile $(SCRIPTS)
	tar -cvf tts$(VERSION).tar $(ALLSRCS) $(ALLHS) Makefile $(SCRIPTS)

tts$(VERSION).tgz: tts$(VERSION).tar
	gzip < $< > $@


clean:
	rm -f ttsp$(VERSION) tts$(VERSION) tts$(VERSION).tar: $(TOBJS) $(POBJS) $(THOBJS) $(CHECKOBJS) core a.out

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

prf%.o:%.c
	$(CC) -DPROOF -c -o $@ $(CFLAGS) $(CPPFLAGS) $<

th%.o:%.c
	$(CC) -DTHREADS -c -o $@ $(CFLAGS) $(CPPFLAGS) $<
