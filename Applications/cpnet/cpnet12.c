/*************************************************************************

    This file is part of the CP/NET 1.1/1.2 server emulator for Unix
    systems. Copyright (C) 2005, Hector Peraza.
    
    This module emulates CP/NET 1.2 functions.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

#include "main.h"
#include "cpmutl.h"
#include "netio.h"
#include "cpnet12.h"

extern char *fn_name[];

static unsigned char buf[256];

int cpnet_12(void)
{
	int sid, fnc, len;
/*	int first_connect; */
	DIR *dirp = NULL;
	struct cpmfcb search_fcb, *last_search;
	struct cpmdpb dpb;

/*	first_connect = 0; */

	/* fake DPB for a hypothetical 32M disk */
	dpb.sptl = 1024 & 0xFF;
	dpb.spth = 1024 >> 8;
	dpb.bsh = 4;		/* bls = 2048 */
	dpb.blm = 15;
	dpb.exm = 0;		/* 128 records per extent */
	dpb.dsml = 16383 & 0xFF;
	dpb.dsmh = 16384 >> 8;
	dpb.drml = 1023 & 0xFF;
	dpb.drmh = 1023 >> 8;
	dpb.al0 = 0xfe;
	dpb.al1 = 0x00;
	dpb.cksl = 0;		/* non-removable media, but we don't really care */
	dpb.cksh = 0;
	dpb.offl = 0;
	dpb.offh = 0;

	for (;;) {
		wait_for_packet();

		if (get_packet(buf, &len, &fnc, &sid) != 0)
			continue;
#ifdef DEBUG
		if (_debug & DEBUG_DATA) {
			char *fname;
			if (fnc <= 40)
				fname = fn_name[fnc];
			else if (fnc >= 64 && fnc <= 69)
				fname = fn_name[fnc - (64 - 41)];
			else
				fname = "";
			printf("Requested function %d (%02Xh): %s\n", fnc, (unsigned char) fnc, fname);
			dump_data(buf, len);
		}
#endif

		switch (fnc) {
		case 0:	/* system reset */
		case 1:	/* console input */
		case 2:	/* console output */
			send_error(sid, 0);
			break;

		case 3:	/* raw console input */
			if (_logged_in) {
				/* buf[0] held console number */
				buf[0] = 0x1a;
				send_packet(sid, 3, buf, 1);
			} else {
				send_error(sid, 3);
			}
			break;

		case 4:	/* raw console output */
			if (_logged_in) {
				/* buf[0] held console number */
				write(1, buf + 1, 1);
				send_ok(sid, 4);
			} else {
				send_error(sid, 4);
			}
			break;

		case 5:	/* list output */
			send_error(sid, 5);
			break;
#ifdef TODO_PRINTER
			if (_logged_in) {
				int printer = buf[0];
				if (lst_output(printer, &buf[1], len - 1) == 0)
					send_ok(sid, 5);
				else
					send_error(sid, 5);
			} else {
				send_error(sid, 5);
			}
			break;
#endif

		case 6:	/* direct console I/O */
		case 7:	/* get I/O byte */
		case 8:	/* set I/O byte */
		case 9:	/* print string */
		case 10:	/* read console buffer */
			send_error(sid, 0);
			break;

		case 11:	/* get console status */
			if (_logged_in) {
				/* buf[0] held console number */
				buf[0] = 0x00;
				send_packet(sid, 11, buf, 1);
			} else {
				send_error(sid, 11);
			}
			break;

		case 12:	/* get version number */
		case 13:	/* reset disk system */
			send_error(sid, 0);
			break;

		case 14:	/* select disk */
			if (_logged_in) {
				int disk = buf[0];
				if (disk_to_dir[disk] != NULL) {
					/* TODO: mark disk as logged-in? */
					send_ok(sid, 14);
				} else {
					send_error(sid, 14);
				}
			} else {
				send_error(sid, 14);
			}
			break;

		case 15:	/* open file */
			if (_logged_in) {
				struct cpmfcb fcb;
				int fd, dirc;
				dirc = 0;
				/*userno = buf[0]; */
				/* TODO: validate user number, check password? */
				memcpy((void *) &fcb, (void *) &buf[1], 13);
				/* close search */
				if (dirp) {
					closedir(dirp);
					dirp = NULL;
				}
				/* select fcb.drive */
				if (fcb.drive > 0) {
					if (goto_drive(fcb.drive - 1) != 0)
						dirc = 0xff;
				}
				if (dirc == 0) {
					fd = open(getname(&fcb), O_RDWR);
					if (fd < 0) {
						dirc = 0xff;
					} else {
						int *ip;
						fcb.s1 = 0x80;
						fcb.s2 = 0;
						fcb.rc = 0;
						fcb.cr = 0;
						ip = (int *) fcb.dmap;
						*ip = fd;
						dirc = 0;
					}
				}
				buf[0] = dirc;
				memcpy((void *) &buf[1], (void *) &fcb, 36);
				send_packet(sid, 15, buf, 37);
			} else {
				/* not logged in */
				send_error(sid, 15);
			}
			break;

		case 16:	/* close file */
			if (_logged_in) {
				int *ip, fd = 0;
				struct cpmfcb fcb;
				/*user = buf[0]; */
				memcpy((void *) &fcb, (void *) &buf[1], 36);
				ip = (int *) fcb.dmap;
				fd = *ip;
				if (close(fd) < 0) {
					send_error(sid, 16);
				} else {
					buf[0] = 0;	/* dirc */
					memcpy((void *) &buf[1], (void *) &fcb, 36);
					send_packet(sid, 16, buf, 37);
				}
			} else {
				send_error(sid, 16);
			}
			break;

		case 17:	/* search first */
			if (_logged_in) {
				int cdisk, userno;
				cdisk = buf[0];
				userno = buf[1];
				memcpy((void *) &search_fcb, (void *) &buf[2], 36);
				if (search_fcb.drive == '?')
					search_fcb.drive = cdisk;
				/* TODO: handle case of any-user (fcb.drive is '?') */
				if (search_fcb.drive > 0) {
					if (goto_drive(search_fcb.drive - 1) != 0) {
						buf[0] = 0xff;
						memset((void *) &buf[1], 0, 32);
						send_packet(sid, 17, buf, 33);
						break;
					}
				}
				/* TODO: check for user number */
				if (dirp)
					closedir(dirp);
				dirp = opendir(".");
				last_search = get_dir_entry(dirp, &search_fcb, 1);
				if (last_search) {
					buf[0] = 0;
					last_search->drive = userno;
					memcpy((void *) &buf[1], (void *) last_search, 32);
				} else {
					buf[0] = 0xff;
					memset((void *) &buf[1], 0xe5, 32);
				}
				send_packet(sid, 17, buf, 33);
			} else {
				send_error(sid, 17);
			}
			break;

		case 18:	/* search next */
			if (_logged_in) {
				int userno = buf[1];
				/* TODO: check for valid user number */
				last_search = get_dir_entry(dirp, &search_fcb, 0);
				if (last_search) {
					buf[0] = 0;
					last_search->drive = userno;
					memcpy((void *) &buf[1], (void *) last_search, 32);
				} else {
					buf[0] = 0xff;
					memset((void *) &buf[1], 0xe5, 32);
				}
				send_packet(sid, 18, buf, 33);
			} else {
				send_error(sid, 18);
			}
			break;

		case 19:	/* delete file */
			if (_logged_in) {
				int retc;/* userno = buf[0]; */
				struct cpmfcb fcb;
				/* buf+1 = first 13 bytes of FCB containing drive fname ext ex */
				/* The FCB may contain the '?' meta-character */
				memcpy((void *) &fcb, (void *) &buf[1], 13);
				retc = 0;
				if (fcb.drive > 0) {
					if (goto_drive(fcb.drive - 1) != 0)
						retc = 0xff;
				}
				if (retc == 0)
					retc = delete_files(&fcb);
				buf[0] = retc;
				send_packet(sid, 19, buf, 1);
			} else {
				send_error(sid, 19);
			}
			break;

		case 20:	/* read sequential */
			if (_logged_in) {
				int n, *ip, fd = 0;
				unsigned long fpos;
				struct cpmfcb fcb;

				/*user = buf[0]; */
				memcpy((void *) &fcb, (void *) &buf[1], 36);
				ip = (int *) fcb.dmap;
				fd = *ip;	/* TODO: check for valid fd? */
				fpos = (fcb.ex * 256L + fcb.cr) * 128L;
				lseek(fd, fpos, SEEK_SET);
				n = read(fd, &buf[37], 128);
				if (n < 0) {
					buf[0] = 0xff;
				} else if (n == 0) {
					buf[0] = 1;	/* CP/M EOF on read */
				} else {
					if (n < 128) {
						for (; n < 128; ++n)
							buf[n + 37] = 0x1a;
					}
					++fcb.cr;
					if (fcb.cr == 0)
						++fcb.ex;
					fcb.r0 = fcb.cr;
					fcb.r1 = fcb.ex;
					fcb.r2 = fcb.rc;
					buf[0] = 0;
				}
				memcpy((void *) &buf[1], (void *) &fcb, 36);
				send_packet(sid, 20, buf, 165);
			} else {
				send_error(sid, 20);
			}
			break;

		case 21:	/* write sequential */
			if (_logged_in) {
				int n, *ip, fd = 0;
				unsigned long fpos;
				struct cpmfcb fcb;

				/* user = buf[0]; */
				memcpy((void *) &fcb, (void *) &buf[1], 36);
				ip = (int *) fcb.dmap;
				fd = *ip;	/* TODO: check for valid fd? */
				fpos = (fcb.ex * 256L + fcb.cr) * 128L;
				lseek(fd, fpos, SEEK_SET);
				n = write(fd, &buf[37], 128);
				if (n < 128) {
					buf[0] = 0xff;
				} else {
					++fcb.cr;
					if (fcb.cr == 0)
						++fcb.ex;
					fcb.r0 = fcb.cr;
					fcb.r1 = fcb.ex;
					fcb.r2 = fcb.rc;
					buf[0] = 0;
				}
				memcpy((void *) &buf[1], (void *) &fcb, 36);
				send_packet(sid, 21, buf, 37);
			} else {
				send_error(sid, 21);
			}
			break;

		case 22:	/* create file */
			if (_logged_in) {
				struct cpmfcb fcb;
				int fd, dirc;
				/* userno = buf[0]; */
				memcpy((void *) &fcb, (void *) &buf[1], 13);
				/* close search */
				if (dirp) {
					closedir(dirp);
					dirp = NULL;
				}
				dirc = 0;
				if (fcb.drive > 0) {
					if (goto_drive(fcb.drive - 1) != 0)
						dirc = 0xff;
				}
				/* TODO: check for existing file? */
				if (dirc == 0) {
					fd = open(getname(&fcb), O_RDWR | O_CREAT, 0644);	// O_TRUNC ?
					if (fd < 0) {
						dirc = 0xff;
					} else {
						int *ip;
						fcb.s1 = 0x80;
						fcb.s2 = 0;
						fcb.rc = 0;
						fcb.cr = 0;
						ip = (int *) fcb.dmap;
						*ip = fd;
						dirc = 0;
					}
				}
				buf[0] = dirc;
				memcpy((void *) &buf[1], (void *) &fcb, 36);
				send_packet(sid, 22, buf, 37);
			} else {
				send_error(sid, 22);
			}
			break;

		case 23:	/* rename file */
			if (_logged_in) {
				int retc; /*, userno = buf[0]; */
				struct cpmfcb oldfcb, newfcb;
				/* buf+1  = first 16 bytes of FCB containing drive old-name ext ? ? ? ? */
				/* buf+17 = first 16 bytes of FCB containing drive new-name ext ? ? ? ? */
				memcpy((void *) &oldfcb, (void *) &buf[1], 16);
				memcpy((void *) &newfcb, (void *) &buf[17], 16);
				retc = 0;
				if (oldfcb.drive > 0) {
					if (goto_drive(oldfcb.drive - 1) != 0)
						retc = 0xff;
				}
				if (retc == 0) {
					/* our getname() uses a static buffer, so we'd better make
					   a copy of the strings */
					char oldname[20], newname[20];
					strcpy(oldname, getname(&oldfcb));
					strcpy(newname, getname(&newfcb));
					if (rename(oldname, newname))
						retc = 0xff;
				}
				buf[0] = retc;
				send_packet(sid, 23, buf, 1);
			} else {
				send_error(sid, 23);
			}
			break;

		case 24:	/* get login vector */
			if (_logged_in) {
				/* TODO: fix this! */
				buf[0] = 0x01;	/* only drive A: logged in */
				buf[1] = 0x00;
				send_packet(sid, 24, buf, 2);
			} else {
				send_error(sid, 24);	/* check NDOS! */
			}
			break;

		case 25:	/* get current disk */
		case 26:	/* set DMA address */
			send_error(sid, 0);
			break;

		case 27:	/* get allocation vector */
			if (_logged_in) {
/*				int cdisk;
				cdisk = buf[0]; */
				/* TODO: check for valid disk */
				update_allocv();
				send_packet(sid, 27, allocv, 256);
			} else {
				send_error(sid, 27);	/* check NDOS for this case */
			}
			break;

		case 28:	/* write protect disk */
			send_ok(sid, 28);	/* TODO: implement this */
			break;

		case 29:	/* get R/O vector */
			if (_logged_in) {
				buf[0] = 0x00;	/* no R/O disks */
				buf[1] = 0x00;
				send_packet(sid, 29, buf, 2);
			} else {
				send_error(sid, 29);	/* check NDOS! */
			}
			break;

		case 30:	/* set file attributes */
			send_ok(sid, 30);	/* just ignore? */
			break;

		case 31:{	/* get DPB */
/*				int disk = buf[0];*/	/* A: = 0 ... P: = 15 */
				memcpy((void *) &buf, (void *) &dpb, 15);
				/* there is no possible error return for this (CP/NET 1.1) */
				send_packet(sid, 31, buf, 15);
				break;
			}

		case 32:	/* get/set user code */
			send_error(sid, 0);
			break;

		case 33:	/* read random */
			if (_logged_in) {
				int n, *ip, fd = 0;
				unsigned long fpos;
				struct cpmfcb fcb;

				/*userno = buf[0]; */
				memcpy((void *) &fcb, (void *) &buf[1], 36);
				ip = (int *) fcb.dmap;
				fd = *ip;	/* TODO: check for valid fd? */
				/* note that read random does not advance the sequential pointers */
				fcb.cr = fcb.r0;
				fcb.ex = fcb.r1;
				fcb.rc = fcb.r2;
				fpos = (fcb.r1 * 256L + fcb.r0) * 128L;
				lseek(fd, fpos, SEEK_SET);
				n = read(fd, &buf[37], 128);
				if (n < 0) {
					buf[0] = 0xff;
				} else if (n == 0) {
					buf[0] = 1;	/* CP/M EOF on read */
				} else {
					if (n < 128) {
						for (; n < 128; ++n)
							buf[n + 37] = 0x1a;
					}
					buf[0] = 0;
				}
				memcpy((void *) &buf[1], (void *) &fcb, 36);
				send_packet(sid, 33, buf, 165);
			} else {
				send_error(sid, 33);
			}
			break;

		case 40:	/* write random with zero fill *//* TODO: check this! */
		case 34:	/* write random */
			if (_logged_in) {
				int n, *ip, fd = 0;
				unsigned long fpos;
				struct cpmfcb fcb;

				/* userno = buf[0]; */
				memcpy((void *) &fcb, (void *) &buf[1], 36);
				ip = (int *) fcb.dmap;
				fd = *ip;	/* TODO: check for valid fd? */
				fcb.cr = fcb.r0;
				fcb.ex = fcb.r1;
				fcb.rc = fcb.r2;
				/* note that write random does not advance the sequential pointers */
				fpos = (fcb.r1 * 256L + fcb.r0) * 128L;
				lseek(fd, fpos, SEEK_SET);
				n = write(fd, &buf[37], 128);
				if (n < 128) {
					buf[0] = 0xff;
				} else {
					buf[0] = 0;
				}
				memcpy((void *) &buf[1], (void *) &fcb, 36);
				send_packet(sid, 34, buf, 37);
			} else {
				send_error(sid, 34);
			}
			break;

		case 35:	/* compute file size */
			if (_logged_in) {
				struct cpmfcb fcb;
				int retc;
				unsigned long nrec;
				struct stat stbuf;
				/* userno = buf[0]; */
				memcpy((void *) &fcb, (void *) &buf[1], 13);
				retc = 0;
				if (fcb.drive > 0) {
					if (goto_drive(fcb.drive - 1) != 0)
						retc = 0xff;
				}
				if (retc == 0)
					retc = stat(getname(&fcb), &stbuf);
				if (retc == 0) {
					nrec = (stbuf.st_size + 127) / 128;
					fcb.r0 = nrec & 0xff;
					fcb.r1 = (nrec >> 8) & 0xff;
					fcb.r2 = (nrec >> 16) & 0xff;
				} else {
					fcb.r0 = 0;
					fcb.r1 = 0;
					fcb.r2 = 0;
				}
				memcpy((void *) &buf[1], (void *) &fcb, 36);
				send_packet(sid, 35, buf, 37);
			} else {
				send_error(sid, 35);
			}
			break;

		case 36:	/* set random record */
			if (_logged_in) {
				/* int *ip; */
				struct cpmfcb fcb;
				/* userno = buf[0]; */
				memcpy((void *) &fcb, (void *) &buf[1], 36);
				/*ip = (int *) fcb.dmap; */
				/* fd = *ip; */
				/* TODO: check for a valid open file */
				fcb.r0 = fcb.cr;
				fcb.r1 = fcb.ex;
				fcb.r2 = fcb.rc;
				buf[0] = 0;
				memcpy((void *) &buf[1], (void *) &fcb, 36);
				send_packet(sid, 36, buf, 37);
			} else {
				send_error(sid, 36);
			}
			break;

		case 37:	/* reset drive */

			/*
			   The Reset Drive function is used to programmatically restore
			   specified drives to the reset state (a reset drive is not logged-in
			   and is in read/write status). The passed parameter in register pair
			   DE is a 16 bit vector of drives to be reset, where the least
			   significant bit corresponds to the first drive A: and the high-order
			   bit corresponds to the sixteenth drive, labelled P:. Bit values of
			   "1" indicate that the specified drive is to be reset.

			   This function is conditional under MP/M II. If another process
			   has a file open on a drive to be reset, and the drive is removeable
			   or read/only, the Drive Reset function is denied and no drives are
			   reset.

			   Upon return, if the reset operation is successful, register A
			   is set to zero. Otherwise, register A is set to 0FFH (255 decimal).
			   If the BDOS error mode is not Return Error mode (see Function 45),
			   then an error message is displayed at the console, identifying a
			   process owning an open file.
			 */

			send_ok(sid, 37);
			break;

		case 38:	/* access drive */

			/*
			   The Access Drive function inserts a special open file item into
			   the system lock list for each specified drive. While the item
			   exists in the lock list, the drive cannot be reset by another
			   process. As in Function 37, the calling process passes the drive
			   vector in register pair DE. The format of the drive vector is the
			   same as that used in Function 37.

			   The Access Drive function inserts no items if insufficient free
			   space exists in the lock list to support all the new items or if the
			   number of items to be inserted puts the calling process over the
			   lock list open file maximum. This maximum is a MP/M II Gensys
			   option. If the BDOS error mode is the default mode (see Function
			   45), a message identifying the error is displayed at the console and
			   the calling process is terminated. Otherwise, the Access Drive
			   function returns to the calling process, register A is set to 0FFH
			   and register H is set to one of the following values.

			   10: Process Open File limit exceeded
			   11: No room in the system lock list

			   Register A is set to zero if the Access Drive function is
			   successful.
			 */

			send_ok(sid, 38);
			break;

		case 39:	/* free drive */
			/* do we have to do anything here? */

			/*
			   The Free Drive function purges the open lock list of all file
			   and locked record items that belong to the calling process on the
			   specified drives. As in Function 38, the calling process passes the
			   drive vector in register pair DE.

			   Function 39 does not close files associated with purged open
			   file lock list items. In addition, if a process references a
			   "purged" file with a BDOS function requiring an open FCB, a checksum
			   error is returned. A file that has been written to should be closed
			   before making a Free Drive call to the file's drive, otherwise data
			   may be lost.
			 */

			send_ok(sid, 39);
			break;

		case 43:	/* unlock record */
			break;

		case 45:	/* set BDOS error mode */
			send_error(sid, 45);
			break;

		case 64:	/* login */
			if (strncmp((char *)buf, _passwd, 8) == 0) {
				_logged_in = 1;
/*				first_connect = 1; */
				send_ok(sid, 64);
#ifdef DEBUG
				if (_debug & DEBUG_MISC)
					printf("requester %d logged in\n", sid);
#endif
			} else {
				send_error(sid, 14);
#ifdef DEBUG
				if (_debug & DEBUG_MISC)
					printf("requester %d login denied, bad password\n", sid);
#endif
			}
			break;

		case 65:	/* logoff */
			if (_logged_in) {
				_logged_in = 0;
				send_ok(sid, 65);
#ifdef DEBUG
				if (_debug & DEBUG_MISC)
					printf("requester %d logged out\n", sid);
#endif
			} else {
				send_error(sid, 65);
			}
			break;

		case 66:	/* send message on network - not used by CP/NET 1.2? */
			send_error(sid, 0);
			break;

		case 67:	/* receive message on network - not used by CP/NET 1.2? */
			send_error(sid, 0);
			break;

		case 68:	/* get network status */
		case 69:	/* get config table address */
			send_error(sid, 0);
			break;

		case 70:	/* set compatibility attributes */
			send_error(sid, 70);
			break;

		case 71:	/* return server configuration */
			/* TODO: implement this */
			send_error(sid, 71);
			break;

		case 106:	/* set default password */
			send_error(sid, 106);
			break;

		default:
			send_error(sid, 0);
			break;
		}
	}

	return 0;
}
