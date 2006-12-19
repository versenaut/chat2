/*
 * Header declaring the channel operation commands.
*/

extern int	cmd_join(Channel *channel, User *speaker, const char *text);
extern int	cmd_leave(Channel *channel, User *speaker, const char *text);
