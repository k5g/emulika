SUBDIRS = misc cpu

bin_PROGRAMS = emulika

emulika_SOURCES = main.c \
	clock.c clock.h \
	environ.c environ.h \
	sms.c sms.h \
	tms9918a.c tms9918a.h \
	ym2413.c ym2413.h \
	icon.c icon.h \
	input.c input.h \
	emuconfig.c emuconfig.h \
	sn76489.c sn76489.h \
	screenshot.c screenshot.h \
	emul.h \
	rom.c rom.h \
	snapshot.c snapshot.h \
	video.c video.h \
	seeprom.c seeprom.h

emulika_LDADD = misc/libmisc.a cpu/libcpu.a
