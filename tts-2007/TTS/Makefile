CC = gcc
CFLAGS += -m32 -Wall -O3 -DNDEBUG -fno-strict-aliasing
CPPFLAGS +=
LDFLAGS += -m32 -lm -static
COUNT = NONE
VERSION = -4-0


CSRCS =  memutil.c clause.c assignment.c proposition.c list.c dimacs.c \
	subsettree.c set.c ternarytree.c counts.c permute.c cputime.c \
	options.c bag.c assoclist.c hashtable.c

TSRCS = $(CSRCS) tts.c
THS =  $(TSRCS:%.c=%.h)
TOBJS =  $(TSRCS:%.c=%.o)



all: tts$(VERSION)

tts$(VERSION): $(TOBJS)
	$(CC) -o tts$(VERSION) $(CFLAGS) $(TOBJS) $(LDFLAGS)

tts$(VERSION).tar: $(TSRCS) $(HS) Makefile
	tar -cvf tts$(VERSION).tar $(TSRCS) $(THS) Makefile

tts$(VERSION).tgz: tts$(VERSION).tar
	gzip < $< > $@

clean:
	rm -f ttsp tts tts$(VERSION).tar: $(TOBJS) $(POBJS) core a.out

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

prf%.o:%.c
	$(CC) -DPROOF -c -o $@ $(CFLAGS) $(CPPFLAGS) $<
