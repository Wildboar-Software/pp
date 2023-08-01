Options for the config.h file are amongst the following.

define NDBM or DBM depening on what database you have/want.
	NDBM - new database stuff - 4.2/3 and sunos. If you have it, use it.
	DBM  - old dbm needs -ldbm for file access etc.

Logging for PP
	PP_DEBUG - this is a number, set to 0 for no debugging, 1 for some
		and 2 for all tracing.

Nameserver access
#define NAMESERVER if you wish to make use of the BIND domain nameserver.
	[Note: This will require proper configuration of the LIBRESOLV
	 make variable]

#define HAS_FSYNC define this if your system has the fsync(2) call.
	This is defined for you if on a BSD or SVR4 based system.

#define UKORDER defines that user tools (mlist etc) should use uk rfc-822 
	domain ordering by default

#define VAT define this for some cute special cases.
