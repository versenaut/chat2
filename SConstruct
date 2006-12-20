#!/usr/bin/env python
#
# simple q&d sconstruct
#

import os
import sys

env = Environment ()
vLIB = ['libverse']
vLIBPATH = ['../verse']
vINC = ['../verse']
vDEF = []

if sys.platform=='win32':
	vLIB.append('ws2_32')
	vDEF.append('_WIN32')

server = ['chatserv.c',
		'channel.c',
		'cmd_nick.c',
		'cmd_chanop.c',
		'command.c',
		'nodedb.c',
		'qsarr.c',
		'user.c',
		'user_verse.c']

client = ['client.c']

env.Append(CPPPATH = vINC)
env.Append(LIBS = vLIB)
env.Append(LIBPATH = vLIBPATH)
env.Append(CPPDEFINES = vDEF)

env.Program(target = 'chatserv', source = server)
env.Program(target = 'client', source = client)