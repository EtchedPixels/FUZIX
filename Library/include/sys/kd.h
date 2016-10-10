/*
  Header file for manipulation of the fuxiz console keyboard
*/


/* IOCTL numbers */
#define KBMAPSIZE	0x20
#define KBMAPGET	0x21
#define KBSETTRANS	(0x23|IOCTL_SUPER)
#define KBRATE          0x25


/* struct for setting keyboard repeat
   delays are demarked in tenths of seconds
 */
struct key_repeat {
	uint8_t first;
	uint8_t continual;
};

