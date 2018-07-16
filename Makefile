.PHONY: all clean test fmt

SOURCES = \
	manual.cpp \
	manual.h \
	manual_test.c \
	preamp_audio.h \
	roto.ino \
	roto_test.c \
	tonewheel_osc.cpp \
	tonewheel_osc.h \
	tonewheel_osc_audio.h \
	tonewheel_osc_test.c \
	vibrato.cpp \
	vibrato.h \
	vibrato_audio.h \
	vibrato_test.c

ROTO_TEST_OBJS = \
	manual.o \
	manual_test.o \
	roto_test.o \
	tonewheel_osc.o \
	tonewheel_osc_test.o \
	vibrato.o \
	vibrato_test.o

CFLAGS=-DROTO_TEST

.c.o:
	$(CC) $(CFLAGS) -c -g -o $@ $<

.cpp.o:
	$(CXX) $(CFLAGS) -c -g -o $@ $<

roto.test: $(ROTO_TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -g -o $@ $(ROTO_TEST_OBJS)

test: roto.test
	./roto.test

fmt:
	clang-format -i $(SOURCES)

clean:
	rm -f $(ROTO_TEST_OBJS) roto.test
