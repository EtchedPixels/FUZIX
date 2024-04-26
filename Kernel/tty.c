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
static void tty_selwake(uint_fast8_t minor, uint16_t event)
{
	if (tty_select) {
		/* 2 is the tty devices */
		selwake_dev(2, minor, event);
	}
}
#else
#define tty_selwake(a,b)	do {} while(0)
#endif

int tty_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	uint_fast8_t c;
	register struct s_queue *q;
	register struct tty *t;

	/* FIXME: fix race of timer versus the ptimer_insert to psleep_flags_io */
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
			        uint_fast8_t n = t->termios.c_cc[VTIME];

				if ((udata.u_done || !n) && udata.u_done >= t->termios.c_cc[VMIN])
					goto out;
				if (n) {
			                udata.u_ptab->p_timeout = n + 1;
			                ptimer_insert();
				}
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

int tty_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	register struct tty *t;
	uint_fast8_t c;

	used(rawflag);

	if (!valaddr_r(udata.u_base, udata.u_count))
		return -1;

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
		/* We could optimize this significantly by 
		   a) looping here if not sleeping rather than repeating all
		   the checks except for STOP/DISCARD
		   b) possibly batching for the case where tty never blocks
		   if we have a way to report that. We could then batch except
		   for conversions and also fast path in vt for 'normal char
		   next char normal'
		   c) look at hint/batching for ugetc into a local work buffer
		*/
		if (!(t->flag & TTYF_DISCARD)) {
			if (udata.u_sysio)
				c = *udata.u_base;
			else
				c = _ugetc(udata.u_base);

			if (t->termios.c_oflag & OPOST) {
				if (c == '\n' && (t->termios.c_oflag & ONLCR)) {
					if (tty_putc_maywait(minor, '\r', flag))
						break;
				} else if (c == '\r' && (t->termios.c_oflag & OCRNL))
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


int tty_open(uint_fast8_t minor, uint16_t flag)
{
	register struct tty *t;
	irqflags_t irq;

	if (minor > NUM_DEV_TTY) {
		udata.u_error = ENODEV;
		return -1;
	}

	t = &ttydata[minor];

	/*
	 *	If the tty has users but is marked dead then we are still
	 *	cleaning up from a carrier drop. If it didn't have users
	 *	then this is fine.
	 */
	if (t->users) {
		if (t->flag & TTYF_DEAD) {
		        udata.u_error = ENXIO;
		        return -1;
	        }
	        t->users++;
	        return 0;
        }

        t->flag &= ~TTYF_DEAD;

	tty_setup(minor, 0);

	/*
	 *	Do not wait for carrier in these cases. If the port has
	 *	no carrier tty_carrier always returns 1 so that hardware
	 *	detail is abstracted
	 */
	if ((t->termios.c_cflag & CLOCAL) || (flag & O_NDELAY))
		goto out;

	/* Wait for carrier */
	irq = di();
        if (!tty_carrier(minor)) {
		if (psleep_flags(&t->termios.c_cflag, flag)) {
			irqrestore(irq);
                        return -1;
		}
        }
        irqrestore(irq);
        /* Carrier rose and then fell again during the open ? */
        if (t->flag & TTYF_DEAD) {
                udata.u_error = ENXIO;
                t->flag &= ~TTYF_DEAD;
                return -1;
        }
out:
	/* Track users */
	t->users++;
        return 0;
}

/* Post processing for a successful tty open */
void tty_post(inoptr ino, uint_fast8_t minor, uint16_t flag)
{
	register struct tty *t = &ttydata[minor];
        irqflags_t irq = di();

	/* If there is no controlling tty for the process, establish it */
	/* Disable interrupts so we don't end up setting up our control after
	   the carrier drops and tries to undo it.. */
	if (!(t->flag & TTYF_DEAD) && !udata.u_ptab->p_tty && !t->pgrp && !(flag & O_NOCTTY)) {
		udata.u_ptab->p_tty = minor;
		udata.u_ctty = ino;
		t->pgrp = udata.u_ptab->p_pgrp;
	}
	irqrestore(irq);
}

int tty_close(uint_fast8_t minor)
{
        struct tty *t = &ttydata[minor];
        /* We only care about the final close */
        if (--t->users)
                return 0;
	/* If we are closing the controlling tty, make note */
	if (minor == udata.u_ptab->p_tty) {
		udata.u_ptab->p_tty = 0;
		udata.u_ctty = NULL;
        }
	t->pgrp = 0;
        /* If we were hung up then the last opener has gone away. This may
           race but we also check in the open path */
        t->flag &= ~TTYF_DEAD;
	return (0);
}

/* If the group owner for the tty dies, the tty loses its group */
void tty_exit(void)
{
        uint_fast8_t t = udata.u_ptab->p_tty;
        uint16_t *pgrp = &ttydata[t].pgrp;
        if (t && *pgrp == udata.u_ptab->p_pgrp && *pgrp == udata.u_ptab->p_pid)
                *pgrp = 0;
}

int tty_ioctl(uint_fast8_t minor, uarg_t request, char *data)
{				/* Data in User Space */
        register struct tty *t;
        uint_fast8_t waito = 0;
        staticfast struct termios tm;

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
	case TCSETSF:
		clrq(&ttyinq[minor]);
		/* Fall through for now */
	case TCSETSW:
		waito = 1;
	case TCSETS:
	{
		tcflag_t mask = termios_mask[minor];
		if (uget(data, &tm, sizeof(struct termios)) == -1)
		        return -1;
		memcpy(t->termios.c_cc, tm.c_cc, NCCS);
		t->termios.c_iflag = tm.c_iflag;
		t->termios.c_oflag = tm.c_oflag;
		t->termios.c_cflag &= ~mask;
		tm.c_cflag &= mask;
		t->termios.c_cflag |= tm.c_cflag;
		t->termios.c_lflag = tm.c_lflag;
                tty_setup(minor, waito);
                tty_selwake(minor, SELECT_IN|SELECT_OUT);
		break;
	}
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
                return uputw(t->pgrp, data);
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
uint_fast8_t tty_inproc(uint_fast8_t minor, register uint_fast8_t c)
{
	uint_fast8_t oc;
	uint_fast8_t canon;
	uint_fast8_t wr;
	register struct tty *t = &ttydata[minor];
	register struct s_queue *q = &ttyinq[minor];

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
		plt_monitor();
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

	/* TODO: we need to find a way to batch queue the erases
	   so we don't stress the uart queues off interrupts */
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
void tty_outproc(uint_fast8_t minor)
{
	wakeup(&ttydata[minor]);
	tty_selwake(minor, SELECT_OUT);
}

void tty_echo(uint_fast8_t minor, uint_fast8_t c)
{
	if (ttydata[minor].termios.c_lflag & ECHO)
		tty_putc_wait(minor, c);
}

void tty_erase(uint_fast8_t minor)
{
	tty_putc_wait(minor, '\b');
	tty_putc_wait(minor, ' ');
	tty_putc_wait(minor, '\b');
}


uint_fast8_t tty_putc_maywait(uint_fast8_t minor, uint_fast8_t c, uint_fast8_t flag)
{
        uint_fast8_t t;

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
				if (t != TTY_READY_SOON) {
					irq = di();
					tty_sleeping(minor);
					psleep(&ttydata[minor]);
					irqrestore(irq);
				} else
					/* Yield */
					switchout();
			}
		}
	}
	tty_putc(minor, c);
	return 0;
}

void tty_putc_wait(uint_fast8_t minor, uint_fast8_t ch)
{
	tty_putc_maywait(minor, ch, 0);
}

/*
 *	This can occur in kernel or interrupt context. We therefore need
 *	to be careful when we are racing open or close. If we race a close
 *	the TTYF_DEAD gets set unnecessarily which is fine as open will
 *	clean it up. If we race an open then open clears the flag early
 *	and checks it before completion.
 */
void tty_hangup(uint_fast8_t minor)
{
        register struct tty *t = &ttydata[minor];
        if (t->users) {
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
}

void tty_carrier_drop(uint_fast8_t minor)
{
        if (ttydata[minor].termios.c_cflag & HUPCL)
                tty_hangup(minor);
}

void tty_carrier_raise(uint_fast8_t minor)
{
	wakeup(&ttydata[minor].termios.c_cflag);
	tty_selwake(minor, SELECT_IN|SELECT_OUT);
}

/*
 *	PTY logic
 */

#ifdef CONFIG_DEV_PTY

static uint8_t ptyusers[PTY_PAIR];

int ptty_open(uint_fast8_t minor, uint16_t flag)
{
	return tty_open(minor + PTY_OFFSET, flag);
}

int ptty_close(uint_fast8_t minor)
{
	return tty_close(minor + PTY_OFFSET);
}

int ptty_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	return tty_write(minor + PTY_OFFSET, rawflag, flag);
}

int ptty_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	return tty_read(minor + PTY_OFFSET, rawflag, flag);
}

int ptty_ioctl(uint_fast8_t minor, uint16_t request, char *data)
{
	return tty_ioctl(minor + PTY_OFFSET, rawflag, flag);
}

int pty_open(uint_fast8_t minor, uint16_t flag)
{
	int r = tty_open(minor + PTY_OFFSET, flag | O_NOCTTY | O_NDELAY);
	if (r == 0) {
		if (!ptyusers[minor])
			tty_carrier_raise(minor + PTY_OFFSET);
		ptyusers[minor]++;
	}
	return r;
}

int pty_close(uint_fast8_t minor)
{
	ptyusers[minor]--;
	if (ptyusers[minor] == 0)
		tty_carrider_drop(minor + PTY_OFFSET);
	return tty_close(minor + PTY_OFFSET);
}

int pty_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
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

int pty_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
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

int pty_ioctl(uint_fast8_t minor, uint16_t request, char *data)
{
	return tty_ioctl(minor + PTY_OFFSET, rawflag, flag);
}

void pty_putc_wait(uint_fast8_t minor, char c)
{
	struct s_queue q = &ptyq[minor + PTY_OFFSET + PTY_PAIR];
	/* tty output queue to pty */
	insq(q, c);
	/* FIXME: select */
	wakeup(q);
}
#endif

/* Additional functionality for bigger systems

	tty_inproc_bad allows the queueing of bytes detected as damaged
	by the UART hardware (except parity)

	tty_inproc_full should be used for normal error free queueing if
	using the full UART feature set

	tty_inproc_softparity should be used to queue bytes when doing
	7E1/7O1/7M1/7S1 in software or for a byte with parity hardware
	detected as wrong

	tty_break_event should be called when an input break condition
	is detected.

	This adds handling of PARMRK, PAREN, PARODD, CMSPAR, BRKINT
	and IGNBRK over the base feature set.
 */

#ifdef CONFIG_LEVEL_2

/* (0x96 >> !! (0x9669 & (1 << (ch >> 4)))) & (1 << (ch & 0x07)); */

static uint8_t paritytab[16] = {
	0x96, 0x69, 0x69, 0x96,
	0x69, 0x96, 0x96, 0x69,
	0x69, 0x96, 0x96, 0x69,
	0x96, 0x69, 0x69, 0x96
};

uint_fast8_t parity(uint8_t ch)
{
	ch &= 0x7F;	/* Strip parity bit */
	return paritytab[ch >> 4] & (1 << (ch & 7));
}

/* Do 7X1 7X2 parity */
uint8_t tty_add_parity(uint_fast8_t minor, uint8_t ch)
{
	struct termios *t = &ttydata[minor].termios;
	if (t->c_cflag & PARENB) {
		ch &= 0x7F;
		if (t->c_cflag & CMSPAR) {
			if (t->c_cflag & PARODD)
				ch |= 0x80;
		} else {
			ch |= parity(ch) ? 0x00 : 0x80;
		}
	}
	return ch;
}

int tty_inproc_bad(uint_fast8_t minor, uint_fast8_t ch)
{
	struct s_queue *q = ttyinq + minor;
	return insq(q, '\377') & insq(q, '\0') &
		tty_inproc(minor, ch);
}

int tty_inproc_full(uint_fast8_t minor, uint_fast8_t ch)
{
	if (ch == '\377' && (ttydata[minor].termios.c_iflag & (PARMRK|ISTRIP)) != PARMRK)
		insq(ttyinq + minor, ch);
	return tty_inproc(minor, ch);
}

int tty_inproc_softparity(uint_fast8_t minor, uint_fast8_t ch)
{
	uint_fast8_t p = 0x00;
	register struct termios *t = &ttydata[minor].termios;

	if (!(t->c_cflag & PARENB))
		return tty_inproc_full(minor, ch);

	if (!(t->c_cflag & CMSPAR))
		p = parity(ch) ? 0x00 : 0x80;
	if (t->c_cflag & PARODD)
		p ^= 0x80;

	/* We only handle 7 bit not weird stuff like 6bit parity */
	if (p ^ ch ^ 0x80) {
		/* Bad parity */
		if (t->c_cflag & IGNPAR)
			return 0;
		return tty_inproc_bad(minor,ch);
	}
	return tty_inproc_full(minor, ch);
}

void tty_break_event(uint_fast8_t minor)
{
	register struct tty *tty = ttydata + minor;
	struct s_queue *q = ttyinq + minor;

	if (tty->termios.c_iflag & IGNBRK)
		return;
	if (tty->termios.c_iflag & BRKINT) {
		sgrpsig(tty->pgrp, SIGINT);
		clrq(ttyinq + minor);
		return;
	}
	if (tty->termios.c_iflag & PARMRK) {
		insq(q, '\377');
		insq(q, 0);
	}
	tty_inproc(minor, 0);
}

#endif
