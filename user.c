/*
 * 
*/

#include <stdlib.h>
#include <string.h>

#include "chat2.h"

#include "user.h"
#include "qsarr.h"

/*------------------------------------------------------------------------------------------------ */

#define	NAME_SERVER	"<server>"

static struct {
	QSArr	*users;
} UserInfo;

/*------------------------------------------------------------------------------------------------ */

static int cmp_user_sort(const void **e1, const void **e2)
{
	const User	*ua = *(User **) e1, *ub = *(User **) e2;

	return strcmp(ua->name, ub->name);
}

static int cmp_user_key(const void *u, const void *key)
{
	return strcmp(((User *) u)->name, key);
}

void user_init(void)
{
	UserInfo.users = qsarr_new(cmp_user_sort, cmp_user_key);
}

int user_name_is_valid(const char *name)
{
	if(name == NULL)
		return 0;
	return strncmp(name, NAME_SERVER, strlen(NAME_SERVER)) != 0;
}

void user_ctor(User *user, const char *name)
{
	if(user == NULL || name == NULL)
		return;
	snprintf(user->name, sizeof user->name, "%s", name);
	qsarr_insert(UserInfo.users, user);
}

const char * user_get_name(const User *user)
{
	if(user == NULL)
		return NULL;
	return user->name;
}

int user_set_name(User *user, const char *name)
{
	if(user == NULL || name == NULL)
		return;
	snprintf(user->name, sizeof user->name, "%s", name);
}

size_t user_count(void)
{
	return qsarr_size(UserInfo.users);
}

User * user_index(unsigned int index)
{
	return qsarr_index(UserInfo.users, index);
}

User * user_lookup(const char *name)
{
	if(name == NULL)
		return NULL;
	return qsarr_lookup(UserInfo.users, name);
}

void user_hear(const User *user, const char *channel, const char *speaker, const char *text)
{
	if(user == NULL || channel == NULL || text == NULL)
		return;
	if(speaker == NULL)
		speaker = NAME_SERVER;
	user->hear((User *) user, channel, speaker, text);
}
