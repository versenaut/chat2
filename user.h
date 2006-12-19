/*
 * 
*/

#if !defined USER_H
#define	USER_H

typedef struct User	User;

/* This struct is public, for inheritance. */
struct User {
	char	name[64];
	void	(*hear)(User *, const char *channel, const char *speaker, const char *text);
};

extern void		user_ctor(User *user, const char *name);

extern const char *	user_get_name(const User *user);

extern int		user_set_name(User *user, const char *name);

extern void		user_destroy(User *user);

extern size_t		user_count(void);

extern User *		user_index(unsigned int index);

extern void		user_hear(const User *user, const char *channel, const char *speaker, const char *text);

#endif		/* USER_H */
