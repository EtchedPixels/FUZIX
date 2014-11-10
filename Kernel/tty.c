#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

/*
 *	Minimal Terminal Interface
 *
 *	TODO:
 *	- VTIME timeout support
 *	- Blocking open
 *	- Hangup
 *	- Invoke device side helpers
 *	- Parity
 *	- Various misc minor flags
 *	- Better /dev/tty handling
 *	- BSD ^Z handling and tty sessions eventually
 *	- Flow control
 *
 *	Add a small echo buffer to each tty
 */

struct termios ttydata[NUM_DEV_TTY + 1];	/* ttydata[0] is not used */
uint16_t tty_pgrp[NUM_DEV_TTY + 1];

static bool stopflag[NUM_DEV_TTY + 1];	// Flag for ^S/^Q
static bool flshflag[NUM_DEV_TTY + 1];	// Flag for ^O
static bool deadflag[NUM_DEV_TTY + 1];  // True if hung up

int tty_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	uint16_t nread;
	unsigned char c;
	struct s_queue *q;
	struct termios *t;

	rawflag;
	flag;			// shut up compiler

	/* Minor == 0 means that it is the controlling tty of the process */
	if (!minor)
		minor = udata.u_ptab->p_tty;
	if (!udata.u_ptab->p_tty)
		udata.u_ptab->p_tty = minor;

	q = &ttyinq[minor];
	t = &ttydata[minor];
	nread = 0;
	while (nread < udata.u_count) {
		for (;;) {
		        if (deadflag[minor]) {
		                udata.u_error = ENXIO;
		                return -1;
                        }
			if (remq(q, &c)) {
				if (udata.u_sysio)
					*udata.u_base = c;
				else
					uputc(c, udata.u_base);
				break;
			}
			if (psleep_flags(q, flag))
				return -1;
		}

		++nread;

		/* return according to mode */
		if (!(t->c_lflag & ICANON)) {
			if (nread >= t->c_cc[VMIN])
				break;
		} else {
			if (nread == 1 && (c == t->c_cc[VEOF])) {
				/* ^D */
				nread = 0;
				break;
			}
			if (c == '\n')
				break;
		}

		++udata.u_base;
	}
	wakeup(&q->q_count);
	return nread;
}


int tty_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	struct termios *t;
	int towrite;
	uint8_t c;

	rawflag;
	flag;			// shut up compiler

	/* Minor == 0 means that it is the controlling tty of the process */
	if (!minor)
		minor = udata.u_ptab->p_tty;
	if (!udata.u_ptab->p_tty)
		udata.u_ptab->p_tty = minor;

	t = &ttydata[minor];

	towrite = udata.u_count;

	while (udata.u_count-- != 0) {
		for (;;) {	/* Wait on the ^S/^Q flag */
		        if (deadflag[minor]) {
		                udata.u_error = ENXIO;
		                return -1;
                        }
			if (!stopflag[minor])
				break;
			if (psleep_flags(&stopflag[minor], flag))
				return -1;
		}

		if (!flshflag[minor]) {
			if (udata.u_sysio)
				c = *udata.u_base;
			else
				c = ugetc(udata.u_base);

			if (t->c_oflag & OPOST) {
				if (c == '\n' && (t->c_oflag & ONLCR))
					tty_putc_wait(minor, '\r');
				if (c == '\r' && (t->c_oflag & OCRNL))
					c = '\n';
			}
			tty_putc_wait(minor, c);
		}
		++udata.u_base;
	}
	return towrite;
}

int tty_open(uint8_t minor, uint16_t flag)
{
	struct termios *t;

	/* FIXME : open/carrier logic is needed here, definitely before we
	   enable ptys */
	/* Minor == 0 means that it is the controlling tty of the process */
	/* FIXME: need to propogate this minor change back into the file handle
	   and inode somehow ??? */
	if (!minor)
		minor = udata.u_ptab->p_tty;
	if (minor < 1 || minor > NUM_DEV_TTY + 1) {
		udata.u_error = ENODEV;
		return (-1);
	}
	/* Hung up but not yet cleared of users */
	if (deadflag[minor]) {
	        udata.u_error = ENXIO;
	        return -1;
        }

	t = &ttydata[minor];

	/* If there is no controlling tty for the process, establish it */
	if (!udata.u_ptab->p_tty && !(flag & O_NOCTTY)) {
		udata.u_ptab->p_tty = minor;
		tty_pgrp[minor] = udata.u_ptab->p_pgrp;
	}
	tty_setup(minor);
	if ((t->c_cflag & CLOCAL) || (flag & O_NDELAY))
	        return 0;

        if (!tty_carrier(minor)) {
                if (psleep_flags(&t->c_cflag, flag))
                        return -1;
        }
        /* Carrier spiked ? */
        if (deadflag[minor]) {
                udata.u_error = ENXIO;
                deadflag[minor] = 0;
                return -1;
        }
        return 0;
}

int tty_close(uint8_t minor)
{
	/* If we are closing the controlling tty, make note */
	if (minor == udata.u_ptab->p_tty)
		udata.u_ptab->p_tty = 0;
        /* If we were hung up then the last opener has gone away */
        deadflag[minor] = 0;
	return (0);
}

int tty_ioctl(uint8_t minor, uint16_t request, char *data)
{				/* Data in User Space */
	if (!minor)
		minor = udata.u_ptab->p_tty;
	if (minor < 1 || minor > NUM_DEV_TTY + 1) {
		udata.u_error = ENODEV;
		return -1;
	}
	if (deadflag[minor]) {
	        udata.u_error = ENXIO;
	        return -1;
        }
	switch (request) {
	case TCGETS:
		uput(&ttydata[minor], data, sizeof(struct termios));
		break;
	case TCSETSW:
		/* We don't have an output queue really so for now drop
		   through */
	case TCSETS:
	case TCSETSF:
		uget(data, &ttydata[minor], sizeof(struct termios));
		if (request == TCSETSF)
			clrq(&ttyinq[minor]);
                tty_setup(minor);
		break;
	case TIOCINQ:
		uput(&ttyinq[minor].q_count, data, 2);
		break;
	case TIOCFLUSH:
		clrq(&ttyinq[minor]);
		break;
        case TIOCHANGUP:
                tty_hangup(minor);
                return 0;
	default:
		udata.u_error = ENOTTY;
		return (-1);
	}
	return (0);
}


/* This routine processes a character in response to an interrupt.  It
 * adds the character to the tty input queue, echoing and processing
 * backspace and carriage return.  If the queue contains a full line,
 * it wakes up anything waiting on it.  If it is totally full, it beeps
 * at the user.
 * UZI180 - This routine is called from the raw Hardware read routine,
 * either interrupt or polled, to process the input character.  HFB
 */

int tty_inproc(uint8_t minor, unsigned char c)
{
	unsigned char oc;
	struct termios *td;
	struct s_queue *q = &ttyinq[minor];
	int canon;
	uint16_t pgrp = tty_pgrp[minor];
	uint8_t wr;

	td = &ttydata[minor];
	canon = td->c_lflag & ICANON;

	if (td->c_iflag & ISTRIP)
		c &= 0x7f;	/* Strip off parity */
	if (canon && !c)
		return 1;	/* Simply quit if Null character */

#ifdef CONFIG_IDUMP
	if (c == 0x1a)		/* ^Z */
		idump();	/*   (For debugging) */
#endif
#ifdef CONFIG_MONITOR
	if (c == 0x01)		/* ^A */
		trap_monitor();
#endif

	if (c == '\r' && (td->c_iflag & ICRNL))
		c = '\n';
	if (c == '\n' && (td->c_iflag & INLCR))
		c = '\r';

	if (td->c_lflag & ISIG) {
		if (c == td->c_cc[VINTR]) {	/* ^C */
			sgrpsig(pgrp, SIGINT);
			clrq(q);
			stopflag[minor] = flshflag[minor] = false;
			return 1;
		} else if (c == td->c_cc[VQUIT]) {	/* ^\ */
			sgrpsig(pgrp, SIGQUIT);
			clrq(q);
			stopflag[minor] = flshflag[minor] = false;
			return 1;
		}
	}
	if (c == td->c_cc[VDISCARD]) {	/* ^O */
		flshflag[minor] = !flshflag[minor];
		return 1;
	}
	if (td->c_iflag & IXON) {
		if (c == td->c_cc[VSTOP]) {	/* ^S */
			stopflag[minor] = true;
			return 1;
		}
		if (c == td->c_cc[VSTART]) {	/* ^Q */
			stopflag[minor] = false;
			wakeup(&stopflag[minor]);
			return 1;
		}
	}
	if (canon) {
		if (c == td->c_cc[VERASE]) {
			if (uninsq(q, &oc)) {
				if (oc == '\n' || oc == td->c_cc[VEOL])
					insq(q, oc);	/* Don't erase past nl */
				else if (td->c_lflag & ECHOE)
					tty_erase(minor);
				return 1;
			} else if (c == td->c_cc[VKILL]) {
				while (uninsq(q, &oc)) {
					if (oc == '\n'
					    || oc == td->c_cc[VEOL]) {
						insq(q, oc);	/* Don't erase past nl */
						break;
					}
					if (td->c_lflag & ECHOK)
						tty_erase(minor);
				}
				return 1;
			}
		}
	}

	/* All modes come here */
	if (c == '\n') {
		if ((td->c_oflag & (OPOST | ONLCR)) == (OPOST | ONLCR))
			tty_echo(minor, '\r');
	}

	wr = insq(q, c);
	if (wr)
		tty_echo(minor, c);
	else if (minor < PTY_OFFSET)
		tty_putc_wait(minor, '\007');	/* Beep if no more room */

	if (!canon || c == td->c_cc[VEOL] || c == '\n'
	    || c == td->c_cc[VEOF])
		wakeup(q);
	return wr;
}

/* called when a UART transmitter is ready for the next character */
void tty_outproc(uint8_t minor)
{
	wakeup(&ttydata[minor]);
}

void tty_echo(uint8_t minor, unsigned char c)
{
	if (ttydata[minor].c_lflag & ECHO)
		tty_putc_wait(minor, c);
}

void tty_erase(uint8_t minor)
{
	tty_putc_wait(minor, '\b');
	tty_putc_wait(minor, ' ');
	tty_putc_wait(minor, '\b');
}


void tty_putc_wait(uint8_t minor, unsigned char c)
{
#ifdef CONFIG_DEV_PTY
	if (minor >= PTY_OFFSET)
		ptty_putc_wait(minor, c);
	else
#endif
	if (!udata.u_ininterrupt) {
		while (!tty_writeready(minor))
			psleep(&ttydata[minor]);
	}
	tty_putc(minor, c);
}

void tty_hangup(uint8_t minor)
{
        /* Kill users */
        sgrpsig(tty_pgrp[minor], SIGHUP);
        /* Stop any new I/O with errors */
        deadflag[minor] = 1;
        /* Wake up read/write */
        wakeup(&ttyinq[minor]);
        wakeup(&stopflag[minor]);
        /* and deadflag will clear when the last user goes away */
}

void tty_carrier_drop(uint8_t minor)
{
        if (ttydata[minor].c_cflag & HUPCL)
                tty_hangup(minor);
}

void tty_carrier_raise(uint8_t minor)
{
        if (ttydata[minor].c_cflag & HUPCL)
                wakeup(&ttydata[minor].c_cflag);
}

/*
 *	PTY logic
 */

#ifdef CONFIG_DEV_PTY
int ptty_open(uint8_t minor, uint16_t flag)
{
	return tty_open(minor + PTY_OFFSET, flag);
}

int ptty_close(uint8_t minor)
{
	return tty_close(minor + PTY_OFFSET);
}

int ptty_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	return tty_write(minor + PTY_OFFSET, rawflag, flag);
}

int ptty_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	return tty_read(minor + PTY_OFFSET, rawflag, flag);
}

int ptty_ioctl(uint8_t minor, uint16_t request, char *data)
{
	return tty_ioctl(minor + PTY_OFFSET, rawflag, flag);
}

int pty_open(uint8_t minor, uint16_t flag)
{
	return tty_open(minor + PTY_OFFSET, flag);
}

int pty_close(uint8_t minor)
{
	return tty_close(minor + PTY_OFFSET);
}

int pty_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	uint16_t nwritten;
	minor += PTY_OFFSET;

	while (nwritten < udata.u_count) {
		if (udata.u_sysio)
			c = udata.u_base;
		else
			c = ugetc(udata.u_base);
		if (tty_inproc(minor, c)) {
			nwritten++;
			udata.u_count++;
			continue;
		}
		if (nwritten == 0
		    && psleep_flags(&ttyinq[minor].q_count, flag))
			return -1;
	}

	return nwritten;
}

int pty_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	struct s_queue q = &ttyinq[minor + PTY_OFFSET + PTY_PAIR];
	char c;

	while (nread < udata.u_count) {
		if (remq(q, &c)) {
			if (udata.u_sysio)
				*udata.u_base = c;
			else
				uputc(c, udata.u_base);
			udata.u_base++;
			nread++;
			continue;
		}
		if (nread == 0 && psleep_flags(q, flag))
			return -1;
	}
	return nread;
}

int pty_ioctl(uint8_t minor, uint16_t request, char *data)
{
	return tty_ioctl(minor + PTY_OFFSET, rawflag, flag);
}

void pty_putc_wait(uint8_t minor, char c)
{
	struct s_queue q = &ptyq[minor + PTY_OFFSET + PTY_PAIR];
	/* tty output queue to pty */
	insq(q, c);
	wakeup(q);
}
#endif
