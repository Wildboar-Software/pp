/* x400trace2rfc: converts an x400 Trace struct into an RFC string as by RFC 1138 */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/format/RCS/x400trc2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/format/RCS/x400trc2rfc.c,v 6.0 1991/12/18 20:22:06 jpo Rel $
 *
 * $Log: x400trc2rfc.c,v $
 * Revision 6.0  1991/12/18  20:22:06  jpo
 * Release 6.0
 *
 */



#include	"util.h"
#include	"mta.h"
#include	<isode/cmd_srch.h>
#include	"tb_bpt88.h"

extern CMD_TABLE	bptbl_body_parts88 [/* body part types */];
extern int	globalid2rfc();
extern UTC	utclocalise();
extern char *oid2lstr ();

static int bodypart2value (bodypart)
char	*bodypart;
{
	int	ret;

	if (bodypart == NULLCP)
		return NOTOK;
	
	switch (ret = cmd_srch (bodypart, bptbl_body_parts88)) {
	    case BPT_UNDEFINED:
	    case BPT_TLX:
	    case BPT_IA5:
	    case BPT_G3FAX:
	    case BPT_TIF0:
	    case BPT_TTX:
	    case BPT_VIDEOTEX:
	    case BPT_VOICE:
	    case BPT_SFD:
	    case BPT_TIF1:
		return ret;
	    default:
		if (strncmp (bodypart, "oid.", 4) == 0)
			return BPT_EXTERNAL;
		break;
	}
	return NOTOK;
}

int x400eits2rfc (eits, buffer)
EncodedIT *eits;
char		*buffer;
{
	LIST_BPT	*ep;
	
	buffer[0] = '\0';

	for (ep = eits -> eit_types; ep; ep = ep -> li_next) {
		switch (bodypart2value (ep -> li_name)) {
		    case BPT_UNDEFINED:
			if (lexequ(ep -> li_name,
				   rcmd_srch(BPT_UNDEFINED,
					     bptbl_body_parts88)) == 0) {
				if ('\0' != buffer[0])
					(void) strcat (buffer, ",");
				(void) strcat(buffer, "undefined (0)");
			}
			break;
		    case BPT_TLX:
			if ('\0' != buffer[0])
				(void) strcat (buffer, ",");
			(void) strcat(buffer, "telex (1)");
			break;

		    case BPT_IA5:
			if ('\0' != buffer[0])
				(void) strcat (buffer, ",");
			(void) strcat(buffer, "ia5 text (2)");
			break;

		    case BPT_G3FAX:
			if ('\0' != buffer[0])
				(void) strcat (buffer, ",");
			(void) strcat(buffer, "g3 facsimile (3)");
			break;

		    case BPT_TIF0:
			if ('\0' != buffer[0])
				(void) strcat (buffer, ",");
			(void) strcat(buffer, "g4 class 1 (4)");
			break;

		    case BPT_TTX:
			if ('\0' != buffer[0])
				(void) strcat (buffer, ",");
			(void) strcat(buffer, "teletex (5)");
			break;

		    case BPT_VIDEOTEX:
			if ('\0' != buffer[0])
				(void) strcat (buffer, ",");
			(void) strcat(buffer, "videotex (6)");
			break;

		    case BPT_VOICE:
			if ('\0' != buffer[0])
				(void) strcat (buffer, ",");
			(void) strcat(buffer, "voice (7)");
			break;

		    case BPT_SFD:
			if ('\0' != buffer[0])
				(void) strcat (buffer, ",");
			(void) strcat(buffer, "sfd (8)");
			break;
		    case BPT_TIF1:
			if ('\0' != buffer[0])
				(void) strcat (buffer, ",");
			(void) strcat(buffer, "mixed mode (9)");
			break;
			
		    case BPT_EXTERNAL:
			/* oid */
			if ('\0' != buffer[0])
				(void) strcat (buffer, ",");
			(void) strcat(buffer, 
				      oid2lstr(str2oid
					       ((ep -> li_name + strlen("oid.")))));
			break;

		    default:
			/* -- unregonised -- */
			break;
		}
	}
	return OK;
}
			
int	x400trace2rfc (trace, buffer)
Trace 	*trace;
char	*buffer;
{
	char	tbuf[LINESIZE], *cp;
	UTC	lut;

	if (trace == (Trace *) NULL)
		return DONE;
	 
	(void) sprintf(cp = buffer, "by ");
	cp += strlen (cp);

	if (trace->trace_mta != NULL) {
		ap_val2str(tbuf, trace->trace_mta, AP_DOMAIN);
		(void) sprintf(cp, "mta %s in ", tbuf);
		cp += strlen(cp);
	}
	if (globalid2rfc(&trace -> trace_DomId, cp) == NOTOK)
		return NOTOK;
	cp += strlen(cp);

	(void) sprintf(cp, ";");
	cp += strlen(cp);

	if (trace->trace_DomSinfo.dsi_deferred != NULLUTC) { 
		if ((lut = utclocalise(trace->trace_DomSinfo.dsi_deferred)) != NULLUTC
		    && UTC2rfc (lut, tbuf) == OK) {
			(void) sprintf (cp, " deferred until %s;", tbuf);
			free ((char *) lut);
			cp += strlen(cp);
		}
		else return NOTOK;
	}

	if (trace->trace_DomSinfo.dsi_converted.eit_types != NULL) {
		if (x400eits2rfc(&trace->trace_DomSinfo.dsi_converted,
				 tbuf) == NOTOK)
			return NOTOK;
		if ('\0' != tbuf[0])
			(void) sprintf(cp, " converted (%s);", tbuf);
		cp += strlen(cp);
	}

	if (trace->trace_DomSinfo.dsi_attempted_md.global_Country != NULLCP) {
		(void) sprintf(cp, " attempted ");
		cp += strlen(cp);
		if (trace->trace_DomSinfo.dsi_attempted_mta != NULLCP) {
			ap_val2str(trace->trace_DomSinfo.dsi_attempted_mta,
				   tbuf, AP_DOMAIN);
			(void) sprintf(cp, " mta %s in ", tbuf);
			cp += strlen(cp);
		}
		if (globalid2rfc(&(trace->trace_DomSinfo.dsi_attempted_md),
				 cp) == NOTOK)
			return NOTOK;
		cp += strlen(cp);
		
		(void) sprintf(cp, ";");
		cp += strlen(cp);
	}
	if (trace->trace_DomSinfo.dsi_action ==  ACTION_RELAYED)
		(void) sprintf(cp, " Relayed");

	else if (trace->trace_DomSinfo.dsi_action == ACTION_ROUTED)
		(void) sprintf(cp, " Rerouted");

	cp += strlen(cp);

	if (trace->trace_DomSinfo.dsi_other_actions & ACTION_REDIRECTED)
		(void) sprintf(cp, " Redirected");
	if (trace->trace_DomSinfo.dsi_other_actions & ACTION_EXPANDED)
		(void) sprintf(cp, " Expanded");
	cp += strlen(cp);

	lut = utclocalise(trace->trace_DomSinfo.dsi_time);
	if (UTC2rfc(lut, tbuf) == OK)
		(void) sprintf(cp, "; %s", tbuf);
	if (lut) free((char *) lut);

	PP_DBG (("Lib/x400trace2rfc returns (%s)", buffer));
	return OK;
}
