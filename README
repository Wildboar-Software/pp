


PP-GEN(8)             MAINTENANCE COMMANDS              PP-GEN(8)



NAME
     pp-gen - generating the PP message system

     You should read over this entire document first, before typ-
     ing any commands.

READ THIS
     This documentation describes how to configure, generate, and
     install  the  PP  message  system.  It is intended only as a
     quick guide. The full details are in  the  PP  documentation
     set  -  _P_P  _M_a_n_u_a_l$  _I_n_s_t_a_l_l_a_t_i_o_n _a_n_d _O_p_e_r_a_t_i_o_n. This can be
     found in the doc/manual/volume1 directory

SYNOPSIS
     make

DESCRIPTION
     This is a description of how one can bring up the PP message
     system.   It  is assumed that you have super-user privileges
     in  order  to   (re-)install   the   software.    Super-user
     privileges  are  not  required to configure or generate this
     software.

     PP is normally installed with most programs and  directories
     owned by a single user. Conventionally this username is "pp"
     and is not a normal user id but this is  not  required.  You
     should normally do most of the compilation and configuration
     as this user.

     The distribution tape contains the hierarchy for the pp-6.0/
     directory.   Bring  the  sources  on-line  by  changing to a
     directory for local sources and running tar as the pp  user-
     name you have chosen, e.g.:

          % cd /usr/src/local/
          % tar xf _t_a_r_f_i_l_e
          % cd pp-6.0/

CONFIGURATION
     First, go to the config/ directory:

          % cd config

     Select the Makefile and include-file  skeletons  which  most
     closely match your system.  The current choices are:

          sun     Fairly standard sun configuration
          vax     Fairly standard vax configuration
          s5r4    System Five, release 4
          hpux    Hewlet Packard

          _m_o_r_e _s_a_m_p_l_e_s _w_e_l_c_o_m_e



Sun Release 4.1     Last change: 25 Nov 1989                    1






PP-GEN(8)             MAINTENANCE COMMANDS              PP-GEN(8)



     The makefile skeleton  has  the  extension  .make,  and  the
     include-file skeleton has the extension .h.

  MAKEFILE
     Copy the makefile skeleton of your  choice  to  pickle.make,
     where  "pickle"  is  the name of your system.  Now edit this
     file to set the _m_a_k_e variables.  There are  many  of  these,
     please  refer  to _T_h_e _P_P _M_a_n_u_a_l: _V_o_l_u_m_e _1 - _I_n_s_t_a_l_l_a_t_i_o_n _a_n_d
     _O_p_e_r_a_t_i_o_n. However, some of the more obvious  variables  are
     documented in ./OPTIONS.make.

     Link pickle.make to Make.defs in the base directory.

          % ln pickle.make ../Make.defs


  INCLUDE-FILE
     Copy the include-file skeleton of your choice  to  pickle.h,
     where  "pickle"  is  the  name  of your system.  Now add any
     additional definitions  you  may  need.   Consult  the  file
     ./OPTIONS.h for a list.

     Now, link pickle.h to ../h/config.h.

          % ln pickle.h ../h/config.h

GENERATION
     It is assumed that you  have  ISODE  7.0  installed  on  the
     machine  in  question,  including the X.500 libraries -ldsap
     -lisode. If you wish to build the  grey  book  channels  for
     JANET  mail,  you  should  first build and install the unix-
     niftp package.

     Go to the pp-6.0/ directory and generate the basic system:

          % cd ..
          % ./make


     This will cause a complete generation of the system.  If all
     goes well, proceed with the installation.  If not, complain,
     as there "should be no problems" at this step.   It  may  be
     worth  saving the output of the make stage in a file in case
     things do go wrong.

INSTALLATION
     You will need to be the super-user to install the  software.
     There are two kinds of activities: once-only activities that
     you perform the first time the software  is  installed;  and
     each-time   activities  that  you  perform  every  time  the
     software is installed.




Sun Release 4.1     Last change: 25 Nov 1989                    2






PP-GEN(8)             MAINTENANCE COMMANDS              PP-GEN(8)



     The first once-only activity  is  to  create  the  necessary
     basic  directories  to install the commands. This is done by
     issuing the command as super-user:

          # su
          # ./make dirs

     Note that the pp user id needs to  have  been  allocated  at
     this point.

     The second once-only activity is to make sure that  PP  will
     run when when the machine goes multi-user.  On Berkeley UNIX
     systems,  add   something   along   these   lines   to   the
     /etc/rc.local file:

          if [ -f /usr/lib/pp/pp.start ]; then
              /usr/lib/pp/pp.start & (echo -n ' pp') > /dev/console
          fi

     There are some examples of this start-up script in the  con-
     fig directory.

  NOTE
     If you wish to use smtp then it may be appropriate to  start
     the  smtp server for /_e_t_c/_i_n_e_t_d by adding a suitable line to
     /etc/inetd.conf. See Volume 1  for  more  details,  but  the
     basic format is something like:

         smtp stream tcp nowait  pp  /usr/lib/pp/cmds/chans/smtpd
            smtpd /usr/lib/pp/cmds/chans/smtpsrvr smtp


     To allow processes to contact the qmgr you must add  a  line
     to your isoentities file,

          pickle "pp qmgr" 1.17.6.2.1 #1001/Internet=pickle+18000

     You will need to replace "pickle" by your  local  host,  and
     you  may need to change the isomacro "Internet" to something
     else if you have a local ethernet or similar.

     Alternatively you may add a suitable entry to the  directory
     if  this  is  being  used  as a nameserver. Some samples are
     given below the examples directory.

     Then to install the following each-time activity is:

          % su
          Password:
          # ./make install





Sun Release 4.1     Last change: 25 Nov 1989                    3






PP-GEN(8)             MAINTENANCE COMMANDS              PP-GEN(8)



     If you plan to run the MTAconsole program (and you  should!)
     you will need to install the application defaults file. This
     is in the MTAconsole directory and it should be installed as
     MTAconsole  in  the app-defaults directory in your X hierar-
     chy. For instance:

          # cd Src/MTAconsole
          # cp MTAconsole.ad /usr/lib/X11/app-defaults/MTAconsole

     The same is true of the user  utility  xalert.  This  has  a
     similar   set   of   application  defaults  that  should  be
     installed.

     That's about it.  This will install everything.  To clean-up
     the source tree as well, then use:

          % make clean

     at this point. (However, unless you are very tight for space
     save this step until you are sure things are working.)

     If this is the first time you have used PP it is worth pick-
     ing  one  of  the  example  configurations  in the examples/
     directory. Choose a sample that is close  to  your  require-
     ments as a starting point. The available samples are

          LOCALSMTP  Very simple local smtp only
          JANET      A simple JANET configuration
          INTERNET   A simple Internet site

     Change to one of these  directories,  and  read  the  README
     there. This will tell you to do various things, depending on
     what configuration you have chosen. Remember, this  is  only
     an example and will not deal with all your needs.

     After all is set up, and at regular  intervals,  you  should
     check  all  is  ok  by  running _c_k_c_o_n_f_i_g (found in the tools
     directory) which will check things are installed correctly.

     Finally, if you are interested in discussing PP with  others
     running  the  software,  drop  a  note  to the Janet mailbox
     "pp-people-request@cs.ucl.ac.uk", and ask to be added to the
     "pp-people@cs.ucl.ac.uk" list.

TAILORING
     Tailoring is an essential part of running PP. This is  some-
     what  complex  to  achieve  the  correct  setup  and  so  is
     described in Volume 1 of the PP manual. It is important that
     this is followed carefully.

GENERATING DOCUMENTATION
     The directory doc/ contains the documentation set  for  this



Sun Release 4.1     Last change: 25 Nov 1989                    4






PP-GEN(8)             MAINTENANCE COMMANDS              PP-GEN(8)



     release.   Consult the file doc/READ-ME for a description of
     each document.  The directory  doc/ps/  contains  PostScript
     versions  of  each  document.  These can be used to generate
     standard documentation on PostScript printers, but users who
     want  to  preview the documentation should generate the .dvi
     files from the SLiTeX and LaTeX sources.

     If you received this distribution from the network, then the
     directory  doc/ps/  does  not  contain any PostScript files.
     There should be a separate compressed _t_a_r  file,  containing
     only  PostScript  files,  available on the machine where you
     retrieved this distribution.

REPORTING PROBLEMS
     Comments concerning this release should be directed  to  the
     authors.   Consult  the  preface  in the _U_s_e_r'_s _M_a_n_u_a_l for a
     current postal address.  Alternately, if you have access  to
     the  Janet  network,  comments  may  be  sent to the mailbox
     "pp-support@cs.ucl.ac.uk".  Do not send bug reports  to  the
     pp-people discussion group.

SEE ALSO
     ckconfig(8), dbmbuild(8)
     _T_h_e _P_P _M_a_n_u_a_l: _V_o_l_u_m_e _1 - _I_n_s_t_a_l_l_a_t_i_o_n _a_n_d _O_p_e_r_a_t_i_o_n































Sun Release 4.1     Last change: 25 Nov 1989                    5



