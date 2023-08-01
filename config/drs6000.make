# Default Configuration values for other Makefiles
#
# You should look through all of them, but in particular lines the first
# too sections.
#	Tailor this		May need to be tailored to your system
#	Check this		Ensure that this is OK
#
# NOTE: SUN MAKE AND SOME NEWER MAKES TOO TAKE OF TRAILING SPACES IN A
#  MACRO DEFINITION. HENCE YOU SHOULD EITHER REMOVE THE TRAILING COMMENTS
#  OR MAKE SURE THE COMMENT STARTS DIRECTLY AFTER THE DEFINITION FOR THOSE
#  MACROS WHERE THIS IS IMPORTANT (e.g pathnames).
#
############################################################
#
#
############################################################
#

############################################################
#
# Things you really should tailor
#
############################################################

# List of channels that you may require. Choose from those in the
# Chans/ directory. Each should be a directory name within Chans.
OPTIONALCHANS	= shell lists 822-local smtp uucp x40084
# If fax defined - give type
#FAXDRIVERS = dexNet200 # or ps250 ...
#size of page to produce when converting to g3fax encoding
#not defined produces US size pages 216 mm by 273 mm (width, height)
#-DA4 produces A4 size pages 210mm by 297mm
#FAXPAGESIZE = -DA4

# Optional filters
OPTIONALFILTERS =

# This is the external libraries required by all programs. 
# Should include isode at the least. dbm maybe also.
LIBSYS          = -ldsap -lisode -ldbm
# For SYS5 also need -lsocket -lgen


# Compilation things.
# C compiler, cc or gcc.
CC              = cc # gcc
# suns cc if using gcc on sparc
OLDCC		= $(CC)
CCOPTIONS       = -O # -g
LIBCCOPTIONS	= $(CCOPTIONS)
LDOPTIONS       = -s # -g


############################################################
#
# These things may need changing
#
############################################################

# The PP base directory from private binaries et al.
PPDIR		= /usr/lib/pp


# A private directory for host specific files - set to PPDIR if you
# are not sharing things.
PRIDIR		= $(PPDIR)


# The spool directory for mail.
SPLDIR		= /usr/spool/pp


# The base directory for Manual pages /usr/man or /usr/local/man or similar
MANDIR          = /usr/man
# options for installing manual pages...
# includes -bsd42 -bsd44 -ros -sys5 -aix -local -l -hpux
# see script man/inst-man.sh for more details
MANOPTS		= -bsd42

# The public access binary directory.
USRBINDIR	= /usr/local/bin


# The username to install PP binaries under
PPUSER		= pp


# Do you have X11 installed? Leave blank otherwise
X11		= true
#The X libraries. May need -lXext for X11R4
LIBX		= -lXaw -lXmu -lXext -lXt -lX11
# The applications default directory
APPDEFAULTS     = /usr/lib/X11/app-defaults


# SMTP DNS support - either empty or the resolver library
LIBRESOLV	=

# Photo support
LIBPHOTO	= /usr/local/etc/g3fax/libphoto.a


# These commands have a habit of moving around
CHOWN		= chown
CHMOD		= chmod



# If grey book is defined - these need to be correct - otherwise ignore
# Niftp src directory - niftp should be built beforehand.
NIFTPSRC	= /usr/src/local/niftp
# The niftp interface in use.
NIFTPINTERFACE	= sun

############################################################
#
# These things are probably OK as they are...
#
############################################################

MAKE		= ./make
SHELL		= /bin/sh

TXTDIR		= $(PPDIR)
PRIDIR		= $(PPDIR)
#
#  These are all sub dirs
TAILOR		= $(PRIDIR)/tailor
BINDIR          = $(PPDIR)/bin
CMDDIR          = $(PPDIR)/cmds
TBLDIR          = $(TXTDIR)/tables
LOGDIR          = $(SPLDIR)/logs
QUEDIR		= $(SPLDIR)/queues
CHANDIR         = $(CMDDIR)/chans
FORMDIR         = $(CMDDIR)/format
TOOLDIR		= $(CMDDIR)/tools

# ISODE dependencies
PEPY            = pepy
PYFLAGS         =
ROSY		= rosy
RYFLAGS		=
POSY		= posy
POFLAGS		=
PEPSY		= pepsy
PEPSYHDRS	= /usr/local/include/isode/pepsy
# Other Libraries and programs

# Archive Configuration
AR              = ar
ARFLAGS         =
RANLIB          = ranlib
# SYS5 has no ranlib - replace with echo

# Lint Config
LINT            = lint
LINTFLAGS       = -haxbc

# program protections?
PGMPROT         = 755
ROOTUSER	= root

# Other things
BACKUP		= cp
INSTALL		= cp

# Documentation support
LATEX           = latex
TGRIND          = tgrind
GRINDEFS        = grindefs
WEAVE           = weave
DVI2PS          = dvi2ps
DVIIMP          = dviimp
DVISP           = dvisp
DFLAGS          =
