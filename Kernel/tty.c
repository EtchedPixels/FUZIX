#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>

/*
 *	Minimal Terminal Interface
 *
 *	TODO:
 *	- Parity
 *	- Various misc minor flags
 *	- Software Flow control
 *
 *	Add a small echo buffer to each tty
 */

struct tty ttydata[NUM_DEV_TTY + 1];	/* ttydata[0] is not used */

#ifdef CONFIG_LEVEL_2
static uint16_t tty_select;		/* Fast path if no selects, could do with being per tty ? */

/* Might be worth tracking tty minor <> inode for performance FIXME */
static void tty_selwake(uint8_t minor, uint16_t event)
{
	if (tty_select) {
		/* 2 is the tty devices */
		selwake_dev(2, minor, event);
	}
}
#else
#define tty_selwake(a,b)	do {} while(0)
#endif

int tty_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	unsigned char c;
	struct s_queue *q;
	struct tty *t;

	used(rawflag);
	used(flag);			/* shut up compiler */

	q = &ttyinq[minor];
	t = &ttydata[minor];

	while (udata.u_done < udata.u_count) {
		for (;;) {
#ifdef CONFIG_LEVEL_2		
                        if (jobcontrol_in(minor, t))
				return udata.u_done;
#endif				
		        if ((t->flag & TTYF_DEAD) && (!q->q_count))
				goto dead;
			if (remq(q, &c)) {
				if (udata.u_sysio)
					*udata.u_base = c;
				else
					uputc(c, udata.u_base);
				break;
			}
			if (!(t->termios.c_lflag & ICANON)) {
			        uint8_t n = t->termios.c_cc[VTIME];

				if ((udata.u_done || !n) && udata.u_done >= t->termios.c_cc[VMIN])
					goto out;
				if (n)
			                udata.u_ptab->p_timeout = n + 1;
                        }
			if (psleep_flags_io(q, flag))
			        goto out;
                        /* timer expired */
                        if (udata.u_ptab->p_timeout == 1)
                                goto out;
		}

		++udata.u_done;

		/* return according to mode */
		if (t->termios.c_lflag & ICANON) {
			if (udata.u_done == 1 && (c == t->termios.c_cc[VEOF])) {
				/* ^D */
				udata.u_done = 0;
				break;
			}
			if (c == '\n' || c == t->termios.c_cc[VEOL])
				break;
		}

		++udata.u_base;
	}
out:
	tty_data_consumed(minor);
	wakeup(&q->q_count);
	return udata.u_done;

dead:
        udata.u_error = ENXIO;
	return -1;
}

int tty_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	struct tty *t;
	uint8_t c;

	used(rawflag);

	t = &ttydata[minor];

	while (udata.u_done != udata.u_count) {
		for (;;) {	/* Wait on the ^S/^Q flag */
#ifdef CONFIG_LEVEL_2		
	                if (jobcontrol_out(minor, t))
				return udata.u_done;
#endif				
		        if (t->flag & TTYF_DEAD) {
			        udata.u_error = ENXIO;
			        return -1;
			}
			if (!(t->flag & TTYF_STOP))
				break;
			if (psleep_flags_io(&t->flag, flag))
				return udata.u_done;
		}
		if (!(t->flag & TTYF_DISCARD)) {
			if (udata.u_sysio)
				c = *udata.u_base;
			else
				c = ugetc(udata.u_base);

			if (t->termios.c_oflag & OPOST) {
				if (c == '\n' && (t->termios.c_oflag & ONLCR))
					if (tty_putc_maywait(minor, '\r', flag))
						break;
				else if (c == '\r' && (t->termios.c_oflag & OCRNL))
					c = '\n';
			}
			if (tty_putc_maywait(minor, c, flag))
				break;
		}
		++udata.u_base;
		++udata.u_done;
	}
	return udata.u_done;
}


int tty_open(uint8_t minor, uint16_t flag)
{
	struct tty *t;
	irqflags_t irq;

	if (minor > NUM_DEV_TTY) {
		udata.u_error = ENODEV;
		return -1;
	}

	t = &ttydata[minor];

	/* Hung up but not yet cleared of users */
	if (t->flag & TTYF_DEAD) {
	        udata.u_error = ENXIO;
	        return -1;
        }

	if (t->users) {
	        t->users++;
	        return 0;
        }
	tty_setup(minor);
	if ((t->termios.c_cflag & CLOCAL) || (flag & O_NDELAY))
		goto out;

	irq = di();
        if (!tty_carrier(minor)) {
		if (psleep_flags(&t->termios.c_cflag, flag)) {
			irqrestore(irq);
                        return -1;
		}
        }
        irqrestore(irq);
        /* Carrier spiked ? */
        if (t->flag & TTYF_DEAD) {
                udata.u_error = ENXIO;
                t->flag &= ~TTYF_DEAD;
                return -1;
        }
 out:   t->users++;
        return 0;
}

/* Post processing for a successful tty open */
void tty_post(inoptr ino, uint8_t minor, uint16_t flag)
{
        struct tty *t = &ttydata[minor];
        irqflags_t irq = di();

	/* If there is no controlling tty for the process, establish it */
	/* Disable interrupts so we don't endup setting up our control after
	   the carrier drops and tries to undo it.. */
	if (!(t->flag & TTYF_DEAD) && !udata.u_ptab->p_tty && !t->pgrp && !(flag & O_NOCTTY)) {
		udata.u_ptab->p_tty = minor;
		udata.u_ctty = ino;
		t->pgrp = udata.u_ptab->p_pgrp;
#ifdef DEBUG
		kprintf("setting tty %d pgrp to %d for pid %d\n",
		        minor, t->pgrp, udata.u_ptab->p_pid);
#endif
	}
	irqrestore(irq);
}

int tty_close(uint8_t minor)
{
        struct tty *t = &ttydata[minor];
        if (--t->users)
                return 0;
	/* If we are closing the controlling tty, make note */
	if (minor == udata.u_ptab->p_tty) {
		udata.u_ptab->p_tty = 0;
		udata.u_ctty = NULL;
#ifdef DEBUG
		kprintf("pid %d loses controller\n", udata.u_ptab->p_pid);
#endif
        }
	t->pgrp = 0;
        /* If we were hung up then the last opener has gone away */
        t->flag &= ~TTYF_DEAD;
#ifdef DEBUG
        kprintf("tty %d last close\n", minor);
#endif
	return (0);
}

/* If the group owner for the tty dies, the tty loses its group */
void tty_exit(void)
{
        uint8_t t = udata.u_ptab->p_tty;
        uint16_t *pgrp = &ttydata[t].pgrp;
        if (t && *pgrp == udata.u_ptab->p_pgrp && *pgrp == udata.u_ptab->p_pid)
                *pgrp = 0;
}

int tty_ioctl(uint8_t minor, uarg_t request, char *data)
{				/* Data in User Space */
        struct tty *t;

        t = &ttydata[minor];

        /* Special case select ending after a hangup */
#ifdef CONFIG_LEVEL_2
	if (request == SELECT_END) {
		tty_select--;
		return 0;
	}
        if (jobcontrol_ioctl(minor, t, request))
		return -1;
#endif		
	if (t->flag & TTYF_DEAD) {
	        udata.u_error = ENXIO;
	        return -1;
        }

	switch (request) {
	case TCGETS:
		return uput(&t->termios, data, sizeof(struct termios));
		break;
	case TCSETSF:
		clrq(&ttyinq[minor]);
		/* Fall through for now */
	case TCSETSW:
		/* We don't have an output queue really so for now drop
		   through */
	case TCSETS:
		if (uget(data, &t->termios, sizeof(struct termios)) == -1)
		        return -1;
                tty_setup(minor);
                tty_selwake(minor, SELECT_IN|SELECT_OUT);
		break;
	case TIOCINQ:
		return uput(&ttyinq[minor].q_count, data, 2);
	case TIOCFLUSH:
		clrq(&ttyinq[minor]);
                tty_selwake(minor, SELECT_OUT);
		break;
        case TIOCHANGUP:
                tty_hangup(minor);
                return 0;
	case TIOCOSTOP:
		t->flag |= TTYF_STOP;
		break;
	case TIOCOSTART:
		t->flag &= ~TTYF_STOP;
                tty_selwake(minor, SELECT_OUT);
		break;
        case TIOCGWINSZ:
                return uput(&t->winsize, data, sizeof(struct winsize));
        case TIOCSWINSZ:
                if (uget(data, &t->winsize, sizeof(struct winsize)))
                        return -1;
                sgrpsig(t->pgrp, SIGWINCH);
                return 0;
        case TIOCGPGRP:
                return uputi(t->pgrp, data);
#ifdef CONFIG_LEVEL_2
        case TIOCSPGRP:
                /* Only applicable via controlling terminal */
                if (minor != udata.u_ptab->p_tty) {
                        udata.u_error = ENOTTY;
                        return -1;
                }
                return tcsetpgrp(t, data);
	case SELECT_BEGIN:
		tty_select++;
		/* Fall through */
	case SELECT_TEST:
	{
		uint8_t n = *data;
		*data = 0;
		if (n & SELECT_EX) {
			if (t->flag & TTYF_DEAD) {
				*data = SELECT_IN|SELECT_EX;
				return 0;
			}
		}
		/* FIXME: IRQ race */
		if (n & SELECT_IN) {
			/* TODO - this one is hard, we need to peek down the queue to see if
			   a canonical input would succeed */
		}
		if (n & SELECT_OUT) {
			if ((!(t->flag & TTYF_STOP)) &&
				tty_writeready(minor) != TTY_READY_LATER)
					*data |= SELECT_OUT;
		}
		return 0;
	}
#endif
	default:
		udata.u_error = ENOTTY;
		return -1;
	}
	return 0;
}


/* This routine processes a character in response to an interrupt.  It
 * adds the character to the tty input queue, echoing and processing
 * backspace and carriage return.  If the queue contains a full line,
 * it wakes up anything waiting on it.  If it is totally full, it beeps
 * at the user.
 * UZI180 - This routine is called from the raw Hardware read routine,
 * either interrupt or polled, to process the input character.  HFB
 */
uint8_t tty_inproc(uint8_t minor, unsigned char c)
{
	unsigned char oc;
	uint8_t canon;
	uint8_t wr;
	struct tty *t = &ttydata[minor];
	struct s_queue *q = &ttyinq[minor];

	/* This is safe as ICANON is in the low bits */
	canon = t->termios.c_lflag & ICANON;

	if (t->termios.c_iflag & ISTRIP)
		c &= 0x7f;	/* Strip off parity */
	if (canon && !c)
		return 1;	/* Simply quit if Null character */

#ifdef CONFIG_IDUMP
	if (c == 0x1a)		/* ^Z */
		idump();	/*   (For debugging) */
#endif
#ifdef CONFIG_MONITOR
	if (c == 0x01)		/* ^A */
		platform_monitor();
#endif

	if (c == '\r' ){
		if(t->termios.c_iflag & IGNCR )
			return 1;
		if(t->termios.c_iflag & ICRNL)
			c = '\n';
	}
	/* Q: should this be else .. */
	if (c == '\n' && (t->termios.c_iflag & INLCR))
		c = '\r';

	if (t->termios.c_lflag & ISIG) {
		if (c == t->termios.c_cc[VINTR]) {	/* ^C */
		        wr = SIGINT;
			goto sigout;
		} else if (c == t->termios.c_cc[VQUIT]) {	/* ^\ */
		        wr = SIGQUIT;
sigout:
			sgrpsig(t->pgrp, wr);
			clrq(q);
			t->flag &= ~(TTYF_STOP | TTYF_DISCARD);
			return 1;
		}
	}
	if (t->termios.c_iflag & IXON) {
		if (c == t->termios.c_cc[VSTOP]) {	/* ^S */
		        t->flag |= TTYF_STOP;
			return 1;
		}
		if (c == t->termios.c_cc[VSTART]) {	/* ^Q */
		        t->flag &= ~TTYF_STOP;
			wakeup(&t->flag);
			tty_selwake(minor, SELECT_OUT);
			return 1;
		}
	}
	if (canon) {
		if (c == t->termios.c_cc[VDISCARD]) {	/* ^O */
		        t->flag ^= TTYF_DISCARD;
			return 1;
		}
		if (c == t->termios.c_cc[VERASE]) {
		        wr = ECHOE;
		        goto eraseout;
		} else if (c == t->termios.c_cc[VKILL]) {
		        wr = ECHOK;
		        goto eraseout;
		}
	}

	/* All modes come here */
	if (c == '\n') {
		if ((t->termios.c_oflag & (OPOST | ONLCR)) == (OPOST | ONLCR))
			tty_echo(minor, '\r');
	}

	wr = insq(q, c);
	if (wr && (!canon || c != t->termios.c_cc[VEOF]))
		tty_echo(minor, c);
	else
		tty_putc(minor, '\007');	/* Beep if no more room */

	if (!canon || c == t->termios.c_cc[VEOL] || c == '\n'
	    || c == t->termios.c_cc[VEOF]) {
		wakeup(q);
		tty_selwake(minor, SELECT_IN);
	}
	return wr;

eraseout:
	while (uninsq(q, &oc)) {
		if (oc == '\n' || oc == t->termios.c_cc[VEOL]) {
			insq(q, oc);	/* Don't erase past nl */
			break;
		}
		if (t->termios.c_lflag & wr)
			tty_erase(minor);
                if (wr == ECHOE)
                        break;
	}
	return 1;
}

/* called when a UART transmitter is ready for the next character */
void tty_outproc(uint8_t minor)
{
	wakeup(&ttydata[minor]);
	tty_selwake(minor, SELECT_OUT);
}

void tty_echo(uint8_t minor, unsigned char c)
{
	if (ttydata[minor].termios.c_lflag & ECHO)
		tty_putc_wait(minor, c);
}

void tty_erase(uint8_t minor)
{
	tty_putc_wait(minor, '\b');
	tty_putc_wait(minor, ' ');
	tty_putc_wait(minor, '\b');
}


uint8_t tty_putc_maywait(uint8_t minor, unsigned char c, uint8_t flag)
{
        uint8_t t;

        flag &= O_NDELAY;

#ifdef CONFIG_DEV_PTY
	if (minor >= PTY_OFFSET)
		ptty_putc_wait(minor, c);
	else
#endif
        /* For slower platforms it's not worth the task switching and return
           costs versus waiting a bit. A box with tx interrupts and sufficient
           performance can buffer or sleep in tty_putc instead.

           The driver should return a value from the ttyready_t enum:
            1 (TTY_READY_NOW) -- send bytes now
            0 (TTY_READY_SOON) -- spinning may be useful
           -1 (TTY_READY_LATER) -- blocked, don't spin (eg flow controlled)

           FIXME: we can make tty_sleeping an add on to a hardcoded function using tty
	   flags, and tty_outproc test it. Then only devices wanting to mask
	   an actual IRQ need to care

	*/
	if (!udata.u_ininterrupt) {
		while ((t = tty_writeready(minor)) != TTY_READY_NOW) {
			if (chksigs()) {
				udata.u_error = EINTR;
				return 1;
			}
			if (t != TTY_READY_SOON || need_reschedule()){
				irqflags_t irq;

				if (flag) {
					udata.u_error = EAGAIN;
					return 1;
				}
				irq = di();
				tty_sleeping(minor);
				psleep(&ttydata[minor]);
				irqrestore(irq);
			}
		}
	}
	tty_putc(minor, c);
	return 0;
}

void tty_putc_wait(uint8_t minor, unsigned char ch)
{
	tty_putc_maywait(minor, ch, 0);
}

void tty_hangup(uint8_t minor)
{
        struct tty *t = &ttydata[minor];
        /* Kill users */
        sgrpsig(t->pgrp, SIGHUP);
        sgrpsig(t->pgrp, SIGCONT);
        t->pgrp = 0;
        /* Stop any new I/O with errors */
        t->flag |= TTYF_DEAD;
        /* Wake up read/write */
        wakeup(&ttyinq[minor]);
        /* Wake stopped stuff */
        wakeup(&t->flag);
        /* and deadflag will clear when the last user goes away */
	tty_selwake(minor, SELECT_IN|SELECT_OUT|SELECT_EX);
}

void tty_carrier_drop(uint8_t minor)
{
        if (ttydata[minor].termios.c_cflag & HUPCL)
                tty_hangup(minor);
}

void tty_carrier_raise(uint8_t minor)
{
	wakeup(&ttydata[minor].termios.c_cflag);
	tty_selwake(minor, SELECT_IN|SELECT_OUT);
}

/*
 *	PTY logic
 */

#ifdef CONFIG_DEV_PTY

static uint8_t ptyusers[PTY_PAIR];

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
	int r = tty_open(minor + PTY_OFFSET, flag | O_NOCTTY | O_NDELAY);
	if (r == 0) {
		if (!ptyusers[minor])
			tty_carrier_raise(minor + PTY_OFFSET);
		ptyusers[minor]++;
	}
	return r;
}

int pty_close(uint8_t minor)
{
	ptyusers[minor]--;
	if (ptyusers[minor] == 0)
		tty_carrider_drop(minor + PTY_OFFSET);
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
	/* FIXME: select */
	wakeup(q);
}
#endif
