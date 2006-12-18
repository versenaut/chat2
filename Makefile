#
#
#

CFLAGS=-I../verse -g
LDFLAGS=-L../verse
LDLIBS=-lverse

ALL=chatserv client

ALL:		$(ALL)

# -------------------------------------------------------------

chatserv:	chatserv.o channel.o qsarr.o user.o user-verse.o

chatserv.o:	chatserv.c

channel.o:	channel.c channel.h

qsarr.o:	qsarr.c qsarr.h

user.o:		user.c user.h

user-verse.o:	user-verse.c user-verse.h user.h

# -------------------------------------------------------------

client:		client.c

# -------------------------------------------------------------

clean:
	rm -f $(ALL) *.o
