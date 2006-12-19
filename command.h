/*
 * Command module header. Supports storing commands, and looking them up.
*/

typedef struct Command	Command;

extern void		command_init(void);

extern Command *	command_new(const char *name, const char *label,
				    const char *args, const char *description,
				    int (*handler)(Channel *channel, User *speaker, const char *text));

extern Command *	command_lookup(const char *name);

extern int		command_run(Command *cmd, Channel *channel, User *speaker, const char *text);

extern void		command_send_list(User *user);
