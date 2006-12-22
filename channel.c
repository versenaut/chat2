/*
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat2.h"

#include "channel.h"
#include "qsarr.h"

struct Channel {
	char	name[64];
	char	topic[256];
	QSArr	*members;	
};

static struct {
	QSArr	*channels;
	Channel	*def;		/* Default channel, named "" and always available. */
} ChannelInfo = { NULL, NULL };

/*------------------------------------------------------------------------------------------------ */

static int cmp_channel_sort(const void **a, const void **b)
{
	const Channel	*ca = *(Channel **) a, *cb = *(Channel **) b;

	return strcmp(ca->name, cb->name);
}

static int cmp_channel_key(const void *a, const void *key)
{
	return strcmp(((Channel *) a)->name, key);
}

void channel_init(void)
{
	ChannelInfo.channels = qsarr_new(cmp_channel_sort, cmp_channel_key);

	ChannelInfo.def = channel_new("");
}

static int cmp_user_sort(const void **a, const void **b)
{
	const User	*ma = *(User **) a, *mb = *(User **) b;

	return ma < mb ? -1 : ma > mb;
}

static int cmp_user_key(const void *a, const void *key)
{
	return a < key ? -1 : a > key;
}

Channel * channel_default(void)
{
	return ChannelInfo.def;
}

Channel * channel_new(const char *name)
{
	Channel	*ch;

	if((ch = malloc(sizeof *ch)) != NULL)
	{
		snprintf(ch->name, sizeof ch->name, "%s", name);
		ch->topic[0] = '\0';
		ch->members = qsarr_new(cmp_user_sort, cmp_user_key);
		qsarr_insert(ChannelInfo.channels, ch);
	}
	return ch;
}

void channel_destroy(Channel *channel)
{
	if(channel == NULL)
		return;
	qsarr_remove(ChannelInfo.channels, channel);
	qsarr_destroy(channel->members);
	free(channel);
}

int channel_is_default(const Channel *channel)
{
	return channel != NULL && channel == ChannelInfo.def;
}

const char * channel_get_name(const Channel *channel)
{
	return channel != NULL ? channel->name : NULL;
}

void channel_set_topic(Channel *channel, const char *topic)
{
	if(channel != NULL && topic != NULL)
	{
		snprintf(channel->topic, sizeof channel->topic, "%s", topic);
		if(qsarr_size(channel->members) > 0)
		{
			char	buf[256];
			int	i;
			User	*u;

			snprintf(buf, sizeof buf, "/topic \"%s\"\n", channel->topic);
			for(i = 0; (u = channel_user_index(channel, i)) != NULL; i++)
				user_hear(u, channel->name, NULL, buf);
		}
	}
}

const char * channel_get_topic(const Channel *channel)
{
	return channel != NULL ? channel->topic : NULL;
}

Channel * channel_index(int index)
{
	return qsarr_index(ChannelInfo.channels, index);
}

Channel * channel_lookup(const char *name)
{
	return qsarr_lookup(ChannelInfo.channels, name);
}

int channel_user_add(Channel *channel, User *user)
{
	char	buf[256];

	if(channel == NULL || user == NULL)
		return 0;
	if(qsarr_lookup(channel->members, user) != NULL)
		return 0;
	qsarr_insert(channel->members, user);
	/* Announce the new user. */
	snprintf(buf, sizeof buf, "%s has joined channel \"%s\"\n", user_get_name(user), channel->name);
	channel_hear(channel, NULL, buf);
	if(channel->topic[0] != '\0')
	{
		snprintf(buf, sizeof buf, "/topic \"%s\"\n", channel->topic);
		user_hear(user, channel->name, NULL, buf);
	}
	return 1;
}

int channel_user_is_member(const Channel *channel, const User *user)
{
	if(channel == NULL || user == NULL)
		return 0;
	return qsarr_lookup(channel->members, user) != NULL;
}

User * channel_user_index(const Channel *channel, int index)
{
	if(channel == NULL)
		return NULL;
	return qsarr_index(channel->members, index);
}

int channel_user_remove(Channel *channel, User *user)
{
	char	buf[128];

	if(channel == NULL || user == NULL)
		return 0;
	qsarr_remove(channel->members, user);
	/* Announce the dropout. */
	snprintf(buf, sizeof buf, "%s has left channel \"%s\"\n", user_get_name(user), channel->name);
	channel_hear(channel, NULL, buf);

	if(channel_size(channel) == 0)	/* Did channel become empty? Then destroy it. */
	{
		channel_destroy(channel);
	}

	return 1;
}

size_t channel_size(const Channel *channel)
{
	return channel != NULL ? qsarr_size(channel->members) : 0;
}

void channel_hear(Channel *channel, const User *speaker, const char *text)
{
	const char	*spk;
	int		i;
	User		*u;

	if(channel == NULL || text == NULL)
		return;
	spk = user_get_name(speaker);	/* Does the right thing for a NULL User. */
	for(i = 0; (u = qsarr_index(channel->members, i)) != NULL; i++)
		user_hear(u, channel->name, spk, text);
}
