#ifndef __AR_H
#define __AR_H

#define ARMAG "!<arch>\n"
#define SARMAG 8
#define ARFMAG "`\n"

struct ar_hdr {
	char	ar_name[16],
		ar_date[12],
		ar_uid[6],
		ar_gid[6],
		ar_mode[8],
		ar_size[10],
		ar_fmag[2];
};

#endif /* __AR_H */
