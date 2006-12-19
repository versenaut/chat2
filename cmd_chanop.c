/*
 * Channel operations, i.e. join and leave channels.
*/

#include "channel.h"
#include "user.h"

#include "cmd_chanop.h"

int cmd_join(Channel *channel, User *speaker, const char *text)
{
	Channel	*ch;

	if((ch = channel_lookup(text)) == NULL)
		ch = channel_new(text);
	if(ch != NULL)
		channel_user_add(ch, speaker);
	return ch != NULL;
}

int cmd_leave(Channel *channel, User *speaker, const char *text)
{
	Channel	*ch;

	if((ch = channel_lookup(text)) != NULL)
	{
		channel_user_remove(ch, speaker);
		if(channel_size(ch) == 0)
		{
			printf("destroying channel %s, it's empty\n", text);
			channel_destroy(ch);
		}
		return 1;
	}
	return 0;
}
