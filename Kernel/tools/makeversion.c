#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void write_body(FILE *out, char *v, char *sv, char *p)
{
	fprintf(out, "#include \"kernel.h\"\n\
#include \"version.h\"\n\
/* Format is version/sysname/release/system */\n\
/* pack version first so we can use uname_str as a printf target */\n\
struct sysinfoblk sysinfo = {\n\
	sizeof(struct sysinfoblk),\n\
	CONFIG_BANKS,\n\
	UFTSIZE,\n\
	PTABSIZE,\n\
	TICKSPERSEC,\n\
	0,\n\
	0,\n\
#ifdef CONFIG_PROFIL\n\
	CONF_PROFIL |\n\
#endif\n\
#ifdef CONFIG_LEVEL_2\n\
	CONF_LEVEL_2 |\n\
#endif\n\
	0,\n\
	{0, 0, 0},\n\
	0,\n\
	0,\n\
	0,\n\
	{0, 0, 0},\n\
	{0, 0, 0, 0, 0, 0, 0, 0},\n\
	/* Quoting to work around cc65 bug */\n\
	\"%s\\0\"\"Fuzix\\0\"\"%s\\0\"\"%s\"\n\
};\n\n",
	v, sv, p);
}

void write_header(FILE *out, char *v, char *sv, char *p)
{
	fprintf(out, "\
/*\n\
 *	System info shared with user space\n\
 */\n\
struct sysinfoblk {\n\
  uint8_t infosize;		/* For expandability */\n\
  uint8_t banks;		/* Banks in 64K (and thus pagesize) */\n\
  uint8_t max_open;\n\
  uint8_t nproc;		/* Number of processes */\n\
  uint16_t ticks;		/* Tick rate in HZ */\n\
  uint16_t memk;		/* Memory in KB */\n\
  uint16_t usedk;		/* Used memory in KB */\n\
  uint16_t config;		/* Config flag mask */\n\
#define CONF_PROFIL		1\n\
#define CONF_NET		2	/* Hah.. 8) */\n\
#define CONF_LEVEL_2		4\n\
  uint16_t loadavg[3];\n\
  uint16_t swapk;\n\
  uint16_t swapusedk;\n\
  uint8_t cputype;		/* CPU type information */\n\
  uint8_t cpu[3];		/* CPU type specific data */\n\
  uint16_t spare[8];\n\
  char uname[%d];\n\
};\n\
\n", (int)(strlen(v) + strlen(sv) + strlen(p) + 9));
}

int main(int argc, char *argv[])
{
	FILE *f;
	if (argc != 4) {
		fprintf(stderr, "%s version subversion platform\n", argv[0]);
		exit(1);
	}
	f = fopen("include/sysinfoblk.h", "w");
	if (f == NULL) {
		perror("include/sysinfoblk.h");
		exit(1);
	}
	write_header(f, argv[1], argv[2], argv[3]);
	fclose(f);
	f = fopen("version.c", "w");
	if (f == NULL) {
		perror("version.c");
		exit(1);
	}
	write_body(f, argv[1], argv[2], argv[3]);
	return fclose(f) != 0;
}
