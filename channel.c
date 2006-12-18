/*
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "channel.h"
#include "qsarr.h"

struct Channel {
	char	name[64];
};

static struct {
	QSArr	*channels;
} ChannelInfo;

/*------------------------------------------------------------------------------------------------ */

static int cmp_channel_sort(const void **a, const void **b)
{
	const Channel	*ca = *(Channel **) a, *cb = *(Channel **) b;

	printf("comparing '%s' and '%s'\n", ca->name, cb->name);

	return strcmp(ca->name, cb->name);
}

static int cmp_channel_key(const void *a, const void *key)
{
	return strcmp(((Channel *) a)->name, key);
}

void channel_init(void)
{
	ChannelInfo.channels = qsarr_new(cmp_channel_sort, cmp_channel_key);
}

Channel * channel_new(const char *name)
{
	Channel	*ch;

	if((ch = malloc(sizeof *ch)) != NULL)
	{
		strcpy(ch->name, name);

		qsarr_insert(ChannelInfo.channels, ch);
	}
	return ch;
}

Channel * channel_lookup(const char *name)
{
	return qsarr_lookup(ChannelInfo.channels, name);
}

int channel_user_add(Channel *channel, User *user)
{
}

int channel_user_remove(Channel *channel, User *user)
{
}

void channel_hear(Channel *channel, const char *sender, const char *text)
{
	if(channel == NULL || text == NULL)
		return;
}
