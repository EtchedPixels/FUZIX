//
// resolv.c
//
// DNS resolver
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright (C) 1996-2002  Internet Software Consortium.
// Portions Copyright (C) 1996-2001  Nominum, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "resolv.h"

struct res_state res;

static int res_nsend(struct res_state *statp, const char *buf, int buflen,
		     char *answer, int anslen);

static int res_nquery(struct res_state *statp, const char *dname,
		      int class, int type, unsigned char *answer,
		      int anslen);

static int res_nsearch(struct res_state *statp, const char *name,
		       int class, int type, unsigned char *answer,
		       int anslen);

static int res_nquerydomain(struct res_state *statp, const char *name,
			    const char *domain, int class, int type,
			    unsigned char *answer, int anslen);


static int res_nmkquery(struct res_state *statp, int op, const char *dname,
			int class, int type,
			const unsigned char *data, int datalen,
			unsigned char *newrr,
			unsigned char *buf, int buflen);

//
// special
//

static int special(int ch)
{
	switch (ch) {
	case 0x22:		// '"'
	case 0x2E:		// '.'
	case 0x3B:		// ';'
	case 0x5C:		// '\\'
		// Special modifiers in zone files
	case 0x40:		// '@'
	case 0x24:		// '$'
		return 1;
	default:
		return 0;
	}
}

//
// printable
//

static int printable(int ch)
{
	return (ch > 0x20 && ch < 0x7f);
}


//
// dn_find
//
// Search for the counted-label name in an array of compressed names.
// Returns offset from msg if found, or error.
//
// dnptrs is the pointer to the first name on the list,
// not the pointer to the start of the message.
//

static int dn_find(unsigned char *domain, unsigned char *msg,
		   unsigned char **dnptrs, unsigned char **lastdnptr)
{
	unsigned char *dn, *cp, *sp;
	unsigned char **cpp;
	unsigned int n;

	for (cpp = dnptrs; cpp < lastdnptr; cpp++) {
		sp = *cpp;

		// Terminate search on:
		//   - root label
		//   - compression pointer
		//   - unusable offset
		while (*sp != 0 && (*sp & NS_CMPRSFLGS) == 0
		       && (sp - msg) < 0x4000) {
			dn = domain;
			cp = sp;
			while ((n = *cp++) != 0) {
				// Check for indirection
				switch (n & NS_CMPRSFLGS) {
				case 0:	// normal case, n == len
					if (n != *dn++)
						goto next;
					for (; n > 0; n--)
						if (tolower(*dn++) !=
						    tolower(*cp++))
							goto next;

					// Is next root for both ?
					if (*dn == '\0' && *cp == '\0')
						return (sp - msg);
					if (*dn)
						continue;
					goto next;

				case NS_CMPRSFLGS:	// indirection
					cp = msg +
					    (((n & 0x3f) << 8) | *cp);
					break;

				default:	// illegal type
					errno = EMSGSIZE;
					return -1;
				}
			}
		      next:
			sp += *sp + 1;
		}
	}

	errno = ENOENT;
	return -1;
}

//
// ns_name_ntop
//
// Convert an encoded domain name to printable ascii as per RFC1035.
// Returns number of bytes written to buffer, or error.
// The root is returned as "."
// All other domains are returned in non absolute form
//

int ns_name_ntop(const unsigned char *src, char *dst, int dstsiz)
{
	const unsigned char *cp;
	unsigned char *dn, *eom;
	unsigned char c;
	unsigned int n;

	cp = src;
	dn = dst;
	eom = dst + dstsiz;

	while ((n = *cp++) != 0) {
		if ((n & NS_CMPRSFLGS) != 0) {
			errno = EMSGSIZE;
			return -1;
		}

		if (dn != dst) {
			if (dn >= eom) {
				errno = EMSGSIZE;
				return -1;
			}
			*dn++ = '.';
		}

		if (dn + n >= eom) {
			errno = EMSGSIZE;
			return -1;
		}
		for (; n > 0; n--) {
			c = *cp++;
			if (special(c)) {
				if (dn + 1 >= eom) {
					errno = EMSGSIZE;
					return -1;
				}
				*dn++ = '\\';
				*dn++ = (char) c;
			} else if (!printable(c)) {
				if (dn + 3 >= eom) {
					errno = EMSGSIZE;
					return -1;
				}
				*dn++ = '\\';
				*dn++ = c / 100 + '0';
				*dn++ = (c % 100) / 10 + '0';
				*dn++ = c % 10 + '0';
			} else {
				if (dn >= eom) {
					errno = EMSGSIZE;
					return -1;
				}
				*dn++ = (char) c;
			}
		}
	}

	if (dn == dst) {
		if (dn >= eom) {
			errno = EMSGSIZE;
			return -1;
		}
		*dn++ = '.';
	}

	if (dn >= eom) {
		errno = EMSGSIZE;
		return -1;
	}
	*dn++ = '\0';

	return dn - dst;
}

//
// ns_name_pton
//      
// Convert a ascii string into an encoded domain name as per RFC1035.
//
// Return:
//     <0 if it fails
//      1 if string was fully qualified
//      0 if string was not fully qualified
//
// Enforces label and domain length limits.
//

int ns_name_pton(const char *src, unsigned char *dst, int dstsiz)
{
	unsigned char *label, *bp, *eom;
	int c, n, escaped;

	escaped = 0;
	bp = dst;
	eom = dst + dstsiz;
	label = bp++;

	while ((c = *src++) != 0) {
		if (escaped) {
			if (c >= '0' && c <= '9') {
				n = (c - '0') * 100;
				if ((c = *src++) == 0 || c < '0'
				    || c > '9') {
					errno = EINVAL;
					return -1;
				}
				n += (c - '0') * 10;
				if ((c = *src++) == 0 || c < '0'
				    || c > '9') {
					errno = EINVAL;
					return -1;
				}
				n += (c - '0');
				if (n > 255) {
					errno = EINVAL;
					return -1;
				}
				c = n;
			}
			escaped = 0;
		} else if (c == '\\') {
			escaped = 1;
			continue;
		} else if (c == '.') {
			c = (bp - label - 1);
			if ((c & NS_CMPRSFLGS) != 0) {
				errno = EMSGSIZE;
				return -1;
			}
			if (label >= eom) {
				errno = EMSGSIZE;
				return -1;
			}
			*label = c;

			// Fully qualified ?
			if (*src == '\0') {
				if (c != 0) {
					if (bp >= eom) {
						errno = EMSGSIZE;
						return -1;
					}
					*bp++ = '\0';
				}
				if ((bp - dst) > NS_MAXCDNAME) {
					errno = EMSGSIZE;
					return -1;
				}
				return 1;
			}

			if (c == 0 || *src == '.') {
				errno = EMSGSIZE;
				return -1;
			}
			label = bp++;
			continue;
		}

		if (bp >= eom) {
			errno = EMSGSIZE;
			return -1;
		}
		*bp++ = (unsigned char) c;
	}

	c = (bp - label - 1);
	if ((c & NS_CMPRSFLGS) != 0) {
		errno = EMSGSIZE;
		return -1;
	}
	if (label >= eom) {
		errno = EMSGSIZE;
		return -1;
	}
	*label = c;
	if (c != 0) {
		if (bp >= eom) {
			errno = EMSGSIZE;
			return -1;
		}
		*bp++ = 0;
	}
	if ((bp - dst) > NS_MAXCDNAME) {
		errno = EMSGSIZE;
		return -1;
	}

	return 0;
}

//
// ns_name_pack
//
// Pack domain name 'src' into 'dst'.
// Returns size of the compressed name, or error.
//
// dnptrs' is an array of pointers to previous compressed names.
// dnptrs[0] is a pointer to the beginning of the message. The array
// ends with NULL.
// 'lastdnptr' is a pointer to the end of the array pointed to
// by 'dnptrs'.
//
// The list of pointers in dnptrs is updated for labels inserted into
// the message as we compress the name.  If 'dnptr' is NULL, we don't
// try to compress names. If 'lastdnptr' is NULL, we don't update the
// list.
//

static int ns_name_pack(const unsigned char *src, unsigned char *dst,
			int dstsiz, unsigned char **dnptrs,
			unsigned char **lastdnptr)
{
	unsigned char *dstp;
	unsigned char **cpp, **lpp, *eob, *msg;
	unsigned char *srcp;
	int n, l, first = 1;

	srcp = (unsigned char *) src;
	dstp = dst;
	eob = dstp + dstsiz;
	lpp = cpp = NULL;
	if (dnptrs != NULL) {
		if ((msg = *dnptrs++) != NULL) {
			for (cpp = dnptrs; *cpp != NULL; cpp++);
			lpp = cpp;
		}
	} else {
		msg = NULL;
	}

	// Make sure the domain we are about to add is legal
	l = 0;
	do {
		n = *srcp;
		if ((n & NS_CMPRSFLGS) != 0) {
			errno = EMSGSIZE;
			return -1;
		}
		l += n + 1;
		if (l > NS_MAXCDNAME) {
			errno = EMSGSIZE;
			return -1;
		}
		srcp += n + 1;
	} while (n != 0);

	// From here on we need to reset compression pointer array on error
	srcp = (unsigned char *) src;
	do {
		// Look to see if we can use pointers
		n = *srcp;
		if (n != 0 && msg != NULL) {
			l = dn_find(srcp, msg, dnptrs, lpp);
			if (l >= 0) {
				if (dstp + 1 >= eob)
					goto cleanup;
				*dstp++ = (l >> 8) | NS_CMPRSFLGS;
				*dstp++ = l % 256;
				return dstp - dst;
			}
			// Not found, save it
			if (lastdnptr != NULL && cpp < lastdnptr - 1
			    && (dstp - msg) < 0x4000 && first) {
				*cpp++ = dstp;
				*cpp = NULL;
				first = 0;
			}
		}
		// Copy label to buffer
		if (n & NS_CMPRSFLGS)
			goto cleanup;
		if (dstp + 1 + n >= eob)
			goto cleanup;
		memcpy(dstp, srcp, n + 1);
		srcp += n + 1;
		dstp += n + 1;
	} while (n != 0);

	if (dstp > eob) {
	      cleanup:
		if (msg != NULL)
			*lpp = NULL;
		errno = EMSGSIZE;
		return -1;
	}

	return dstp - dst;
}

//
// ns_name_unpack
//
// Unpack a domain name from a message, source may be compressed.
// Return error if it fails, or consumed octets if it succeeds.
//

static int ns_name_unpack(const unsigned char *msg,
			  const unsigned char *eom,
			  const unsigned char *src, unsigned char *dst,
			  int dstsiz)
{
	const unsigned char *srcp, *dstlim;
	unsigned char *dstp;
	int n, len, checked;

	len = -1;
	checked = 0;
	dstp = dst;
	srcp = src;
	dstlim = dst + dstsiz;
	if (srcp < msg || srcp >= eom) {
		errno = EMSGSIZE;
		return -1;
	}
	// Fetch next label in domain name
	while ((n = *srcp++) != 0) {
		// Check for indirection
		switch (n & NS_CMPRSFLGS) {
		case 0:
			// Limit checks
			if (dstp + n + 1 >= dstlim || srcp + n >= eom) {
				errno = EMSGSIZE;
				return -1;
			}
			checked += n + 1;
			*dstp++ = n;
			memcpy(dstp, srcp, n);
			dstp += n;
			srcp += n;
			break;

		case NS_CMPRSFLGS:
			if (srcp >= eom) {
				errno = EMSGSIZE;
				return -1;
			}
			if (len < 0)
				len = srcp - src + 1;
			srcp = msg + (((n & 0x3F) << 8) | (*srcp & 0xFF));
			if (srcp < msg || srcp >= eom) {
				errno = EMSGSIZE;
				return -1;
			}
			checked += 2;

			// Check for loops in the compressed name; 
			// if we've looked at the whole message, there must be a loop.
			if (checked >= eom - msg) {
				errno = EMSGSIZE;
				return -1;
			}
			break;

		default:
			errno = EMSGSIZE;
			return -1;
		}
	}

	*dstp = '\0';
	if (len < 0)
		len = srcp - src;
	return len;
}

//
// dn_comp
//

int dn_comp(const char *src, unsigned char *dst, int dstsiz,
	    unsigned char **dnptrs, unsigned char **lastdnptr)
{
	unsigned char tmp[NS_MAXCDNAME];
	int rc;

	rc = ns_name_pton(src, tmp, sizeof tmp);
	if (rc < 0)
		return rc;

	return ns_name_pack(tmp, dst, dstsiz, dnptrs, lastdnptr);
}

//
// dn_expand
//

int dn_expand(const unsigned char *msg, const unsigned char *eom,
	      const unsigned char *src, char *dst, int dstsiz)
{
	unsigned char tmp[NS_MAXCDNAME];
	int n;
	int rc;

	n = ns_name_unpack(msg, eom, src, tmp, sizeof tmp);
	if (n < 0)
		return n;

	rc = ns_name_ntop(tmp, dst, dstsiz);
	if (rc < 0)
		return rc;

	if (n > 0 && dst[0] == '.')
		dst[0] = '\0';
	return n;
}


//
// res_randomid
//

static unsigned int res_randomid(void)
{
	struct timeval now;

	gettimeofday(&now, NULL);
	return (now.tv_sec ^ now.tv_usec ^ getpid()) & 0xFFFF;
}

//
// res_init
//

int res_init(void)
{
	char *addr;

	memset(&res, 0, sizeof(struct res_state));
	res.options = RES_DEFAULT;
	res.retry = RES_DFLRETRY;
	res.retrans = RES_TIMEOUT;
	res.id = res_randomid();
	res.ndots = 1;
	res.nscount = 0;

	/* FIXME: parse file */
#if 0
	if (peb->primary_dns.s_addr != INADDR_ANY) {
		res.nsaddr_list[res.nscount].sin_addr.s_addr =
		    peb->primary_dns.s_addr;
		res.nsaddr_list[res.nscount].sin_family = AF_INET;
		res.nsaddr_list[res.nscount].sin_port =
		    htons(NS_DEFAULTPORT);
		res.nscount++;
	}

	if (peb->secondary_dns.s_addr != INADDR_ANY) {
		res.nsaddr_list[res.nscount].sin_addr.s_addr =
		    peb->secondary_dns.s_addr;
		res.nsaddr_list[res.nscount].sin_family = AF_INET;
		res.nsaddr_list[res.nscount].sin_port =
		    htons(NS_DEFAULTPORT);
		res.nscount++;
	}

	addr = get_property(osconfig(), "dns", "primary", NULL);
	if (addr != NULL) {
		res.nsaddr_list[res.nscount].sin_addr.s_addr =
		    inet_addr(addr);
		res.nsaddr_list[res.nscount].sin_family = AF_INET;
		res.nsaddr_list[res.nscount].sin_port =
		    htons(NS_DEFAULTPORT);
		res.nscount++;
	}

	addr = get_property(osconfig(), "dns", "secondary", NULL);
	if (addr != NULL) {
		res.nsaddr_list[res.nscount].sin_addr.s_addr =
		    inet_addr(addr);
		res.nsaddr_list[res.nscount].sin_family = AF_INET;
		res.nsaddr_list[res.nscount].sin_port =
		    htons(NS_DEFAULTPORT);
		if (res.nsaddr_list[res.nscount].sin_addr.s_addr !=
		    INADDR_NONE)
			res.nscount++;
	}

	if (res.nscount == 0) {
		res.nsaddr_list[res.nscount].sin_addr.s_addr =
		    INADDR_LOOPBACK;
		res.nsaddr_list[res.nscount].sin_family = AF_INET;
		res.nsaddr_list[res.nscount].sin_port =
		    htons(NS_DEFAULTPORT);
		if (res.nsaddr_list[res.nscount].sin_addr.s_addr !=
		    INADDR_NONE)
			res.nscount++;
	}

	strcpy(res.defdname, peb->default_domain);
	if (!*res.defdname) {
		strcpy(res.defdname,
		       get_property(osconfig(), "dns", "domain",
				    "local.domain"));
	}
#endif

	res.dnsrch[0] = res.defdname;
	return 0;
}

//
// send_vc
//

static int send_vc(struct res_state *statp,
		   const unsigned char *buf, int buflen,
		   unsigned char *answer, int anslen, int *terrno, int ns)
{
	const struct dns_hdr *hp = (const struct dns_hdr *) buf;
	struct dns_hdr *anhp = (struct dns_hdr *) answer;
	struct sockaddr_in *nsap = &statp->nsaddr_list[ns];
	int resplen, n;
	unsigned short len;
	unsigned char *cp;
	int s;
	int rc;

	/* Need a timer on this */

	// Connect to name server
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		*terrno = s;
		return -1;
	}

	rc = connect(s, (struct sockaddr *) nsap, sizeof *nsap);
	if (rc < 0) {
		*terrno = rc;
		close(s);
		return 0;
	}

	// Send length & message
	len = htons((unsigned short) buflen);

	if (write(s, &len, 2) != 2 || (rc = write(s, buf, buflen)) != buflen) {
		*terrno = rc;
		close(s);
		return 0;
	}
	// Receive length & response
	cp = answer;
	len = sizeof(unsigned short);
	while ((n = recv(s, cp, len, 0)) > 0) {
		cp += n;
		if ((len -= n) <= 0)
			break;
	}

	if (n <= 0) {
		*terrno = n;
		close(s);
		return 0;
	}

	resplen = ntohs(*(unsigned short *) answer);
	if (resplen > anslen) {
		len = anslen;
	} else {
		len = resplen;
	}

	if (len < NS_HFIXEDSZ) {
		// Undersized message
		*terrno = -EMSGSIZE;
		close(s);
		return 0;
	}

	cp = answer;
	while (len != 0 && (n = recv(s, cp, len, 0)) > 0) {
		cp += n;
		len -= n;
	}

	if (n <= 0) {
		*terrno = n;
		close(s);
		return 0;
	}
	// All is well, or the error is fatal.
	// Signal that the next nameserver ought not be tried.

	close(s);
	return resplen;
}

//
// send_dg
//

static int send_dg(struct res_state *statp,
		   const unsigned char *buf, int buflen,
		   unsigned char *answer, int anslen,
		   int *terrno, int ns, int *v_circuit, int *gotsomewhere)
{
	const struct dns_hdr *hp = (const struct dns_hdr *) buf;
	struct dns_hdr *anhp = (struct dns_hdr *) answer;
	const struct sockaddr_in *nsap = &statp->nsaddr_list[ns];
	struct sockaddr_in from;
	//struct sockaddr_in local;
	int fromlen, resplen, timeout, s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		*terrno = s;
		return -1;
	}
	//local.sin_family = AF_INET;
	//local.sin_port = htons(1024);
	//local.sin_addr.s_addr = INADDR_ANY;

	if (connect(s, (struct sockaddr *) nsap, sizeof *nsap) < 0) {
		close(s);
		return 0;
	}

	if (send(s, (char *) buf, buflen, 0) != buflen) {
		close(s);
		return 0;
	}
	// Wait for reply.
	timeout = (statp->retrans << ns);
	if (ns > 0)
		timeout /= statp->nscount;
	if (timeout <= 0)
		timeout = 1;
	timeout = timeout * 1000;

	/* We need to use a timer instead */
#ifdef FIXME	
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(int));
#endif	

wait:
	fromlen = sizeof(struct sockaddr_in);
	resplen =
	    recvfrom(s, (char *) answer, anslen, 0,
		     (struct sockaddr *) &from, &fromlen);

	if (resplen == 0) {
		*gotsomewhere = 1;
		close(s);
		return 0;
	}

	if (resplen < 0) {
		if (errno == ETIMEDOUT) {
			*gotsomewhere = 1;
			close(s);
			return 0;
		}

		close(s);
		return 0;
	}

	*gotsomewhere = 1;

	if (resplen < NS_HFIXEDSZ) {
		// Undersized message
		*terrno = -EMSGSIZE;
		close(s);
		return 0;
	}

	if (hp->id != anhp->id) {
		// Response from old query, ignore it.
		goto wait;
	}

	if (!(statp->options & RES_IGNTC) && anhp->tc) {
		// To get the rest of answer, TCP with same server.
		*v_circuit = 1;
		close(s);
		return 1;
	}
	// All is well, or the error is fatal.  
	// Signal that the next nameserver ought not be tried.
	close(s);
	return resplen;
}

//
// res_nsend
//

static int res_nsend(struct res_state *statp, const char *buf, int buflen,
		     char *answer, int anslen)
{
	int gotsomewhere, try, v_circuit, resplen, ns, n, terrno;

	if (statp->nscount == 0) {
		errno = ESRCH;
		return -1;
	}
	if (anslen < NS_HFIXEDSZ) {
		errno = EINVAL;
		return -1;
	}

	v_circuit = (statp->options & RES_USEVC) || buflen > NS_PACKETSZ;
	gotsomewhere = 0;
	terrno = -ETIMEDOUT;

	// FIXME: add dns cache lookup here

	// Some resolvers want to even out the load on their nameservers.
	if ((statp->options & RES_ROTATE) != 0) {
		struct sockaddr_in ina;
		int lastns = statp->nscount - 1;

		memcpy(&ina, &statp->nsaddr_list[0], sizeof(ina));
		for (ns = 0; ns < lastns; ns++) {
			memcpy(&statp->nsaddr_list[ns],
			    &statp->nsaddr_list[ns + 1], sizeof(struct sockaddr_in));
		}
		memcpy(&statp->nsaddr_list[lastns], &ina, sizeof(ina));
	}
	// Send request, RETRY times, or until successful.
	for (try = 0; try < statp->retry; try++) {
		for (ns = 0; ns < statp->nscount; ns++) {
			struct sockaddr_in *nsap = &statp->nsaddr_list[ns];

		      same_ns:
			if (v_circuit) {
				// Use VC; at most one attempt per server.
				try = statp->retry;
				n = send_vc(statp, buf, buflen, answer,
					    anslen, &terrno, ns);
				if (n < 0) {
					errno = -terrno;
					return -1;
				}
				if (n == 0)
					break;
				resplen = n;
			} else {
				// Use datagrams.
				n = send_dg(statp, buf, buflen, answer,
					    anslen, &terrno, ns,
					    &v_circuit, &gotsomewhere);
				if (n < 0) {
					errno = -terrno;
					return -1;
				}
				if (n == 0)
					break;
				if (v_circuit)
					goto same_ns;
				resplen = n;
			}

			return resplen;
		}
	}

	if (!v_circuit) {
		if (!gotsomewhere) {
			errno = ECONNREFUSED;	// No nameservers found
			return -1;
		} else {
			errno = ETIMEDOUT;	// No answer obtained
			return -1;
		}
	} else {
		errno = -terrno;
		return -1;
	}
}

//
// res_send
//

int res_send(const char *buf, int buflen, char *answer, int anslen)
{
	return res_nsend(&res, buf, buflen, answer, anslen);
}

//
// res_nquery
//

static int res_nquery(struct res_state *statp, const char *dname,
		      int class, int type, unsigned char *answer,
		      int anslen)
{
	/* Big.. ugly, FIXME and brk this */
	unsigned char buf[QUERYBUF_SIZE];
	struct dns_hdr *hp = (struct dns_hdr *) answer;
	int n;

	hp->rcode = 0;

	n = res_nmkquery(statp, DNS_OP_QUERY, dname, class, type, NULL, 0,
			 NULL, buf, sizeof(buf));
	if (n <= 0)
		return n;

	n = res_nsend(statp, buf, n, answer, anslen);
	if (n < 0)
		return n;

	if (hp->rcode != 0 || ntohs(hp->ancount) == 0)
		return -1;

	return n;
}

//
// res_query
//

int res_query(const char *dname, int class, int type,
	      unsigned char *answer, int anslen)
{
	return res_nquery(&res, dname, class, type, answer, anslen);
}

//
// res_nsearch
//

static int res_nsearch(struct res_state *statp, const char *name,
		       int class, int type, unsigned char *answer,
		       int anslen)
{
	const char *cp;
	char **domain;
	struct dns_hdr *hp = (struct dns_hdr *) answer;
	int dots;
	int trailing_dot, rc, saved_rc;
	int got_nodata = 0, got_servfail = 0, root_on_list = 0;
	int tried_as_is = 0;

	dots = 0;
	for (cp = name; *cp != '\0'; cp++)
		dots += (*cp == '.');
	trailing_dot = 0;
	if (cp > name && *--cp == '.')
		trailing_dot++;

	// If there are enough dots in the name, let's just give it a
	// try 'as is'. The threshold can be set with the "ndots" option.
	// Also, query 'as is', if there is a trailing dot in the name.

	saved_rc = 0;
	if (dots >= statp->ndots || trailing_dot) {
		rc = res_nquerydomain(statp, name, NULL, class, type,
				      answer, anslen);
		if (rc > 0 || trailing_dot)
			return rc;
		saved_rc = rc;
		tried_as_is++;
	}
	// We do at least one level of search if
	// - there is no dot and RES_DEFNAME is set, or
	// - there is at least one dot, there is no trailing dot,
	//   and RES_DNSRCH is set.

	if ((!dots && (statp->options & RES_DEFNAMES) != 0) ||
	    (dots && !trailing_dot
	     && (statp->options & RES_DNSRCH) != 0)) {
		int done = 0;

		for (domain = statp->dnsrch; *domain && !done; domain++) {

			if (domain[0][0] == '\0'
			    || (domain[0][0] == '.'
				&& domain[0][1] == '\0')) {
				root_on_list++;
			}

			rc = res_nquerydomain(statp, name,
					      (const char *) *domain,
					      class, type, answer, anslen);
			if (rc > 0)
				return rc;

			if (errno == ECONNREFUSED)
				return -1;

			switch (errno) {
			case ETIMEDOUT:
				got_nodata++;
				// FALLTHROUGH

			case EHOSTUNREACH:
			case ENETUNREACH:
				// Keep trying
				break;

			case EAGAIN:
				if (hp->rcode == DNS_ERR_SERVFAIL) {
					// Try next search element, if any
					got_servfail++;
					break;
				}
				// FALLTHROUGH

			default:
				// Anything else implies that we're done
				done++;
			}

			// If we got here for some reason other than DNSRCH,
			// we only wanted one iteration of the loop, so stop.
			if ((statp->options & RES_DNSRCH) == 0)
				done++;
		}
	}
	// If the name has any dots at all, and no earlier 'as-is' query
	// for the name, and "." is not on the search list, then try an as-is
	// query now.
	if (statp->ndots && !(tried_as_is || root_on_list)) {
		rc = res_nquerydomain(statp, name, NULL, class, type,
				      answer, anslen);
		if (rc > 0)
			return rc;
	}
	// If we got here, we didn't satisfy the search.
	// If we did an initial full query, return that query's return code
	// (note that we wouldn't be here if that query had succeeded).
	// else if we ever got a nodata, send that back as the reason.
	// else send back meaningless error code, that being the one from
	// the last DNSRCH we did.
	if (saved_rc != 0) {
		return saved_rc;
	} else if (got_nodata) {
		errno = ETIMEDOUT;
		return -1;
	} else if (got_servfail) {
		errno = EAGAIN;
		return -1;
	} else {
		errno = EIO;
		return -1;	// ???
	}
}

//
// res_search
//

int res_search(const char *name, int class, int type,
	       unsigned char *answer, int anslen)
{
	return res_nsearch(&res, name, class, type, answer, anslen);
}

//
// res_nquerydomain
//
// Perform a call on res_query on the concatenation of name and domain,
// removing a trailing dot from name if domain is NULL.
//

static int res_nquerydomain(struct res_state *statp, const char *name,
			    const char *domain, int class, int type,
			    unsigned char *answer, int anslen)
{
	char nbuf[NS_MAXDNAME];
	const char *longname = nbuf;
	int n, d;

	if (domain == NULL) {
		// Check for trailing '.'; copy without '.' if present.
		n = strlen(name);
		if (n >= NS_MAXDNAME) {
			errno = EMSGSIZE;
			return -1;
		}

		n--;
		if (n >= 0 && name[n] == '.') {
			strncpy(nbuf, name, n);
			nbuf[n] = '\0';
		} else {
			longname = name;
		}
	} else {
		n = strlen(name);
		d = strlen(domain);
		if (n + d + 1 >= NS_MAXDNAME) {
			errno = EMSGSIZE;
			return -1;
		}
		sprintf(nbuf, "%s.%s", name, domain);
	}

	return res_nquery(statp, longname, class, type, answer, anslen);
}

//
// res_querydomain
//

int res_querydomain(const char *name, const char *domain, int class,
		    int type, unsigned char *answer, int anslen)
{
	return res_nquerydomain(&res, name, domain, class, type, answer,
				anslen);
}

//
// res_nmkquery
//

static int res_nmkquery(struct res_state *statp, int op, const char *dname,
			int class, int type,
			const unsigned char *data, int datalen,
			unsigned char *newrr,
			unsigned char *buf, int buflen)
{
	struct dns_hdr *hp;
	unsigned char *cp;
	int n;
	unsigned char *dnptrs[20], **dpp, **lastdnptr;

	// Initialize header fields.
	if (buf == NULL || buflen < NS_HFIXEDSZ) {
		errno = EINVAL;
		return -1;
	}

	memset(buf, 0, NS_HFIXEDSZ);
	hp = (struct dns_hdr *) buf;
	hp->id = htons(++statp->id);
	hp->opcode = op;
	hp->rd = (statp->options & RES_RECURSE) != 0;
	hp->rcode = 0;
	cp = ((unsigned char *) buf) + NS_HFIXEDSZ;
	buflen -= NS_HFIXEDSZ;
	dpp = dnptrs;
	*dpp++ = (unsigned char *) buf;
	*dpp++ = NULL;
	lastdnptr = dnptrs + sizeof dnptrs / sizeof dnptrs[0];

	// Perform opcode specific processing
	switch (op) {
	case DNS_OP_QUERY:
	case DNS_OP_NOTIFY:
		if ((buflen -= NS_QFIXEDSZ) < 0) {
			errno = EMSGSIZE;
			return -1;
		}

		if ((n =
		     dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0) {
			errno = EMSGSIZE;
			return -1;
		}

		cp += n;
		buflen -= n;

		*(unsigned short *) cp = htons((unsigned short) type);
		cp += sizeof(unsigned short);

		*(unsigned short *) cp = htons((unsigned short) class);
		cp += sizeof(unsigned short);

		hp->qdcount = htons(1);
		if (op == DNS_OP_QUERY || data == NULL)
			break;

		// Make an additional record for completion domain
		buflen -= NS_RRFIXEDSZ;
		n = dn_comp((const char *) data, cp, buflen, dnptrs,
			    lastdnptr);
		if (n < 0) {
			errno = EMSGSIZE;
			return -1;
		}
		cp += n;
		buflen -= n;

		*(unsigned short *) cp = htons(DNS_TYPE_NULL);
		cp += sizeof(unsigned short);

		*(unsigned short *) cp = htons((unsigned short) class);
		cp += sizeof(unsigned short);

		*(unsigned long *) cp = 0;
		cp += sizeof(unsigned long);

		*(unsigned short *) cp = 0;
		cp += sizeof(unsigned short);

		hp->arcount = htons(1);
		break;

	case DNS_OP_IQUERY:
		// Initialize answer section
		if (buflen < 1 + NS_RRFIXEDSZ + datalen) {
			errno = EMSGSIZE;
			return -1;
		}
		*cp++ = '\0';	// No domain name

		*(unsigned short *) cp = htons((unsigned short) type);
		cp += sizeof(unsigned short);

		*(unsigned short *) cp = htons((unsigned short) class);
		cp += sizeof(unsigned short);

		*(unsigned long *) cp = 0;
		cp += sizeof(unsigned long);

		*(unsigned short *) cp = htons((unsigned short) datalen);
		cp += sizeof(unsigned short);

		if (datalen) {
			memcpy(cp, data, datalen);
			cp += datalen;
		}

		hp->ancount = htons(1);
		break;

	default:
		errno = ENOSYS;
		return -1;
	}

	return cp - buf;
}

//
// res_mkquery
//

int res_mkquery(int op, const char *dname, int class, int type, char *data,
		int datalen, unsigned char *newrr, char *buf, int buflen)
{
	return res_nmkquery(&res, op, dname, class, type, data, datalen,
			    newrr, buf, buflen);
}
