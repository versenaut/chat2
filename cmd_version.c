/*
 * 
*/

#include "chat2.h"

#include "channel.h"
#include "user.h"

#include "cmd_version.h"

int cmd_version(Channel *channel, User *speaker, const char *text)
{
	char	buf[256];

	snprintf(buf, sizeof buf, "/version %s", VERSION);
	user_hear(speaker, "", NULL, buf);

	return 1;
}
