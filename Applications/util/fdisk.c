/*
 *  fdisk.c  Disk partitioning program.
 *  Copyright (C) 1997  David Murn
 *
 *  This program is distributed under the GNU General Public Licence, and
 *  as such, may be freely distributed.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "fdisk.h"

#define isxdigit(x)	( ((x)>='0' && (x)<='9') || ((x)>='A' && (x)<='F') \
						 || ((x)>='a' && (x)<='f') )

#define die { printf("Usage %s [-l] <device-name>\n",argv[0]); fflush(stdout); exit(1); }
#define spc ((unsigned long)(geometry.heads*geometry.sectors))

static Funcs funcs[] = {
    { 'a',"Add partion",           add_part },
    { 'b',"Set bootable flag",     set_boot },
    { 'd',"Delete partition",      del_part },
    { 'l',"List partition types",  list_types },
    { 'p',"Print partition table", list_part },
    { 'q',"Quit fdisk",            quit },
    { 't',"Set partition type",    set_type },
    { 'w',"Write partition table", write_out },
    { '?',"Help",                  help },
    {  0,  NULL,                   NULL }
};

/* Table indexing the valid partition types.
 *
 * Any value of 'Y' indicates a valid partition type, and
 * any other value says otherwise.
 *
 * The types in this list are those recognised by the fdisk
 * command that is distributed with Red Hat Linux 6.2.
 */

const char partOK[257] =     /*	'0123456789ABCDEF'		*/

				"YYYYYYYYYYYYY.YY"	/*  0x	*/
				"YYY.Y.YYY..YY.Y."	/*  1x	*/
				"....Y..........."	/*  2x	*/
				"............Y..."	/*  3x	*/
				"YYY..........YYY"	/*  4x	*/
				"YYYYYYY.....Y..."	/*  5x	*/
				".Y.YYY.........."	/*  6x	*/
				"Y....Y.........."	/*  7x	*/
				"YYYYYYYY........"	/*  8x	*/
				"...YY..........."	/*  9x	*/
				"Y....YYY........"	/*  Ax	*/
				".......YY......."	/*  Bx	*/
				".Y..Y.YY........"	/*  Cx	*/
				"...........Y...."	/*  Dx	*/
				".Y.YY......Y...."	/*  Ex	*/
				".YY.Y........YYY"	/*  Fx	*/
;

#define valid(n)	(partOK[n] == 'Y')

void list_types(void)	/* FIXME - Should make this more flexible */
{
    printf(
	" 0 Empty                  3c PartitionMagic recovery 85 Linux extended\n"
	" 1 FAT12                  40 Venix 80286             86 NTFS volume set\n"
	" 2 XENIX root             41 PPC PReP Boot           87 NTFS volume set\n"
	" 3 XENIX usr              42 SFS                     93 Amoeba\n"
	" 4 FAT16 <32M             4d QNX4.x                  94 Amoeba BBT\n"
	" 5 Extended               4e QNX4.x 2nd part         a0 IBM Thinkpad hibernate\n"
	" 6 FAT16                  4f QNX4.x 3rd part         a5 BSD/386\n"
	" 7 HPFS/NTFS              50 OnTrack DM              a6 OpenBSD\n"
	" 8 AIX                    51 OnTrack DM6 Aux1        a7 NeXTSTEP\n"
	" 9 AIX bootable           52 CP/M                    b7 BSDI fs\n"
	" a OS/2 Boot Manager      53 OnTrack DM6 Aux3        b8 BSDI swap\n"
	" b Win9x FAT32            54 OnTrack DM6             c1 DR-DOS/sec FAT-12\n"
	" c Win9x FAT32 (LBA)      55 EZ-Drive                c4 DR-DOS/sec FAT-16 <32M\n"
	" e Win9x FAT16 (LBA)      56 Golden Bow              c6 DR-DOS/sec FAT-16\n"
	" f Win9x Extended (LBA)   5c Priam Edisk             c7 Syrinx\n"
	"10 OPUS                   61 SpeedStor               db CP/M / CTOS / ...\n"
	"11 Hide FAT12             63 GNU HURD or SysV        e1 DOS access\n"
	"12 Compaq diagnostics     64 Novell Netware 286      e3 DOS R/O\n"
	"14 Hide FAT16 <32M        65 Novell Netware 386      e4 SpeedStor\n"
	"16 Hide FAT16             70 DiskSecure Multi-Boot   eb BeOS fs\n"
	"17 Hide HPFS/NTFS         75 PC/IX                   f1 SpeedStor\n"
	"18 AST Windows swapfile   80 ELKS / Old Minix        f2 DOS secondary\n"
	"1b Hide Win9x FAT32       81 New Minix / Old Linux   f4 SpeedStor\n"
	"1c Hide Win9x FAT32 (LBA) 82 Linux swap              fd Linux raid autodetect\n"
	"1e Hide Win9x FAT16 (LBA) 83 New Linux               fe LANstep\n"
	"24 NEC DOS                84 OS/2 Hide C:            ff BBT\n"
    );
    fflush(stdout);
}

void quit(void)
{
    exit(1);
}

void list_part(void)
{
    if(*dev!=0)
	list_partition(NULL);
}

void add_part(void)
{
    unsigned char pentry[16];
    char buf[8];
    unsigned long tmp;
    unsigned char *oset;
    int part, scyl, ecyl;

    pentry[0]=0;
    printf("Add partition:\n\n");
    for (part=0;part<1 || part>4;) {
	printf("Which partition to add(1-4): ");
	fflush(stdout);
	fgets(buf,8,stdin);
	part=atoi(buf);
	if (*buf=='\n')
	    return;
    }

    oset=partitiontable+(0x1be)+((part-1)*16);

    printf("geo.cyl=%d\n",geometry.cylinders);
    for (scyl=geometry.cylinders+1;scyl<0 || scyl>geometry.cylinders;) {
	printf("First cylinder(%d-%d):",0,geometry.cylinders);
	fflush(stdout);
	fgets(buf,8,stdin);
	scyl=atoi(buf);
	if (*buf=='\n')
	    return;
    }

    pentry[1]=scyl==0?1:0;
    pentry[2]=1+((scyl >> 2) & 0xc0);
    pentry[3]=(scyl&0xff);

#ifdef PART_TYPE
    pentry[4]=PART_TYPE;
#else
    pentry[4]=0x80;
#endif

    for (ecyl=geometry.cylinders+1;ecyl<scyl || ecyl>geometry.cylinders;) {
	printf("Ending cylinder(%d-%d):",0,geometry.cylinders);
	fflush(stdout);
	fgets(buf,8,stdin);
	ecyl=atoi(buf);
	if(*buf=='\n')
	    return;
    }

    pentry[5]=geometry.heads-1;
    pentry[6]=geometry.sectors+((ecyl >> 2) & 0xc0);
    pentry[7]=(ecyl&0xff);

    tmp=spc*(unsigned long)scyl;
    if (scyl==0)
	tmp=(unsigned long)geometry.sectors;

    pentry[11]=(unsigned char)((tmp>>24uL)&0x000000ffuL);
    pentry[10]=(unsigned char)((tmp>>16uL)&0x000000ffuL);
    pentry[ 9]=(unsigned char)((tmp>> 8uL)&0x000000ffuL);
    pentry[ 8]=(unsigned char)((tmp      )&0x000000fful);

    tmp=spc*(unsigned long)(ecyl-scyl+1);
    if (scyl==0)
	tmp-=(unsigned long)geometry.sectors;

    pentry[15]=(unsigned char)((tmp>>24uL)&0x000000ffuL);
    pentry[14]=(unsigned char)((tmp>>16uL)&0x000000ffuL);
    pentry[13]=(unsigned char)((tmp>> 8uL)&0x000000ffuL);
    pentry[12]=(unsigned char)((tmp      )&0x000000ffuL);

    printf("Adding partition");
    memcpy(oset,pentry,16);
    printf("\n");
    fflush(stdout);
}

void set_boot(void)
{
    char buf[8];
    int part, a;

    printf("Toggle bootable flag:\n\n");
    for(part=0;part<1 || part>4;) {
	printf("Which partition to toggle(1-4): ");
	fflush(stdout);
	fgets(buf,8,stdin);
	part=atoi(buf);
	if (*buf=='\n')
	    return;
    }

    a=(0x1ae)+(part*16);
    partitiontable[a]=(partitiontable[a]==0x00?0x80:0x00);
}

int atohex(char *s)
{
    int n, r;

    n = 0;
    if (!isxdigit(*s))
	return 256;
    while (*s) {
	if (*s >= '0' && *s <= '9')
	    r = *s - '0';
	else if (*s >= 'A' && *s <= 'F')
	    r = *s - 'A' + 10;
	else if (*s >= 'a' && *s <= 'f')
	    r = *s - 'a' + 10;
	else
	    break;
	n = 16 * n + r;
	s++;
    }
    return n;
}

void set_type(void)  /* FIXME - Should make this more flexible */
{
    char buf[8];
    int part, type, a;

    printf("Set partition type:\n\n");
    for (part=0;part<1 || part>4;) {
	printf("Which partition to toggle(1-4): ");
	fflush(stdout);
	fgets(buf,8,stdin);
	part=atoi(buf);
	if (*buf=='\n')
	    return;
    }
    a=(0x1ae)+(part*16)+4;
    type=256;
    while (!valid(type)) {
	printf("Set partition type (l for list, q to quit): ");
	fflush(stdout);
	fgets(buf,8,stdin);
	if (isxdigit(*buf))
	    type=atohex(buf) % 256;
	else
	    switch (*buf) {

		case 'l':
		    list_types();
		    break;

		case 'q':
		    return;

		default:
		    printf("Invalid: %c\n", *buf);
		    break;
	    }
    }
    partitiontable[a]=type;
}

void del_part(void)
{
    char buf[8];
    int part;

    printf("Delete partition:\n\n");
    for (part=0;part<1 || part>4;) {
	printf("Which partition to delete(1-4): ");
	fflush(stdout);
	fgets(buf,8,stdin);
	part=atoi(buf);
	if (*buf=='\n')
	    return;
    }
    printf("Deleting partition %d...",part);
    memset(partitiontable+(0x1be)+((part-1)*16),0,16);
    printf("Done\n");
    fflush(stdout);
}

void write_out(void)
{
    int i;

    if (lseek(pFd,0L,SEEK_SET)!=0)
	printf("ERROR!  Cannot seek to offset 0.\n");
    else {
        partitiontable[510] = 0x55;
        partitiontable[511] = 0xAA;
	if ((i=write(pFd,partitiontable,512))!=512) {
	    printf("ERROR!  Only wrote %d of 512 bytes to the partition table.\n",i);
	    printf("        Table possibly corrupted.\n");
	} else
	    printf("Successfully wrote %d bytes to %s\n",i,dev);
    }
    fflush(stdout);
}

void help(void)
{
    Funcs *tmp;

    printf("Key Description\n");
    for (tmp=funcs;tmp->cmd;tmp++)
	printf("%c   %s\n",tmp->cmd,tmp->help);
    fflush(stdout);
}

static unsigned char ptbl[512];

void list_partition(char *devname)
{
    unsigned char *partition;
    unsigned long seccnt;
    int i;

    if (devname!=NULL) {
	if ((pFd=open(devname,O_RDONLY))==-1) {
	    printf("Error opening %s (%d)\n",devname,-pFd);
	    exit(1);
	}
	if ((i=read(pFd,ptbl,512))!=512) {
	    printf("Unable to read first 512 bytes from %s, only read %d bytes\n",
		   devname,i);
	    exit(1);
	}
    } else
	memcpy(ptbl,partitiontable,512);
    printf("                      START                  END\n");
    printf("Device    Boot  Head Sector Cylinder   Head Sector Cylinder  Type  Sector count\n\n");
    for (i=0x1be;i<0x1fe;i+=16) {
	partition=ptbl+i;   /* FIXME /--- gotta be a better way to do this ---\ */
	seccnt= ((unsigned long) partition[15] << 24uL) + \
		((unsigned long) partition[14] << 16uL) + \
		((unsigned long) partition[13] <<  8uL) + \
		 (unsigned long) partition[12];
	if (partition[5]) {
	    printf("%s%d  %c     %2d    %3d    %5d     %2d",
		devname==NULL?dev:devname,1+((i-0x1be)/16),
		partition[0]==0?' ':(partition[0]==0x80?'*':'?'),
		partition[1],				     /* Start head */
		partition[2] & 0x3f,			     /* Start sector */
		partition[3] | ((partition[2] & 0xc0) << 2), /* Start cylinder */
		partition[5]);				     /* End head */
	    printf("    %3d    %5d    %2x    %10ld\n",
		partition[6] & 0x3f,			     /* End sector */
		partition[7] | ((partition[6] & 0xc0) << 2), /* End cylinder */
		partition[4],				     /* Partition type */
		seccnt);				     /* Sector count */
	}
    }
    if (devname!=NULL)
	close(pFd);
    fflush(stdout);
}

int main(int argc,char *argv[])
{
    int i, mode=MODE_EDIT;

    dev[0]=0;
    for (i=1;i<argc;i++) {
	if (*argv[i]=='/') {
	    if (*dev!=0) {
		printf("Can only specify one device on the command line.\n");
		exit(1);
	    } else
		strncpy(dev,argv[i],256); /* FIXME - Should be some value from a header */
	} else
	    if (*argv[i]=='-')
		switch(*(argv[i]+1)) {
		    case 'l':
			mode=MODE_LIST;
			break;
		    default:
			printf("Unknown command: %s\n",argv[i]);
			exit(1);
		}
	    else
		die;
    }

    if(argc==1)
#ifdef DEFAULT_DEV
	strncpy(dev,DEFAULT_DEV,256);
#else
	die;
#endif


    if (mode==MODE_LIST) {
	if (*dev!=0)
	    list_partition(dev);
	else {
	    list_partition("/dev/hda");
	    list_partition("/dev/hdb");
	    /* Other drives */
	}
	return(1);
    }

    if (mode==MODE_EDIT) {
	char buf[CMDLEN];
	Funcs *tmp;
	int flag=0;

	if ((pFd=open(dev,O_RDWR))==-1) {
	    printf("Error opening %s (%d)\n",dev,-pFd);
	    exit(1);
	}

	if ((i=read(pFd,partitiontable,512))!=512) {
	    printf("Unable to read first 512 bytes from %s, only read %d bytes\n",
		   dev,i);
	    exit(1);
	}

	if (ioctl(pFd, HDIO_GETGEO, &geometry)) {
	    printf("Error getting geometry of disk, exiting.\n");
	    exit(1);
	}
	if (geometry.heads==0 && geometry.cylinders==0 && geometry.sectors==0)
	    printf("WARNING!  Read disk geometry as 0/0/0.  Things may break.\n");
	printf("\nGeometry: %d cylinders, %d heads, %d sectors.\n\n",
		geometry.cylinders,geometry.heads,geometry.sectors);
	fflush(stdout);
	while(!feof(stdin)) {
	    printf("Command%s:",flag==0?" (? for help)":"");
	    flag=1;
	    fflush(stdout);
	    *buf = 0;
	    if (fgets(buf,CMDLEN-1,stdin)) {
		printf("\n");
		for (tmp=funcs; tmp->cmd; tmp++)
		    if (*buf==tmp->cmd) {
			tmp->func();
			break;
		    }
	    }
	}
    }
    exit(0);
}
