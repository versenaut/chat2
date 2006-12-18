/*
 * 
*/

#include "user.h"

typedef struct Channel	Channel;

extern Channel *	channel_new(const char *name);
extern Channel *	channel_lookup(const char *name);
extern int		channel_user_add(Channel *channel, User *user);
extern int		channel_user_remove(Channel *channel, User *user);
