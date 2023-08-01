/* bin2ascii: converts a Binary file to a ascii one */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/bin2ascii/RCS/bin2ascii.c,v 6.0 1991/12/18 20:28:31 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/bin2ascii/RCS/bin2ascii.c,v 6.0 1991/12/18 20:28:31 jpo Rel $
 *
 * $Log: bin2ascii.c,v $
 * Revision 6.0  1991/12/18  20:28:31  jpo
 * Release 6.0
 *
 */



#include <stdio.h>


#define ASC_HT                  0x09
#define ASC_CR                  0x0D
#define ASC_DC1                 0x11
#define ASC_DC3                 0x13

#define ASC_0                   0x30
#define ASC_1                   0x31
#define ASC_2                   0x32
#define ASC_3                   0x33
#define ASC_4                   0x34
#define ASC_5                   0x35
#define ASC_6                   0x36
#define ASC_7                   0x37
#define ASC_8                   0x38
#define ASC_9                   0x39
#define ASC_A                   0x41
#define ASC_B                   0x42
#define ASC_C                   0x43
#define ASC_D                   0x44
#define ASC_E                   0x45
#define ASC_F                   0x46
#define ASC_X                   0x58

#define ASC_DOLLAR              0x24
#define ASC_TILDE               0x7E

#define ASC_CTL                 0x01
#define ASC_NUM                 0x02
#define ASC_UPP                 0x04
#define ASC_LOW                 0x08
#define ASC_SPA                 0x10
#define ASC_PUN                 0x20

#define ASC_ISCTL(chr)          ( Asc_class[chr] & ASC_CTL )
#define ASC_ISNUMER(chr)        ( Asc_class[chr] & ASC_NUM )
#define ASC_ISUPPER(chr)        ( Asc_class[chr] & ASC_UPP )
#define ASC_ISLOWER(chr)        ( Asc_class[chr] & ASC_LOW )
#define ASC_ISPUNCT(chr)        ( Asc_class[chr] & ASC_PUN )
#define ASC_ISSPACE(chr)        ( Asc_class[chr] & ASC_SPA )
#define ASC_ISALPHA(chr)        ( Asc_class[chr] & (ASC_UPP|ASC_LOW) )
#define ASC_ISALPHANUM(chr)     ( Asc_class[chr] & (ASC_UPP|ASC_LOW|ASC_NUM) )
#define ASC_ISPRINT(chr)        \
	 ( Asc_class[chr] & (ASC_UPP|ASC_LOW|ASC_NUM|ASC_PUN|ASC_SPA) )

#define ASC_ISDEC(chr)          ( Asc_value[chr] < 10 )
#define ASC_ISHEX(chr)          ( Asc_value[chr] < 16 )
#define ASC_ISBIN(chr)          ( Asc_value[chr] < 2  )
#define ASC_ISOCT(chr)          ( Asc_value[chr] < 8  )

#define ASC_TOUPPER(chr)        ( ASC_ISLOWER(chr) ? (chr)&~0x20 : (chr) )
#define ASC_TOLOWER(chr)        ( ASC_ISUPPER(chr) ? (chr)| 0x20 : (chr) )

extern char                     Asc_class[];
extern char                     Asc_value[];
extern char                     Asc_digit[];

#define ILG                     64


char Asc_class[256] = {
    ASC_CTL,    ASC_CTL,        ASC_CTL,        ASC_CTL,
    ASC_CTL,    ASC_CTL,        ASC_CTL,        ASC_CTL,
    ASC_CTL,    ASC_SPA,        ASC_CTL,        ASC_CTL,
    ASC_CTL,    ASC_CTL,        ASC_CTL,        ASC_CTL,

    ASC_CTL,    ASC_CTL,        ASC_CTL,        ASC_CTL,
    ASC_CTL,    ASC_CTL,        ASC_CTL,        ASC_CTL,
    ASC_CTL,    ASC_CTL,        ASC_CTL,        ASC_CTL,
    ASC_CTL,    ASC_CTL,        ASC_CTL,        ASC_CTL,

    ASC_SPA,    ASC_PUN,        ASC_PUN,        ASC_PUN,
    ASC_PUN,    ASC_PUN,        ASC_PUN,        ASC_PUN,
    ASC_PUN,    ASC_PUN,        ASC_PUN,        ASC_PUN,
    ASC_PUN,    ASC_PUN,        ASC_PUN,        ASC_PUN,

    ASC_NUM,    ASC_NUM,        ASC_NUM,        ASC_NUM,
    ASC_NUM,    ASC_NUM,        ASC_NUM,        ASC_NUM,
    ASC_NUM,    ASC_NUM,        ASC_PUN,        ASC_PUN,
    ASC_PUN,    ASC_PUN,        ASC_PUN,        ASC_PUN,


    ASC_PUN,    ASC_UPP,        ASC_UPP,        ASC_UPP,
    ASC_UPP,    ASC_UPP,        ASC_UPP,        ASC_UPP,
    ASC_UPP,    ASC_UPP,        ASC_UPP,        ASC_UPP,
    ASC_UPP,    ASC_UPP,        ASC_UPP,        ASC_UPP,

    ASC_UPP,    ASC_UPP,        ASC_UPP,        ASC_UPP,
    ASC_UPP,    ASC_UPP,        ASC_UPP,        ASC_UPP,
    ASC_UPP,    ASC_UPP,        ASC_UPP,        ASC_PUN,
    ASC_PUN,    ASC_PUN,        ASC_PUN,        ASC_PUN,

    ASC_PUN,    ASC_LOW,        ASC_LOW,        ASC_LOW,
    ASC_LOW,    ASC_LOW,        ASC_LOW,        ASC_LOW,
    ASC_LOW,    ASC_LOW,        ASC_LOW,        ASC_LOW,
    ASC_LOW,    ASC_LOW,        ASC_LOW,        ASC_LOW,

    ASC_LOW,    ASC_LOW,        ASC_LOW,        ASC_LOW,
    ASC_LOW,    ASC_LOW,        ASC_LOW,        ASC_LOW,
    ASC_LOW,    ASC_LOW,        ASC_LOW,        ASC_PUN,
    ASC_PUN,    ASC_PUN,        ASC_PUN,        ASC_CTL,


    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,

    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,

    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,

    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,


    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,

    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,

    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,

    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0,
    0,          0,              0,              0
  };


char Asc_value[256] = {
    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,
    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,

    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,
      0,  1,  2,  3,   4,  5,  6,  7,   8,  9,ILG,ILG, ILG,ILG,ILG,ILG,

    ILG, 10, 11, 12,  13, 14, 15,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,
    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,

    ILG, 10, 11, 12,  13, 14, 15,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,
    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,

    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,
    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,

    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,
    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,

    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,
    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,

    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG,
    ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG, ILG,ILG,ILG,ILG
  };


char Asc_digit[16] = {
    ASC_0, ASC_1, ASC_2, ASC_3,  ASC_4, ASC_5, ASC_6, ASC_7,
    ASC_8, ASC_9, ASC_A, ASC_B,  ASC_C, ASC_D, ASC_E, ASC_F
};


#define NUM 16




/* ---------------------  Begin  Routines  -------------------------------- */




main (argc, argv)
int     argc;
char    **argv;
{
	FILE    *fd;
	int     c, i=0, flag=0;
	char    hex[80], ascii[80];

	if (argc != 2)
		return (printf ("\n\nUsage:  bin2ascii file-name\n\n"));

	fd = fopen (argv[1], "r");

	while ((c = getc(fd)) != EOF)
	{
		if (i == NUM)
		{
			sprbin (&ascii[0], &hex[0], NUM);
			printf ("%s\n", &ascii[0]);
			i=0;
		}

		hex[i] = c;
		i++;
		flag++;
	}

	if (flag)
	{
		sprbin (&ascii[0], &hex[0], NUM);
		printf ("%s\n", &ascii[0]);
	}

	fclose (fd);
	return;
  }




/* ---------------------  Static  Routines  ------------------------------- */




static sprbin (line, buf, max)
    unsigned char*      line;
    unsigned char*      buf;
    int                 max;
  {
    unsigned char       *chrs, *nums;
    int                 c, i;

    chrs = line;
    nums = line + NUM;

    if (max > NUM)  max = NUM;

    for (i=0; i<max; ++i)
      {
	if ((i&3) == 0)  *nums++ = ' ';

	c = *buf++;
	*nums++ = Asc_digit[c>>4];
	*nums++ = Asc_digit[c&0xF];
	c &= 0x7F;
	*chrs++ = ASC_ISPRINT(c) && c!=ASC_HT ? c : '.';
      }

    while (i++ < NUM)  *chrs++ = ' ';

    *nums = '\0';
  }
