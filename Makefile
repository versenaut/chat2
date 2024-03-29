#
# Simple Makefile for building the chat v2 server and client.
#

VERSE=../verse

CFLAGS=-I$(VERSE) -Wall -g
LDFLAGS=-L$(VERSE)
LDLIBS=-lverse

ALL=chatserv client

ALL:		$(ALL)

# -------------------------------------------------------------

chatserv:	chatserv.o channel.o cmd_chanop.o cmd_nick.o cmd_version.o command.o nodedb.o qsarr.o user.o user_verse.o

chatserv.o:	chatserv.c

channel.o:	channel.c channel.h

cmd_chanop.o:	cmd_chanop.c cmd_chanop.h

cmd_nick.o:	cmd_nick.c cmd_nick.h

cmd_version.o:	cmd_version.c cmd_version.h chat2.h

command.o:	command.c command.h

nodedb.o:	nodedb.c nodedb.h

qsarr.o:	qsarr.c qsarr.h

user.o:		user.c user.h

user_verse.o:	user_verse.c user_verse.h user.h

# -------------------------------------------------------------

client:		client.c

# -------------------------------------------------------------

clean:
	rm -f $(ALL) *.o
