/*
 * Channel operations, i.e. join and leave channels.
*/

#include "channel.h"
#include "user.h"

#include "cmd_chanop.h"

#if defined _WIN32
#define	snprintf	_snprintf
#endif

int cmd_join(Channel *channel, User *speaker, const char *text)
{
	Channel	*ch;

	if((ch = channel_lookup(text)) == NULL)
		ch = channel_new(text);
	if(ch != NULL && !channel_is_default(ch))
		channel_user_add(ch, speaker);
	return ch != NULL;
}

int cmd_leave(Channel *channel, User *speaker, const char *text)
{
	Channel	*ch;

	if((ch = channel_lookup(text)) != NULL)
	{
		if(channel_is_default(ch))
			return 1;
		channel_user_remove(ch, speaker);
		if(channel_size(ch) == 0)
			channel_destroy(ch);
		return 1;
	}
	return 0;
}

int cmd_listchan(Channel *channel, User *speaker, const char *text)
{
	char	buf[128];
	int	i;
	Channel	*ch;

	for(i = 0; (ch = channel_index(i)) != NULL; i++)
	{
		if(channel_is_default(ch))
			continue;
		snprintf(buf, sizeof buf, "/listchan: %s\n", channel_get_name(ch));
		user_hear(speaker, "", NULL, buf);
	}
	return 1;
}

int cmd_who(Channel *channel, User *speaker, const char *text)
{
	char	buf[128];
	int	i;
	User	*u;

	for(i = 0; (u = channel_user_index(channel, i)) != NULL; i++)
	{
		snprintf(buf, sizeof buf, "/who %s\n", user_get_name(u));
		user_hear(speaker, channel_get_name(channel), NULL, buf);
	}
	return 1;
}
