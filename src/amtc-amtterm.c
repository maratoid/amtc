
static int recv_tty(void *cb_data, unsigned char *buf, int len)
{
//    struct redir *r = cb_data;

    return write(STDOUT_FILENO, buf, len);
}

static void state_tty(void *cb_data, enum redir_state old, enum redir_state new)
{
    struct redir *r = cb_data;

    if (r->verbose)
	fprintf(stderr, APPNAME ": %s -> %s (%s)\n",
		redir_state_name(old), redir_state_name(new),
		redir_state_desc(new));
    switch (new) {
    case REDIR_RUN_SOL:
	if (r->verbose)
	    fprintf(stderr,
		    "serial-over-lan redirection ok\n"
		    "connected now, use ^] to escape\n");
	break;
    case REDIR_ERROR:
	fprintf(stderr, APPNAME ": ERROR: %s\n", r->err);
	break;
    default:
	break;
    }
}

static int redir_loop(struct redir *r)
{
    unsigned char buf[BUFSIZE+1];
    struct timeval tv;
    int rc, i;
    fd_set set;

    for(;;) {
	if (r->state == REDIR_CLOSED ||
	    r->state == REDIR_ERROR)
	    break;

	FD_ZERO(&set);
	if (r->state == REDIR_RUN_SOL)
	    FD_SET(STDIN_FILENO,&set);
	FD_SET(r->sock,&set);
	tv.tv_sec  = HEARTBEAT_INTERVAL * 4 / 1000;
	tv.tv_usec = 0;
	switch (select(r->sock+1,&set,NULL,NULL,&tv)) {
	case -1:
	    perror("select");
	    return -1;
	case 0:
	    fprintf(stderr,"select: timeout\n");
	    return -1;
	}

	if (FD_ISSET(STDIN_FILENO,&set)) {
	    /* stdin has data */
	    rc = read(STDIN_FILENO,buf,BUFSIZE);
	    switch (rc) {
	    case -1:
		perror("read(stdin)");
		return -1;
	    case 0:
		fprintf(stderr,"EOF from stdin\n");
		return -1;
	    default:
		if (buf[0] == 0x1d) {
		    if (r->verbose)
			fprintf(stderr, "\n" APPNAME ": saw ^], exiting\n");
		    redir_sol_stop(r);
                }
                for (i = 0; i < rc; i++) {
                    /* meet BIOS expectations */
                    if (buf[i] == 0x0a)
                        buf[i] = 0x0d;
		}
		if (-1 == redir_sol_send(r, buf, rc))
		    return -1;
		break;
	    }
	}

	if (FD_ISSET(r->sock,&set)) {
	    if (-1 == redir_data(r))
		return -1;
	}
    }
    return 0;
}

/* ------------------------------------------------------------------ */

struct termios  saved_attributes;
int             saved_fl;

static void tty_save(void)
{
    fcntl(STDIN_FILENO,F_GETFL,&saved_fl);
    tcgetattr (STDIN_FILENO, &saved_attributes);
}

static void tty_noecho(void)
{
    struct termios tattr;

    memcpy(&tattr,&saved_attributes,sizeof(struct termios));
    tattr.c_lflag &= ~(ECHO);
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}

static void tty_raw(void)
{
    struct termios tattr;

    fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK);
    memcpy(&tattr,&saved_attributes,sizeof(struct termios));
    tattr.c_lflag &= ~(ISIG|ICANON|ECHO);
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = STDIN_FILENO;
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}

static void tty_restore(void)
{
    fcntl(STDIN_FILENO,F_SETFL,saved_fl);
    tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

/* ------------------------------------------------------------------ */

