/* gc.c - generates the required mappings */


# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/charset/RCS/gc.c,v 6.0 1991/12/18 20:21:39 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/charset/RCS/gc.c,v 6.0 1991/12/18 20:21:39 jpo Rel $
 *
 * $Log: gc.c,v $
 * Revision 6.0  1991/12/18  20:21:39  jpo
 * Release 6.0
 *
 */


#include	<stdio.h>
#include    <string.h>
#include	<ctype.h>
#include	"charset.h"

#define	AMAX	2000

extern char	*charset_sets, *charset_defs, *charset_map, *charset_mnem;
extern char	*charset_dir;
/* char charset_sets[] = "CHARSETS"; */
/* char charset_defs[] = "CHARDEFS"; */
/* char charset_map[] = "CHARMAP.10646"; */
/* char charset_mnem[] = "MNEM"; */
/* char charset_dir[] = "cs"; */

FILE		*f,*g,*h;
CHAR8U		s[LSIZE], c[AMAX], ref[C256], cmd[LSIZE];
INT16S		v[AMAX], l[AMAX], codetable[C256];
int		i,mx,a,cv,val,cod,dupl,ecma,line,group,plane,row,cell,num;

void gwrite() {
	if (g) {
		for (i= 0; i < C256; i++)
			if (c[codetable[i]] == NULL)
				c[codetable[i]] = i;
		fwrite (codetable,2,C256,g);
		fwrite (c,1,mx,g);
		fclose (g);
	}
}

FILE *
fopener (file,mode)
char *file, *mode;
{
	FILE *f;
	f= fopen (file,mode);
	if (f == (FILE *)NULL) {
		(void) fprintf (stderr,"*** Error: unable to open %s\n", file);
		exit (1);
	}
	return f;
}

void main (argc,argv) int argc; char **argv; {
	CHAR8U	defname[LSIZE], linkname[LSIZE], *p;

	mx= 0; line= 0;

	f= fopener (charset_defs,"r");
	h= fopener (charset_map,"w");

	v[mx++]= '?' * C256 +'?';
	while (fgets ((char *)s,LSIZE,f)) {
		line++;
		if (mx > AMAX) perror ("Warning: too many chars\n");
		if (s[0] != ' ' && strlen (s) > 2) {
			v[mx++]= s[0]* C256 + s[1]; l[mx]= line;
			for (i=0; i < mx-1; i++) if (v[mx-1] == v[i])
				(void) fprintf (stderr, 
			"Warning: char %2.2s occurs in line %d and %d\n",
			s,l[i+1],line);
			(void) fprintf (h,"%2.2s\t,,,%.3d,,,%.3d,,,%.3d,,,%.3d\t%s",
			s,group,plane,row,cell,s+3);
			cell++;
		}
		else if (strlen (s) >3 ){
			sscanf ((char *)s," %s %d", cmd, &num);
			for (p=cmd; *p; p++) if (isupper (*p)) *p= tolower (*p);
			/* printf ("command %s %d\n",cmd,num); */
			if (strcmp (cmd,"group") == 0) group=num;
			else if (strcmp (cmd,"plane") == 0) plane=num;
			else if (strcmp (cmd,"row") == 0) row=num;
			else if (strcmp (cmd,"cell") == 0) cell=num;
		}
	}

	(void) fprintf (stdout,"%d chars defined\n",mx);
	fclose (f); fclose (h);

	v[0]= mx;
	g= fopener (charset_mnem,"w");

	fwrite (v,2,mx,g);
	v[0]= '?' * C256 +'?';

	f= fopener (charset_sets,"r");

	if (chdir (charset_dir))
		(void) fprintf (stderr,"Warning: cannot cd to %s\n",charset_dir);

	while (fscanf (f,"%s",s) != EOF) {
		if (strlen (s) == 1) { s[1]= ' '; s[2]= '\0'; }
		cv= s[0]* C256 + s[1];
		if (strlen (s) != 2) {
			if (strcmp ((char *)s,"referenceset") == 0) {
				fgets ((char *)ref+32,40,f); /* skip rest of line */
				fgets ((char *)ref+32,40,f);
				fgets ((char *)ref+64,40,f);
				fgets ((char *)ref+96,40,f);
				for (i=0; i< 32; i++) ref[i]= 0;
				for (i=33; i< C256; i++)
				if (ref[i] <= ref[32]) ref[i]= 0;
				fwrite (ref,1,C256,g);
				fclose (g); g= NULL;
			} else if (strcmp ((char *)s,"charset") == 0) {
				fscanf (f,"%s",defname);
				gwrite();
				for (p= defname; *p; p++)
				if (islower (*p)) *p= toupper (*p);
				g= fopener (defname,"w");
				cod= 0; ecma= 0;
				for (i= 0; i < mx; i++) c[i] = 0;
				for (i=0; i< C256; i++) codetable[i]= 0;
			} else if (strcmp ((char *)s,"alias") == 0) {
				fscanf (f,"%s",linkname);
				for (p= linkname; *p; p++)
				if (islower (*p)) *p= toupper (*p);
				unlink (linkname);
				link (defname,linkname);
			} else if (strcmp ((char *)s,"ecma") == 0) {
				ecma= fscanf (f,"%s",s);
				if (s[0] == 'o') sscanf ((char *)s+1,"%o",&ecma);
				else if (s[0] == 'x') sscanf ((char *)s+1,"%x",&ecma);
				else sscanf ((char *)s,"%d",&ecma);
			} else if (strcmp ((char *)s,"code") == 0) {
				cod= fscanf (f,"%s",s);
				if (s[0] == 'o') sscanf ((char *)s+1,"%o",&cod);
				else if (s[0] == 'x') sscanf ((char *)s+1,"%x",&cod);
				else sscanf ((char *)s,"%d",&cod);
			} else if (strcmp ((char *)s,"comment:") == 0) {
				char c;
				while ((c = fgetc (f)) != EOF && c != '\n');
			} else if (strcmp ((char *)s,"duplicate") == 0) {
				dupl= fscanf (f,"%s",s);
				if (s[0] == 'o') sscanf ((char *)s+1,"%o",&dupl);
				else if (s[0] == 'x') sscanf ((char *)s+1,"%x",&dupl);
				else sscanf ((char *)s,"%d",&dupl);
				if (dupl <0 || dupl >= C256 ||  
				! codetable[dupl])
					(void) fprintf (stderr, "Warning: charset %s has duplicate %d but no original\n",defname,dupl);
				fscanf (f,"%s",s);
				if (strlen (s) == 1) { s[1]= ' '; s[2]= '\0'; }
				cv= s[0]* C256 + s[1];
				for (i= 0; i < mx && cv != v[i]; i++);
				if (i >= mx)
					(void) fprintf (stderr, "Warning: charset %s duplicate %d %s unknown\n",defname,dupl,s);
				c[i] = dupl;
			} else
				(void) fprintf (stderr, "Warning: charset %s unknown command '%s'\n",defname,s);
		} else {
			for (i= 0; i < mx && cv != v[i]; i++);
			if (i >= mx)
				(void) fprintf (stderr, "charset %s char %s %d not defined\n",defname,s,cod++);
			else  {
				val= i;
				if (val) for (i=0; i <= cod && codetable[i] != val; i++);
				if (val && codetable[i] == val)
					(void) fprintf (stderr, "Warning: charset %s char %s occurs twice %d %d\n",defname,s,i,cod);
				codetable[cod++]= val;
			}
		}
	}
	fclose (f);
	gwrite();
	exit (0);
}
