This file describes the various macros you will need to define for Make.defs
See volume 1 of the PP user guide for more details.

BINDIR		= /usr/lib/pp/bin
administrator binaries - no user program found here.

CMDDIR		= /usr/lib/pp/cmds
PP main programs

CHANDIR		= $(CMDDIR)/chans
PP channel programs

FORMDIR		= $(CMDDIR)/format
PP formatting channels run under fcontrol

TOOLDIR		= $(CMDDIR)/tools
misc tools

LOGDIR		= /usr/lib/pp/logs
Where the log files should be placed

MANDIR		= /usr/local/man
where the manual pages should go
MANOPTS		 = -bsd42
Manual page installation options

TAILOR		= /usr/lib/pp/tailor
The tailor file. This file is critical.

QUEDIR		= /usr/lib/pp/spool/queues
Where the PP disc queue is kept - needs plenty of headroom

TBLDIR		= /usr/lib/pp/tables
Where the routing tables etc go.

USRBINDIR	= /usr/local/bin
Where user visible binaries should be placed.

# Other Libraries and programs
LIBSYS		= -ldsap -lisode
System installed libraries. -ldbm may be useful

LIBPHOTO	= /usr/local/etc/g3fax/libphoto.a
The quipu photo library - required for xalert

OPTIONALCHANS	= lists local shell smtp uucp x40084
The particular channels that you require building.

FAXDRIVERS	= dexNet200 # or ps250 ...
The particular fax modem you have (need fax in OPTIONALCHANS)

#size of page to produce when converting to g3fax encoding
#not defined produces US size pages 216 mm by 273 mm (width, height)
#-DA4 produces A4 size pages 210mm by 297mm
#FAXPAGESIZE = -DA4

OPTIONALFILTERS	=
Other filters you may require

PEPY		= pepy
PYFLAGS		=
ISODE pepy program and any global flags

ROSY		= rosy
RYFLAGS		=
ISODE rosy program and any global flags

POSY		= posy
POFLAGS		=
ISODE posy program and any global flags

PEPSY		= pepsy
ISODE pepsy program
PEPSYHDRS   	= /usr/local/include/isode/pepsy

LINTISODE	= -lisode
The ISODE lint library if needed. Only useful if you wish to lint the
PP sources

X11		= true
Set to "true" if you have X11, leave empty otherwise

LIBX		= -lXaw -lXmu -lXt -lX11 
The X11 libraries you will need.

APPDEFAULTS	= /usr/lib/X11/app-defaults
The X11 application defaults directory.

LIBRESOLV	=
The name of the BIND resolver library. You will need to define
NAMESERVER if you want to use this.

CC		= cc
The C compiler
OLDCC		= $(CC)
System CC - used to work round gcc (1.37-1.??) bug on sparcs

CCOPTIONS	= -O
Compiler options

LIBCCOPTIONS	= -O
Compiler options for just the libraries.

LDOPTIONS	=
Any loader options needed

AR		= ar
The archiver program

ARFLAGS		=
Archiver flags

RANLIB		= ranlib
The ranlib program if you need it

LINT		= lint
The lint program

LINTFLAGS	= -haxbc
The lint flags in force

PGMPROT		= 755
The mode for general binaries to be installed with

PPUSER		= pp
The User which will own the PP system

ROOTUSER	= root
The root user - needed for setuid channels

CHOWN		= chown
man 8 chown command

CHMOD		= chmod
man 1 chmod command

BACKUP		= cp
What to use to save backup copies - set to : if you don't want any

INSTALL		= cp
# what to use to install new binaries etc. install(1) is also suitable


# Various documentation support
LATEX           = latex
TGRIND          = tgrind
GRINDEFS        = grindefs
WEAVE           = weave
DVI2PS          = dvi2ps
DVIIMP          = dviimp
DVISP           = dvisp
DFLAGS          =

