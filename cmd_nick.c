/*
 * A simple command to implement nickname-setting.
*/

#include "channel.h"
#include "user.h"

#include "cmd_nick.h"

int cmd_nick(Channel *channel, User *speaker, const char *text)
{
	user_set_name(speaker, text);
	return 1;
}
