.PHONY: all clean test fmt

SOURCES = \
	roto.ino \
	roto_test.c \
	tonewheel_osc.cpp \
	tonewheel_osc.h \
	tonewheel_osc_audio.h \
	tonewheel_osc_test.c

ROTO_TEST_OBJS = \
	roto_test.o \
	tonewheel_osc.o \
	tonewheel_osc_test.o

CFLAGS=-DROTO_TEST
AO_CFLAGS=`pkg-config --cflags ao`
AO_LIBS=`pkg-config --libs ao`

.c.o:
	$(CXX) $(CFLAGS) -c -g -o $@ $<

.cpp.o:
	$(CXX) $(CFLAGS) -c -g -o $@ $<

roto.test: $(ROTO_TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(AO_LIBS) -g -o $@ $(ROTO_TEST_OBJS)

test: roto.test
	./roto.test

fmt:
	clang-format -i $(SOURCES)

clean:
	rm -f $(ROTO_TEST_OBJS) roto.test
