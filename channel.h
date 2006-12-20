/*
 * 
*/

#include "user.h"

typedef struct Channel	Channel;

extern Channel *	channel_default(void);

extern Channel *	channel_new(const char *name);
extern void		channel_destroy(Channel *channel);

extern int		channel_is_default(const Channel *channel);
extern const char *	channel_get_name(const Channel *channel);

extern Channel *	channel_index(int index);
extern Channel *	channel_lookup(const char *name);
extern int		channel_user_add(Channel *channel, User *user);
extern int		channel_user_is_member(const Channel *channel, const User *user);
extern int		channel_user_remove(Channel *channel, User *user);
extern size_t		channel_size(const Channel *channel);

extern void		channel_hear(Channel *channel, const User *speaker, const char *text);
