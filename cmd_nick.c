/*
 * A simple command to implement nickname-setting.
*/

#include "channel.h"
#include "user.h"

#include "cmd_nick.h"

#if defined _WIN32
#define	snprintf	_snprintf
#endif

int cmd_nick(Channel *channel, User *speaker, const char *text)
{
	int	i;
	Channel	*ch;
	char	buf[256];

	if(!user_name_is_valid(text))
		return 0;

	if(user_lookup(text) != NULL)		/* Refuse if nick is in use. */
		return 0;

	/* Before doing the change, go through all channels and notify. */
	snprintf(buf, sizeof buf, "%s is now known as %s\n", user_get_name(speaker), text);
	for(i = 0; (ch = channel_index(i)) != NULL; i++)
	{
		if(channel_user_is_member(ch, speaker))
			channel_hear(ch, NULL, buf);
	}
	user_set_name(speaker, text);
	return 1;
}
