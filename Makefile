.PHONY: all clean test

ROTO_SOURCES=\
	tonewheel_osc.cpp

ROTO_TEST_SOURCES=\
	roto_test.c \
	tonewheel_osc_test.c

ROTO_OBJS=$(subst .cpp,.o,$(ROTO_SOURCES))
ROTO_TEST_OBJS=$(subst .c,.o,$(ROTO_TEST_SOURCES))

CFLAGS=-DROTO_TEST
AO_CFLAGS=`pkg-config --cflags ao`
AO_LIBS=`pkg-config --libs ao`

.c.o:
	$(CC) $(CFLAGS) -c -g -o $@ $<

roto.test: $(ROTO_OBJS) $(ROTO_TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(AO_LIBS) -g -o $@ $(ROTO_OBJS) $(ROTO_TEST_OBJS)

test: roto.test
	./roto.test

clean:
	rm -f $(ROTO_OBJS) $(ROTO_TEST_OBJS) roto.test
