#
#
#

CFLAGS=-I../verse -g
LDFLAGS=-L../verse
LDLIBS=-lverse

ALL=chatserv client

ALL:		$(ALL)

# -------------------------------------------------------------

chatserv:	chatserv.o channel.o cmd_chanop.o cmd_nick.o command.o nodedb.o qsarr.o user.o user-verse.o

chatserv.o:	chatserv.c

channel.o:	channel.c channel.h

cmd_chanop.o:	cmd_chanop.c cmd_chanop.h

cmd_nick.o:	cmd_nick.c cmd_nick.h

command.o:	command.c command.h

nodedb.o:	nodedb.c nodedb.h

qsarr.o:	qsarr.c qsarr.h

user.o:		user.c user.h

user-verse.o:	user-verse.c user-verse.h user.h

# -------------------------------------------------------------

client:		client.c

# -------------------------------------------------------------

clean:
	rm -f $(ALL) *.o
