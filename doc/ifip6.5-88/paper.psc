%!              for use by dvi2ps Version 2.00
% a start (Ha!) at a TeX mode for PostScript.
% The following defines procedures assumed and used by program "dvi2ps"
% and must be downloaded or sent as a header file for all TeX jobs.

% By:  Neal Holtz, Carleton University, Ottawa, Canada
%      <holtz@cascade.carleton.cdn>
%      <holtz%cascade.carleton.cdn@ubc.csnet>
%      June, 1985
%      Last Modified: Aug 25/85
% oystr 12-Feb-1986
%   Changed @dc macro to check for a badly formed bits in character
%   definitions.  Can get a <> bit map if a character is not actually
%   in the font file.  This is absolutely guaranteed to drive the
%   printer nuts - it will appear that you can no longer define a
%   new font, although the built-ins will still be there.

% To convert this file into a downloaded file instead of a header
% file, uncomment all of the lines beginning with %-%

%-%0000000 			% Server loop exit password
%-%serverdict begin exitserver
%-%  systemdict /statusdict known
%-%  {statusdict begin 9 0 3 setsccinteractive /waittimeout 300 def end}
%-% if

/TeXDict 200 dict def   % define a working dictionary
TeXDict begin           % start using it.

                        % units are in "dots" (300/inch)
/Resolution 300 def
/Inch  {Resolution mul} def  % converts inches to internal units

/Mtrx 6 array def

%%%%%%%%%%%%%%%%%%%%% Page setup (user) options %%%%%%%%%%%%%%%%%%%%%%%%

% dvi2ps will output coordinates in the TeX system ([0,0] 1" down and in
% from top left, with y +ive downward).  The default PostScript system
% is [0,0] at bottom left, y +ive up.  The Many Matrix Machinations in
% the following code are an attempt to reconcile that. The intent is to
% specify the scaling as 1 and have only translations in the matrix to
% properly position the text.  Caution: the default device matrices are
% *not* the same in all PostScript devices; that should not matter in most
% of the code below (except for lanscape mode -- in that, rotations of
% -90 degrees resulted in the the rotation matrix [ e 1 ]
%                                                 [ 1 e ]
% where the "e"s were almost exactly but not quite unlike zeros.

% NOTE: We use A4 size paper. For letter size paper the constants '340' in the
% following code should be changed to '310'.

/@a4
{a4 initmatrix
72 Resolution div dup neg scale
270 -3215 translate
Mtrx currentmatrix pop
}def
/@letter
  { letter initmatrix
    72 Resolution div dup neg scale          % set scaling to 1.
    310 -3005 translate      % move origin to top (these are not exactly 1"
    Mtrx currentmatrix pop   % and -10" because margins aren't set exactly right)
  } def
        % note mode is like letter, except it uses less VM
/@note
  { note initmatrix
    72 Resolution div dup neg scale          % set scaling to 1.
    310 -3005 translate                      % move origin to top
    Mtrx currentmatrix pop
  } def

		% A3 modes courtesy of Francis Pintos, UCL

/@a3landscape
{a3 initmatrix
72 Resolution div dup neg scale
-90 rotate
300 310 translate
Mtrx currentmatrix pop
statusdict begin 1 setpapertray end
}def

/@a3portrait
{a3 initmatrix
72 Resolution div dup neg scale
300 310 translate
Mtrx currentmatrix pop
statusdict begin 1 setpapertray end
}def

/@landscape
  { letter initmatrix
    72 Resolution div dup neg scale          % set scaling to 1.
    -90 rotate                              % it would be nice to be able to do this
%    Mtrx currentmatrix 0 0.0 put             % but instead we have to do things like this because what
%    Mtrx 1 -1.0 put                          % should be zero terms aren't (and text comes out wobbly)
%    Mtrx 2 1.0 put                           % Fie!  This likely will not work on QMS printers
%    Mtrx 3 0.0 put                           % (nor on others where the device matrix is not like
%    Mtrx  setmatrix                          %  like it is on the LaserWriter).
    300 310  translate                       % move origin to top
    Mtrx currentmatrix pop
  } def

/@legal
  { legal initmatrix
    72 Resolution div dup neg scale          % set scaling to 1.
    295 -3880 translate                      % move origin to top
    Mtrx currentmatrix pop
  } def

/@manualfeed
   { statusdict /manualfeed true put
   } def
        % n @copies -   set number of copies
/@copies
   { /#copies exch def
   } def
/@restore /restore load def
/restore
   {vmstatus pop
   dup @VMused lt{pop @VMused}if		% calculate virtual memory used
   exch pop exch @restore /@VMused exch def
   }def

/@pri
    {
    ( ) print
    (                                       ) cvs print
    }def

/@FontMatrix [1 0 0 -1 0 0] def
/@FontBBox [0 0 1 1] def

%%%%%%%%%%%%%%%%%%%% Procedure Defintions %%%%%%%%%%%%%%%%%%%%%%%%%%

/@newfont       % id @newfont -         -- initialize a new font dictionary
  { /newname exch def
    newname 7 dict def          % allocate new font dictionary
    newname load begin
        /FontType 3 def
	/FontMatrix @FontMatrix def
	/FontBBox @FontBBox def
        /BitMaps 128 array def
        /BuildChar {CharBuilder} def
        /Encoding 128 array def
        0 1 127 {Encoding exch /.undef put} for
        end
    newname newname load definefont pop
  } def


% the following is the only character builder we need.  it looks up the
% char data in the BitMaps array, and paints the character if possible.
% char data  -- a bitmap descriptor -- is an array of length 6, of
%          which the various slots are:

/ch-image {ch-data 0 get} def   % the hex string image
/ch-width {ch-data 1 get} def   % the number of pixels across
/ch-height {ch-data 2 get} def  % the number of pixels tall
/ch-xoff  {ch-data 3 get} def   % number of pixels below origin
/ch-yoff  {ch-data 4 get} def   % number of pixels to left of origin
/ch-tfmw  {ch-data 5 get} def   % spacing to next character

/CharBuilder    % fontdict ch Charbuilder -     -- image one character
     {save 3 1 roll exch /BitMaps get exch get /ch-data exch def
      ch-data null ne
      {ch-tfmw 0 ch-xoff neg ch-yoff neg ch-width ch-xoff sub ch-height ch-yoff sub
            setcachedevice
        ch-width ch-height true [1 0  0 1  ch-xoff ch-yoff]
            {ch-image} imagemask
     }if
     restore
  } def


/@sf            % fontdict @sf -        -- make that the current font
  { dup
    % All smallcaps fonts must have the string SmallCaps in their name
    /FontName known
      { dup /FontName get tempstring cvs (SmallCaps) search
	  {/smallcaps true def pop pop pop}
	  {/smallcaps false def pop}
        ifelse
      }
      {/smallcaps false def} 
    ifelse
    setfont
  } def

                % in the following, the font-cacheing mechanism requires that
                % a name unique in the particular font be generated

/@dc            % char-data ch @dc -    -- define a new character bitmap in current font
  { /ch-code exch def
% ++oystr 12-Feb-86++
    dup 0 get
    length 2 lt
      { pop [ <00> 1 1 0 0 8.00 ] } % replace <> with null
    if
% --oystr 12-Feb-86--
    /ch-data exch def
    currentfont /BitMaps get ch-code ch-data put
    currentfont /Encoding get ch-code
       dup (   ) cvs cvn   % generate a unique name simply from the character code
       put
  } def

/@pc		% char-data ch @pc -    -- print a character bitmap not downloaded
    {pop
    /ch-data exch def
    currentpoint translate
    ch-width ch-height true [1 0 0 -1 ch-xoff ch-yoff]
    {ch-image}imagemask
    }def

/@bop0           % n @bop0 -              -- begin the char def section of a new page
  { pop
  } def

/@bop1           % n @bop1 -              -- begin a brand new page
  { pop
    erasepage initgraphics
    Mtrx setmatrix
    /SaveImage save def
%
% RLW:
% The following is a fix necessary since a /@beginspecial immediately
% following a /@bop1 failed.  This was due to a lack of definition of the
% current point, something which can only happen in the one situation.
% Suffer the extra code for all other case as new pages don't happen too often.
%
    0 0 moveto
%
  } def

/@eop           % - @eop -              -- end a page
  { showpage
    SaveImage restore
  } def

/@start         % - @start -            -- start everything
  { @a4                             % (there is not much to do)
    vmstatus pop /@VMused exch def pop
  } def

/@end           % - @end -              -- done the whole shebang
  {(VM used: ) print @VMused @pri
  (. Unused: ) print vmstatus @VMused sub @pri pop pop
  (\n) print flush
  end
  } def

/p              % x y p -               -- move to position
  { moveto
  } def

/r              % x r -                 -- move right
  { 0 rmoveto
  } def

/s              % string s -            -- show the string
  { smallcaps {SmallCapShow} {show} ifelse
  } def

/c              % ch c -                -- show the character (code given)
  { c-string exch 0 exch put
    c-string s
  } def

/c-string ( ) def

/ru             % dx dy ru -   -- set a rule (rectangle)
  { /dy exch neg def    % because dy is height up from bottom
    /dx exch def
    /x currentpoint /y exch def def   % remember current point
    newpath x y moveto
    dx 0 rlineto
    0 dy rlineto
    dx neg 0 rlineto
    closepath fill
    x y moveto
  } def

/l              % x y l -               -- draw line to
  { lineto
  } def

/rl             % dx dy rl -            -- draw relative line
  { rlineto
  } def

/rc             % x0 y0 x1 y1 y2 y2 rc  -- draw bezier curve
  { rcurveto
  } def

/np		% np -			-- start a new path and save currenpoint
  { /SaveX currentpoint /SaveY exch def def   % remember current point
    newpath
  } def

/st             % st -                  -- draw the last path and restore currentpoint
  { stroke
    SaveX SaveY moveto                  % restore the current point
  } def

/f              % f                     -- fill the last path and restore currentpoint
  { fill
    SaveX SaveY moveto                  % restore the current point
  } def

/ellipse        % xc yc xrad yrad startAngle endAngle ellipse
    {
        /endangle exch def
        /startangle exch def
        /yrad exch def
        /xrad exch def
        /y exch def
        /x exch def

        /savematrix matrix currentmatrix def

        x y translate
        xrad yrad scale
        0 0 1 startangle endangle arc
        savematrix setmatrix
    } def

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%     the \special command junk
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   The structure of the PostScript produced by dvi2ps for \special is:
%         @beginspecial
%           - any number of @hsize, @hoffset, @hscale, etc., commands
%         @setspecial
%           - the users file of PostScript commands
%         @endspecial

% The @beginspecial command recognizes whether the Macintosh Laserprep
% has been loaded or not, and redfines some Mac commands if so.
%%% NOTE: This has been disabled as we don't use it !!
% The @setspecial handles the users shifting, scaling, clipping commands


% The following are user settable options from the \special command.

/@SpecialDefaults
  { /hs 8.5 Inch def
    /vs 11.68 Inch def
    /ho 0 def
    /vo 0 def
    /hsc 1 def
    /vsc 1 def
    /CLIP false def
  } def

%       d @hsize -      specify a horizontal clipping dimension
%                       these 2 are executed before the MacDraw initializations
/@hsize {/hs exch def /CLIP true def} def
/@vsize {/vs exch def /CLIP true def} def
%       d @hoffset -    specify a shift for the drwgs
/@hoffset {/ho exch def} def
/@voffset {/vo excl def} def
%       s @hscale -     set scale factor
/@hscale {/hsc exch def} def
/@vscale {/vsc exch def} def

/@setclipper
  { hsc vsc scale
    CLIP
      { newpath 0 0 moveto hs 0 rlineto 0 vs rlineto hs neg 0 rlineto closepath clip }
    if
  } def

% this will be invoked as the result of a \special command (for the
% inclusion of PostScript graphics).  The basic idea is to change all
% scaling and graphics back to defaults, but to shift the origin
% to the current position on the page.  Due to TeXnical difficulties,
% we only set the y-origin.  The x-origin is set at the left edge of
% the page.

/@beginspecial          % - @beginspecial -     -- enter special mode
  { gsave /SpecialSave save def
          % the following magic incantation establishes the current point as
          % the users origin, and reverts back to default scalings, rotations
    currentpoint transform initgraphics itransform translate
    @SpecialDefaults    % setup default offsets, scales, sizes
%%%    @MacSetUp           % fix up Mac stuff  -- DISABLED
  /showpage {} def %%% Ignore showpage commands
  } def

/@setspecial    % to setup user specified offsets, scales, sizes (for clipping)
  {
    MacDrwgs
      {md begin /pxt ho def /pyt vo neg def end}
      {ho vo translate @setclipper}
    ifelse
  } def

/@endspecial            % - @endspecial -       -- leave special mode
  { SpecialSave restore
    grestore
  } def

%!
% All software, documentation, and related files in this distribution of
% psfig/tex are Copyright (c) 1987 Trevor J. Darrell
%
% Permission is granted for use and non-profit distribution of psfig/tex 
% providing that this notice be clearly maintained, but the right to
% distribute any portion of psfig/tex for profit or as part of any commercial
% product is specifically reserved for the author.
%
%
% psfigTeX PostScript Prolog
% $Header: tex.ps,v 1.13 87/12/14 00:57:00 van Exp $
%
/psf$TeXscale { 65536 div } def

/DocumentInitState [ matrix currentmatrix currentlinewidth currentlinecap
currentlinejoin currentdash currentgray currentmiterlimit ] cvx def

%  x y bb-llx bb-lly bb-urx bb-ury startFig -
/startTexFig {
	/psf$SavedState save def
	userdict maxlength dict begin
	currentpoint transform

	DocumentInitState setmiterlimit setgray setdash setlinejoin setlinecap
		setlinewidth setmatrix

	itransform moveto

	/psf$ury exch psf$TeXscale def
	/psf$urx exch psf$TeXscale def
	/psf$lly exch psf$TeXscale def
	/psf$llx exch psf$TeXscale def
	/psf$y exch psf$TeXscale def
	/psf$x exch psf$TeXscale def
	
	currentpoint /psf$cy exch def /psf$cx exch def

	/psf$sx psf$x psf$urx psf$llx sub div def 	% scaling for x
	/psf$sy psf$y psf$ury psf$lly sub div def	% scaling for y

	psf$sx psf$sy scale			% scale by (sx,sy)

	psf$cx psf$sx div psf$llx sub
	psf$cy psf$sy div psf$ury sub translate
	
	/DefFigCTM matrix currentmatrix def

	/initmatrix {
		DefFigCTM setmatrix
	} def
	/defaultmatrix {
		DefFigCTM exch copy
	} def

	/initgraphics {
		DocumentInitState setmiterlimit setgray setdash 
			setlinejoin setlinecap setlinewidth setmatrix
		DefFigCTM setmatrix
	} def

	/showpage {
		initgraphics
	} def
	@MacSetUp
} def

% llx lly urx ury doclip -	(args in figure coordinates)
/doclip {
	currentpoint 6 2 roll
	newpath 4 copy
	4 2 roll moveto
	6 -1 roll exch lineto
	exch lineto
	exch lineto
	closepath clip
	newpath
	moveto
} def

% - endTexFig -
/endTexFig { end psf$SavedState restore } def



%%%% Additions by LA Carr to reencode Adobe fonts as TeX fonts (almost)
%%%% Based on routine in LaserWriter Cookbook
/ReEncodeForTeX
  { /newfontname exch def
    /basefontname exch def
    /TeXstr 30 string def

    /basefontdict basefontname findfont def
    /newfont basefontdict maxlength dict def
    basefontdict
      { exch dup /FID ne
	  { dup /Encoding eq
	    { exch dup length array copy
	        newfont 3 1 roll put }
	    { exch newfont 3 1 roll put }
	    ifelse
	  }
	{ pop pop }
	ifelse
      } forall

%%    newfont /FontName newfontname put %%
      basefontname TeXstr cvs (Dingbat) search
	{ pop pop pop }
        { pop
	  /TeXvec basefontname TeXstr cvs (Courier) search
	    { pop pop pop TeXcourvec }
	    { pop TeXnormalvec }
	  ifelse def
          TeXvec aload pop

          TeXvec length 2 idiv
	    { newfont /Encoding get 3 1 roll put }
	  repeat
	}
      ifelse

      newfontname newfont definefont pop
  } def

/TeXnormalvec
	[ 8#014 /fi 8#015 /fl 8#020 /dotlessi 8#022 /grave 8#023 /acute
	  8#024 /caron 8#025 /breve 8#026 /macron 8#027 /ring 8#030 /cedilla
	  8#031 /germandbls 8#032 /ae 8#033 /oe 8#034 /oslash 8#035 /AE
	  8#036 /OE 8#037 /Oslash 8#042 /quotedblright 8#074 /exclamdown
	  8#076 /questiondown 8#134 /quotedblleft 8#136 /circumflex
	  8#137 /dotaccent 8#173 /endash 8#174 /emdash 8#175 /quotedbl
	  8#177 /dieresis ] def
/TeXcourvec
	[ 8#016 /exclamdown 8#017 /questiondown 8#020 /dotlessi 8#022 /grave
	  8#023 /acute 8#024 /caron 8#025 /breve 8#026 /macron 8#027 /ring
	  8#030 /cedilla 8#031 /germandbls 8#032 /ae 8#033 /oe 8#034 /oslash
	  8#035 /AE 8#036 /OE 8#037 /Oslash 8#074 /less 8#076 /greater
	  8#134 /backslash 8#136 /circumflex 8#137 /underscore 8#173 /braceleft
	  8#174 /bar 8#175 /braceright 8#177 /dieresis ] def

/TeXPSmakefont {	% defines a routine for generating PS fonts, fudged!
	/TeXsize exch def
	findfont 
	[ TeXsize 0 0 TeXsize neg 0 0 ] makefont
	} def

%Create a General Oblique font
/ObliqueFont {
    /ObliqueAngle exch def
    /ObliqueBaseName exch def
    /ObliqueFontName exch def
    /ObliqueTransform [1 0 ObliqueAngle sin ObliqueAngle cos div 1 0 0] def
    /basefontdict ObliqueBaseName findfont ObliqueTransform makefont def
    /newfont basefontdict maxlength dict def
    basefontdict
      { exch dup /FID ne
	  { dup /Encoding eq
	    { exch dup length array copy
	        newfont 3 1 roll put }
	    { exch newfont 3 1 roll put }
	    ifelse
	  }
	{ pop pop }
	ifelse
      } forall

     newfont /FontName ObliqueFontName put
     ObliqueFontName newfont definefont
     pop
     } def

/Times-Oblique /Times-Roman 15.5 ObliqueFont
/Times-BoldOblique /Times-Bold 15 ObliqueFont
%/Palatino-Oblique /Palatino-Roman 10 ObliqueFont
%/Palatino-BoldOblique /Palatino-Bold 10 ObliqueFont
/Times-ItalicUnslanted /Times-Italic -15.15 ObliqueFont

%Create a Palatino-ItalicUnslanted font? You must be joking!

%Create a General SmallCaps font
/SmallCapsFont {
    /SmallCapsBaseName exch def
    /SmallCapsFontName exch def
    /basefontdict SmallCapsBaseName findfont def
    /newfont basefontdict maxlength dict def
    basefontdict
      { exch dup /FID ne
	  { dup /Encoding eq
	    { exch dup length array copy
	        newfont 3 1 roll put }
	    { exch newfont 3 1 roll put }
	    ifelse
	  }
	{ pop pop }
	ifelse
      } forall

     newfont /FontName SmallCapsFontName put
     SmallCapsFontName newfont definefont
     pop
     } def

/Times-SmallCaps /Times-Roman SmallCapsFont
%/Palatino-SmallCaps /Palatino-Roman SmallCapsFont

/SmallCapShow {
	% string smallcaps show
	/achar (A) def
	/xfac 0.8 def
	/yfac 0.8 def
	/xrec 1 xfac div def
	/yrec 1 yfac div def
	{ dup dup
	  8#141 ge exch 8#172 le and 
	    { 8#40 sub achar exch 0 exch put achar
	      xfac yfac scale show xrec yrec scale }
	    { achar exch 0 exch put achar show }
	  ifelse
	} forall
} def

/tempstring 100 string def	% used for string conversions
%%%% Additions by LA Carr to reencode Adobe fonts as TeX fonts (almost)
%%%% Based on routine in LaserWriter Cookbook

/MacDrwgs false def     % will get set if we think the Mac LaserPrep file has been loaded

        % - @MacSetUp -   turn-off/fix-up all the MacDraw stuff that might hurt us
                        % we depend on 'psu' being the first procedure executed
                        % by a Mac document.  We redefine 'psu' to adjust page
                        % translations, and to do all other the fixups required.
                        % This stuff will not harm other included PS files
/@MacSetUp
  { userdict /md known  % if md is defined
      { userdict /md get type /dicttype eq      % and if it is a dictionary
         { /MacDrwgs true def
           md begin                             % then redefine some stuff
              /psu                              % redfine psu to set origins, etc.
                /psu load
                        % this procedure contains almost all the fixup code
                { /letter {} def        % it is bad manners to execute the real
                  /note {} def          %   versions of these (clears page image, etc.)
                  /legal {} def
                  statusdict /waittimeout 300 put
                  /page {pop} def       % no printing of pages
                  /pyt vo neg def       % x & y pixel translations
                  /pxt ho def
                }
                concatprocs
              def
              /od                               % redefine od to set clipping region
                /od load
                { @setclipper }
                concatprocs
              def
           end }
        if }
    if
  } def

%       p1 p2 concatprocs p       - concatenate procedures
/concatprocs
  { /p2 exch cvlit def
    /p1 exch cvlit def
    /p p1 length p2 length add array def
    p 0 p1 putinterval
    p p1 length p2 putinterval
    p cvx
  } def

end                     % revert to previous dictionary

statusdict /waittimeout 300 put
TeXDict begin @start
%%Title: paper-t.dvi
%%Creator: dvi2ps
%%EndProlog
13 @bop0
/cmr10.329 @newfont
cmr10.329 @sf
[<FFFE03F8FFFE0FFC07C01F8E07C03F0607C03E0607C03E0007C03E0007C03E0007C03E0007C03E0007C03E0007C03E0007C0
  7E0007C07C0007C0F80007FFF00007FFF80007C07C0007C01E0007C01F0007C00F0007C00F8007C00F8007C00F8007C00F80
  07C00F0007C01F0007C01E0007C07C00FFFFF800FFFFC000> 31 31 -1 0 33] 82 @dc
[<03F8000FFE001F87003E03807C0180780000F80000F00000F00000F00000FFFF80FFFF80F00380F007807807807807003C0F
  001E1E000FFC0003F000> 17 20 -1 0 20] 101 @dc
[<CFC0FFE0F8F0F078E038C038C07800F807F83FF07FF07FC0FF00F000E030E030E07070F07FF01FB0> 13 20 -2 0 18] 115 @dc
[<1F83C03FE7E07C7FF0F81F30F01F30F00F30F00F30F80F007C0F003E0F000FCF0003FF00000F00000F00380F007C1F007C1E
  007C7E007FFC001FF000> 20 20 -2 0 23] 97 @dc
[<FFF0FFF00F000F000F000F000F000F000F000F000F000F000F000F000F0E0F9F0F9FFFDFFFFF0F7C> 16 20 0 0 18] 114 @dc
[<03F00FFC1F0E3E077C037800F800F000F000F000F000F000F000F000781C783E3C3E1E3E0FFE03F8> 16 20 -2 0 20] 99 @dc
[<FFF3FFFFF3FF0F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F80F00F80F00FC1
  F00F61E00F3FE00F1F800F00000F00000F00000F00000F00000F00000F00000F00000F0000FF0000FF00000F0000> 24 32 0 0 25] 104 @dc
[<FFF000C0FFF001C00F0003C0060003C0060007C0060007C006000FC006001FC006001FC006003FC006007EC006007EC00600
  FCC00601F8C00601F8C00603F0C00603E0C00607E0C0060FC0C0060FC0C0061F80C0063F00C0063F00C0067E00C006FC00C0
  06FC00C007F800C007F800C007F001E0FFE01FFEFFE01FFE> 31 31 -1 0 34] 78 @dc
[<01F80007FE001E07803C03C03801C07801E07000E0F000F0F000F0F000F0F000F0F000F0F000F07000E07000E03801C03C03
  C01E078007FE0001F800> 20 20 -1 0 23] 111 @dc
[<07E00FF01F301E181E181E181E181E181E001E001E001E001E001E001E001E001E001E00FFF8FFF83E001E000E000E000600
  060006000600> 13 28 -1 0 18] 116 @dc
[<C000C000E000600060007000300030003800180018001C000C000C000E0006000600070003000300038001800180018001C0
  00C000C000E000600060007000300030003800180018001C000C000C000E00060006000700030003> 16 45 -3 11 23] 47 @dc
[<07F0001FFC003E1F00780780700380F001C0E001C0E001C0E003C0E007C0F00FC0781FC03C7F801FFF000FFF000FFC001FF8
  003FFE003FCF007F07007C0780780380700380700380700780380F001E1F000FFE0003F800> 18 29 -2 0 23] 56 @dc
[<07F0003FFC007C3F00F81F00FC0F80FC07C0FC07C0FC07C07807C00007C00007C0000780000F80001F00003E0003F00003F8
  00007E00001F00000F00000F807C0F807C0F807E0F807C0F807C0F003C3F001FFE0007F000> 18 29 -2 0 23] 51 @dc
[<7FFF7FFF03C003C003C003C003C003C003C003C003C003C003C003C003C003C003C003C003C003C003C003C003C003C0F3C0
  FFC00FC001C000C0> 16 29 -3 0 23] 49 @dc
[<FEFEC0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0FEFE> 7 45 -4 11 13] 91 @dc
[<FFFF80FFFF80FFFF807FFF803801C01C00C00E00C00780C003C00001E00000F000007800003C00003E00001F00000F80000F
  800007C00007C03007C0FC03C0FC07C0FC07C0FC07C0F80F80701F80783F001FFE0007F000> 18 29 -2 0 23] 50 @dc
[<FEFE0606060606060606060606060606060606060606060606060606060606060606060606060606060606FEFE> 7 45 -1 11 13] 93 @dc
[<001FE00000FFF80003F83C0007E00E000F8007001F0003801E0001803E0001C07C0000C07C0000C07C0000C0F8000000F800
  0000F8000000F8000000F8000000F8000000F8000000F8000000F80000C07C0000C07C0000C07C0001C03E0001C01E0003C0
  1F0003C00F8007C007E00FC003F83DC000FFF8C0001FC0C0> 26 31 -3 0 33] 67 @dc
[<FFFEFFFE07C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C0
  07C007C007C007C0FFFEFFFE> 15 31 0 0 16] 73 @dc
[<07FFFF0007FFFF00000F8000000F8000000F8000000F8000000F8000000F8000000F8000000F8000000F8000000F8000000F
  8000000F8000000F8000000F8000000F8000000F8000000F8000000F8000C00F8030C00F8030C00F8030C00F8030E00F8070
  600F8060600F8060700F80E07C0F81E07FFFFFE07FFFFFE0> 28 31 -2 0 33] 84 @dc
[<C3F800EFFE00FE1F00F80780F00780E00380C003C0C003C0C003C00003C00007C00007C0001FC000FF800FFF801FFF003FFE
  007FFC007FE000FE0000FC0000F80000F00180F00180F00380F00380780780780F803C3F801FFF8007F180> 18 31 -3 0 25] 83 @dc
[<001FF06000FFFCE001FC1FE007E007E00FC003E01F0003E01F0003E03E0003E07C0003E07C0003E07C0003E0F800FFFCF800
  FFFCF8000000F8000000F8000000F8000000F8000000F8000000F80000607C0000607C0000607C0000E03E0000E01F0001E0
  1F0001E00F8003E007E00FE001F81EE000FFF860001FE060> 30 31 -3 0 36] 71 @dc
[<07F0001FFC007C3E00701F00F80F80F80780F807C0F807C0F807C00007C00007C00007C0000780380F803C0F003E1F003FFC
  0033F8003000003000003000003000003000003000003FF0003FFC003FFE003FFF00380700> 18 29 -2 0 23] 53 @dc
[<0003800000038000000380000007C0000007C000000FE000000FE000000FE000001FB000001F3000001F3000003E1800003E
  1800003E1800007C0C00007C0C0000FC0E0000F8060000F8060001F8030001F0030001F0030003E0018003E0018003E00180
  07C000C007C000C00FC000E00FC001F0FFF807FEFFF807FE> 31 31 -1 0 34] 86 @dc
[<60F07038181C0C0C0C7CFCFCF870> 6 14 -4 9 13] 44 @dc
[<381C7C3EFC7EFC7EF87CC060C060C060E07060307038381C3C1E180C> 15 14 -5 -18 23] 92 @dc
[<FFF3FF9FFCFFF3FF9FFC0F007803C00F007803C00F007803C00F007803C00F007803C00F007803C00F007803C00F007803C0
  0F007803C00F007803C00F007803C00F007803C00F807C03C00F807C03C00FC0FE07C0FF71F38F80FF3FF1FF800F1FC0FE00> 38 20 0 0 39] 109 @dc
[<FFF3FFFFF3FF0F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F80F00F80F00FC1
  F0FF61E0FF3FE00F1F80> 24 20 0 0 25] 110 @dc
[<07E3FC1FFBFC3E1FC03C0FC07807C07803C0F003C0F003C0F003C0F003C0F003C0F003C0F003C0F003C07803C07803C03C07
  C01E1FC00FFFC007F3C00003C00003C00003C00003C00003C00003C00003C00003C00003C0003FC0003FC00003C0> 22 32 -2 0 25] 100 @dc
[<FFE0FFE00F000F000F000F000F000F000F000F000F000F000F000F000F000F000F007F007F000F0000000000000000000000
  00000E001F001F001F000E00> 11 31 0 0 13] 105 @dc
[<FFF07FFEFFF07FFE0FC00FE003800FC001800F8001C01F8000C03F0000E03E0000607E0000307C000038FC000019F800000D
  F000000FF0000007E0000007E000000FC000000F8000001FC000001FC000003FE000007E6000007C300000FC380000F81800
  01F80C0003F00E0003E00F0007F01F807FFC7FF87FFC7FF8> 31 31 -1 0 34] 88 @dc
[<70F8F8F870> 5 5 -4 0 13] 46 @dc
[<01FFE001FFE0001E00001E00001E00001E00001E00001E00FFFFF0FFFFF0E01E00601E00301E00381E00181E000C1E000E1E
  00061E00031E00039E00019E0000DE0000FE00007E00003E00003E00001E00000E00000E00> 20 29 -1 0 23] 52 @dc
[<03F0000FFC001E1E003C0F00380700780780700380700380F003C0F003C0F003C0F003C0F003C0F003C0F003C0F003C0F003
  C0F003C0F003C0F003C0F003C07003807003807807803807003C0F001E1E000FFC0003F000> 18 29 -2 0 23] 48 @dc
[<6030F0787038381C180C1C0E0C060C060C067C3EFC7EFC7EF87C7038> 15 14 -2 -18 23] 34 @dc
[<FFF060FFFEFFF0F0FFFE0F00F007C00600F007C00601F807C00601F807C00601F807C00603CC07C00603CC07C00603CC07C0
  06078607C006078607C006078607C0060F0307C0060F0307C0060F0307C0061E0187C0061E0187C0063C00C7C0063C00C7C0
  063C00C7C006780067C006780067C006780067C006F00037C006F00037C006F00037C007E0001FC007E0001FC0FFE0001FFE
  FFC0000FFE> 39 31 -1 0 42] 77 @dc
[<03FC001FFF803E07C07801E0F000F0E00070E00070E00070F000F07803F03FFFE01FFFE03FFF803FFE003800003000003000
  0037F0003FFC003E3E003C1E00780F00780F00780F00780F00780F003C1E703E3EF01FFFF007F1E0> 20 30 -1 10 23] 103 @dc
[<FFFC7FFEFFFC7FFE07C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C0
  07C007C007C007C007C007FFFFC007FFFFC007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C007C0
  07C007C007C007C007C007C007C007C0FFFC7FFEFFFC7FFE> 31 31 -1 0 34] 72 @dc
[<FFF0FFF00F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F00
  0F000F000F000F00FF00FF000F00> 12 32 0 0 13] 108 @dc
[<3C00007F0000E38000C18000F8C00070C00000600000600000600000700000700000700000F80000F80001FC0001EC0001EC
  0003EE0003C60003C6000783000783000783000F01800F01800F01C01F03E0FFC7F8FFC7F8> 21 29 -1 9 24] 121 @dc
[<FFC0FFC0FFC0> 10 3 -1 -9 15] 45 @dc
[<70F8F8F8700000000000000000000070F8F8F870> 5 20 -4 0 13] 58 @dc
[<00700000700000700000F80000F80001FC0001EC0001EC0003EE0003C60003C6000783000783000783000F01800F01800F01
  C01F03E0FFC7F8FFC7F8> 21 20 -1 0 24] 118 @dc
[<FFFFFFC0FFFFFFC007C007C007C001E007C000E007C000E007C0006007C0006007C0006007C0003007C0603007C0603007C0
  600007C0600007C0E00007FFE00007FFE00007C0E00007C0600007C0600007C060C007C060C007C000C007C000C007C001C0
  07C0018007C0018007C0038007C00F80FFFFFF80FFFFFF80> 28 31 -1 0 31] 69 @dc
[<003FC00000FFF00003E07C0007C03E000F801F001F000F801E0007803E0007C07C0003E07C0003E07C0003E0F80001F0F800
  01F0F80001F0F80001F0F80001F0F80001F0F80001F0F80001F0F80001F0780001E07C0003E03C0003C03C0003C01E000780
  1E0007800F000F0007801E0003E07C0000FFF000003FC000> 28 31 -3 0 35] 79 @dc
[<0C3F800E7FC00FE1E00F80F00F80780F00780F003C0F003C0F003C0F003C0F003C0F003C0F003C0F003C0F00780F00780F80
  F00FE1F00FFFE00F1F800F00000F00000F00000F00000F00000F00000F00000F00000F0000FF0000FF00000F0000> 22 32 0 0 25] 98 @dc
[<0FC0003FF000787C007C1C007C0E007C0F007C07000007800007800003800183C00FF3C01FFBC03C1FC0780FC07007C0F007
  C0F003C0F003C0F003C0F003C0F00380F003807007807807003C0F001E1E000FFC0003F000> 18 29 -2 0 23] 57 @dc
[<FFFFF000FFFFFE0007C01F0007C0078007C003C007C001E007C000F007C000F007C000F807C0007807C0007807C0007C07C0
  007C07C0007C07C0007C07C0007C07C0007C07C0007C07C0007C07C0007C07C0007807C0007807C000F807C000F007C001F0
  07C001E007C003C007C0078007C01F00FFFFFE00FFFFF000> 30 31 -1 0 35] 68 @dc
[<00E01C0000E01C0000F03C0001F03E0001F03E0001F87E0003F87F0003D87B0003D8730003CCF300078CF180078CE1800787
  E1800F07E0C00F07C0C00F03C0C01E03C0E01E07C0F0FF9FF3FCFF9FF3FC> 30 20 -1 0 33] 119 @dc
[<FFE07FFEFFE07FFE0F800FC0070007C003000F8003000F8003000F8001801F0001801F0001FFFF0000FFFE0000C03E0000C0
  3E0000607C0000607C0000607C000030F8000030F8000030F8000019F0000019F0000019F000000FE000000FE000000FE000
  0007C0000007C0000007C000000380000003800000038000> 31 31 -1 0 34] 65 @dc
[<FFF000FFF0000F00000F00000F00000F00000F00000F00000F00000F3F800F7FC00FE3E00F80F00F80F80F00780F007C0F00
  3C0F003C0F003C0F003C0F003C0F003C0F007C0F00780F00F80F80F0FFE1F0FFFFE00F1F80> 22 29 0 9 25] 112 @dc
[<FFE7FCFFE7FC0F03E00F07C00F07C00F0F800F1F800F1F000FBE000FFE000FFC000FFC000F78000F3C000F1E000F0F000F07
  800F07C00F0FF80F0FF80F00000F00000F00000F00000F00000F00000F00000F00000F0000FF0000FF00000F0000> 22 32 0 0 24] 107 @dc
[<FFF000FFF0000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00
  000F0000FFF000FFF0000F00000F00000F00000F00000F00000F00000F0F800F8F8007CF8003EF8001FF00007E00> 17 32 0 0 14] 102 @dc
[<FFFF0000FFFF000007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C0600007C0600007C0
  600007C0600007C0E00007FFE00007FFE00007C0E00007C0600007C0600007C060C007C060C007C000C007C000C007C001C0
  07C0018007C0018007C0038007C00F80FFFFFF80FFFFFF80> 26 31 -1 0 30] 70 @dc
[<FFFE0000FFFE000007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C0
  000007C0000007FFF00007FFFC0007C03E0007C00F0007C00F8007C0078007C007C007C007C007C007C007C007C007C007C0
  07C0078007C00F8007C00F0007C03E00FFFFFC00FFFFF000> 26 31 -1 0 31] 80 @dc
[<FFCFFEFFCFFE1F83E00703C003878001878001CF0000FE00007E00003C0000780000780000FC0001EE0001E60003C3000783
  800F83E07FE7FC7FE7FC> 23 20 0 0 24] 120 @dc
[<01F8FF07FEFF0F87F00F03F00F01F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00
  F0FF0FF0FF0FF00F00F0> 24 20 0 0 25] 117 @dc
[<FFFE3FFEFFFE3FFE07C00FF007C007E007C00FC007C00F8007C01F8007C03F0007C03F0007C07E0007C07C0007C0FC0007E1
  F80007F1F80007FBF00007DFE00007CFE00007C7C00007C3C00007C1C00007C0E00007C0700007C0780007C03C0007C01E00
  07C00F0007C0078007C003C007C003F0FFFE0FFEFFFE0FFE> 31 31 -1 0 35] 75 @dc
/cmsl10.329 @newfont
cmsl10.329 @sf
[<00FE000007FF80000F83C0001F00E0001E0070003E0038003C001C003C000C003E000C003E000E003E0006003E0006003E00
  06003E0006003F0007001F0007001F0003001F0003001F0003001F0003001F8003800F8003800F8001800F8001800F800180
  0F8001800FC001C007C001C007C003E0FFFE1FFEFFFE1FFE> 31 31 -6 0 34] 85 @dc
[<CFC0FFF0F8787038701C601C601C003C01FC0FFC1FF81FF03FC03E003C0E1C0E1C0E0E1E0FFE03FE> 15 20 -2 0 18] 115 @dc
[<0FE0001FF8003E1E00780600F00700F00000F00000E00000F00000F00000FFFF80FFFF80F003807803807803803C07801E07
  000F0F0007FE0001FC00> 17 20 -3 0 20] 101 @dc
[<FFE7FEFFE7FE1F01F00F01F00F00F00F00F00F00F00F00F00F80F80780F807807807807807807807C07807C07803E07807F0
  783FF8781FCFF803C7F0> 23 20 -1 0 25] 110 @dc
[<FFC0FFC01F000F000F000F000F000F000F800780078007800780078007C003C007C01FC01FC001C000000000000000000000
  000001E001F001F001F000E0> 12 31 -1 0 13] 105 @dc
[<FF9FFCFF8FFC1F07C00707C0030780018F8000CF0000FF00007E00003E00003C00007C00007E0000FB0000F38001F1C001E0
  E003E0F81FF9FF1FF9FF> 24 20 -1 0 24] 120 @dc
[<007F800003FFE00007E078000F801C001F000E003E0007003C0003807C0003807C000180780001C0F80000C0F8000000F800
  0000F8000000FC0000007C0000007C0000007C0000007C0000003E0000603E0000703F0000701F0000700F80007007C000F0
  07E000F003F001F800F803F8007F07F8001FFE380003F818> 29 31 -5 0 33] 67 @dc
[<07E0001FFC003C1E00700700F00380F003C0E001C0E001E0E001E0F000F0F000F0F000F0F000F07000F03800E03C00E01E01
  C00F038003FF0000FC00> 20 20 -3 0 23] 111 @dc
[<FFF000FFF0001F00001F00000F00000F00000F00000F00000F800007800007800007800007800007800007C00003C00003C0
  0003C0003FFC003FFC0003E00001E00001E00001E00001E00001E00001F0F000F0F80078F8003EF8001FF00007F0> 21 32 -1 0 14] 102 @dc
[<FFF000FFF0001F00000F00000F00000F00000F00000F00000F800007800007800007800007800007C00007C78003C7C007E7
  C03FF7C01FFFC003DF00> 18 20 -1 0 18] 114 @dc
[<0FE03FF87C3C780CF00EF000F000E000F000F000F000F000F0007800781E3C1F1E1F0F1F07FF01FC> 16 20 -4 0 20] 99 @dc
cmr10.329 @sf
[<FFFFFEFFFFFE07C07E07C01E07C00E07C00607C00707C00707C00307C00307C00307C00307C00007C00007C00007C00007C0
  0007C00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C000FFFF00FFFF00> 24 31 -1 0 28] 76 @dc
[<00E001E003C0038007000F000E001E001C003C00380038007800780070007000F000F000F000F000F000F000F000F000F000
  F000F000F000F000F0007000700078007800380038003C001C001E000E000F000700038003C001E000E0> 11 46 -3 12 18] 40 @dc
[<E000F000780038001C001E000E000F00070007800380038003C003C001C001C001E001E001E001E001E001E001E001E001E0
  01E001E001E001E001E001C001C003C003C003800380078007000F000E001E001C0038007800F000E000> 11 46 -3 12 18] 41 @dc
[<03F8000FFC001E1E003C0F003807807803807803C07003C0F003C0F003C0F003C0F003C0F803C0F80380FC0780FE0F00F7FE
  00F3FC00F06000700000780000780000380F803C0F801E0F800F0F8007878003FF0000FE00> 18 29 -2 0 23] 54 @dc
[<03800007C00007C00007C00007C00007C00007C00007C00007C00003C00003C00003C00003C00001C00001E00000E00000E0
  00006000007000003800001800C01C00C00E00C00600E007007FFF807FFFC07FFFC07FFFC0600000> 18 30 -3 0 23] 55 @dc
[<000FF000003FFC00007C1E0000F0070001E0038003E0018003C0018007C001C007C000C007C000C007C000C007C000C007C0
  00C007C000C007C000C007C000C007C000C007C000C007C000C007C000C007C000C007C000C007C000C007C000C007C000C0
  07C000C007C000C007C000C007C001E0FFFE1FFEFFFE1FFE> 31 31 -1 0 34] 85 @dc
cmsl10.329 @sf
[<FFFE00007FFE000007E0000003E0000003E0000003E0000003E0000003E0000003F0000001F0000001F0000001F0000001F0
  000001F0000001FFFC0000FFFF0000F807C000F801E000F800F000F800F800FC00F8007C007C007C007C007C007C007C007C
  007C007C007E0078003E00F8003E01F007FFFFE007FFFF80> 30 31 -1 0 31] 80 @dc
[<0FC7F81FFFF83C3FC0780FC07807C07003C07003C0F003C0F803E07803E07801E07801E07801E03C01E03C01F01E01F01F01
  F00F87F003FFF001FCF00000F80000F800007800007800007800007800007C00007C00007C0003FC0001FC00003C> 22 32 -3 0 25] 100 @dc
[<07FC001FFF003E0FC07801E0F000F0F00070F000707000787800783C01F01FFFF00FFFF01FFFE01FFF801E00001C00000C00
  000CFC0007FF000787800F03C00F01E00F01E00F01E00F81E00781E007C1E703E3EF01FFFF007F1F> 24 30 0 10 23] 103 @dc
cmr10.329 @sf
[<FFFFF800FFFFFE0007C01F0007C00F8007C007C007C003C007C003E007C003E007C003E007C003E007C003E007C003C007C0
  07C007C0078007C01F0007FFFE0007FFFC0007C03E0007C01F0007C00F8007C0078007C007C007C007C007C007C007C007C0
  07C007C007C00F8007C00F0007C01F00FFFFFC00FFFFF000> 27 31 -1 0 32] 66 @dc
[<1FC0003FF00078FC00FC7C00FC3E00FC3E00FC3E00783E00003E00003E00003E00003E00003E00003E00003E00003E00003E
  00003E00003E00003E00003E00003E00003E00003E00003E00003E00003E00003E00003E000FFFE00FFFE0> 19 31 -1 0 23] 74 @dc
cmsl10.329 @sf
[<1FC0007FF000F1F800F8FC00FC7E00FC3E00FC3E00783E00003F00003F00001F00001F00001F00001F00001F80001F80000F
  80000F80000F80000F80000FC0000FC00007C00007C00007C00007C00007E00007E00007E000FFFC00FFFE> 23 31 -2 0 23] 74 @dc
[<FFF000C0007FF001C0000F8001E000070003E000030003E000030003E000030007E000030007E00003800FF00001800FF000
  01801FB00001801F300001803F300001803F300001C07E380000C07E380000C0FC180000C0FC180000C1F8180000C1F81800
  00E1F01C000063F01C000063E00C000067E00C000067C00C00006FC00C00007F800E00003F800E00003F801F0007FF00FFF0
  07FF00FFF0> 36 31 -1 0 34] 78 @dc
[<3FFFF8001FFFF800007E0000003E0000003E0000003E0000003E0000003E0000003F0000001F0000001F0000001F0000001F
  0000001F0000001F8000000F8000000F8000000F8000000F8000000F8000C00FC0306007C0306007C0386007C0187007C018
  3007C0383807E0383803E0381E03E0F81FFFFFF81FFFFFF8> 29 31 -5 0 33] 84 @dc
[<FFF060FFFE007FF0707FFE000F807007E0000700F807E0000300F803E0000300FC03E0000300FC03E0000300F603E0000381
  E603F0000181E303F0000181E301F0000181E181F0000181E0C1F0000183C0C1F00001C3C061F80000C3C061F80000C3C030
  F80000C3C030F80000C3C018F80000C78018F80000E7800CFC000067800CFC00006780067C00006780067C00006F00037C00
  006F00037C00007F0001FE00003F0001FE00003F0000FE0007FE0000FFF007FE00007FF0> 44 31 -1 0 42] 77 @dc
[<3F07807FCFC0F8EF60F03F30F01F30F00F30F00F30780F007C0F803F0F800FE78001FF800007800007801E07801F07801F07
  801F1F801FFF0007FC00> 20 20 -3 0 23] 97 @dc
[<FFE0FFE01F001F000F000F000F000F000F800780078007800780078007C003C003C003C003C003C003E001E001E001E001E0
  01E001F000F001F00FF007F000F0> 12 32 -1 0 13] 108 @dc
[<1F007F8079C078C078607860786078607C003C003C003C003C003C003E001E001E001E00FFF8FFF83E000F00070007000300
  030003000180> 13 28 -4 0 18] 116 @dc
[<030007000E001C001C003800380038007000700070007000F000F000F000F000F000F000F000F000F0007000700078007800
  7800780038003C003C001C001E001E000E000F0007000780038001C001E000E000700038001C000E0007> 16 46 -4 12 18] 40 @dc
[<03800003800003C00003C00003E00007E00007B00007B000079800079C000F8C000F0E000F06000F03000F03001F01801E01
  801F03E0FFC7F8FFC7F8> 21 20 -4 0 24] 118 @dc
[<FFFEFFFE07C003C003C003C003C003C003E001E001E001E001E001E001F000F000F000F000F000F000F80078007800781E78
  1FF801FC003C000C> 15 29 -4 0 23] 49 @dc
[<70F8F87878> 5 5 -4 0 13] 46 @dc
[<0FC0001FF000387800701C00700E00F00F00E00700F00780F00780F00780F003C0F003C0F003C0F003C0F003C07801E07801
  E07801E07801E03801E03C01E03C01E01C01E01C01E00E01C00701C003838001FF00007E00> 19 29 -4 0 23] 48 @dc
[<E000700038001C001E000F000700038003C001C001E000E000F00070007800780038003C003C001C001C001E001E001E001E
  000E000E000E000E000E000E000E000E000E000E000E001E001C001C001C003800380070007000E000C0> 15 46 -1 12 18] 41 @dc
[<FFFFFFC07FFFFFC007E007E003E001E003E000E003E0007003E0003003E0003003F0001801F0001801F0181801F0180C01F0
  180001F0180001F8380000FFFC0000FFFC0000F83C0000F80C0000F80C0000FC0C18007C0618007C001C007C001C007C001C
  007C001C007E001C003E003C003E007C07FFFFFC07FFFFFC> 30 31 -1 0 31] 69 @dc
[<007FC18001FFF38007F07FC00FC01FC01F000FC03E0007C03E0007C07C0007C07C0007E0780007E0780003E0F800FFFCF800
  FFFCF8000000FC0000007C0000007C0000007C0000007C0000003E0000303E0000381F0000381F0000380F80003807C00078
  03E0007801F000FC00FC01FC007F07BC001FFF1C0003FC0C> 30 31 -5 0 36] 71 @dc
[<C0E070381C0C0C0E063E7E7E3E3C> 7 14 -3 9 13] 44 @dc
cmr10.329 @sf
[<07F01FC01FFC7FE03E1EFC707C07F038F807E018F80FE000F00FE000F01F7000F03E3800F07E1800787C1C0038F80C001CF8
  0E000FF0070007E0030003E0038003E0038003E007E003F01FFC07F81FFC079C0000078E0000078E00000787000007830000
  07830000078300000783000003C7000003C6000001FE0000007C0000> 30 32 -2 0 35] 38 @dc
cmsl10.329 @sf
[<FFC0FFFCFFC0FFFC1F000FC00E000F8006000F8003000F8003000F8001800F8001801F8000FFFF0000FFFF0000601F000060
  1F0000301F0000303F0000183E0000183E00000C3E00000C3E0000063E0000067E0000037C0000037C000001FC000001FC00
  0000FC000000F80000007800000078000000380000003800> 30 31 -2 0 34] 65 @dc
[<FFE7FF3FF8FFE7FF3FF81F00F807C00F00F807C00F007803C00F007803C00F007803C00F007803C00F807C03E007803C01E0
  07803C01E007803C01E007803C01E007C03E01E007C03E01E003E03F01E007F03D81E03FDC7CE3E01FCFFC7FE003C3F01F80> 37 20 -1 0 39] 109 @dc
[<FFE00000FFE000000F0000000F0000000F0000000F8000000F8000000780000007800000078FC00007BFF00007F0F80003E0
  7C0003C03E0003C01E0003C01F0003C00F0003E00F0001E00F8001E0078001E0078001E0078001E0078001F0078000F00F00
  00F80F000FFE1F0007FFFE0000F1F800> 25 29 1 9 25] 112 @dc
[<0FC7F83FF7F83C3FC03C0FC03C07C03C07C03C03C03C03C03E03E01E03E01E01E01E01E01E01E01E01E01F01F00F01F01F01
  F0FF0FF07F07F00F00F0> 21 20 -3 0 25] 117 @dc
[<FFFE03F87FFE0FFC07E01F8E03E01F0703E01F0303E01F0003E01F0003E01F0003F01F8001F00F8001F00F8001F00F8001F0
  1F8001F01F0001F83F0000FFFE0000FFFE0000F80F8000F803C000F801E000FC01F0007C00F0007C00F8007C00F8007C00F8
  007C00F8007E00F0003E01F0003E03E007FFFF8007FFFE00> 32 31 -1 0 33] 82 @dc
[<0300E0000700E0000780F0000780F00007C0F80007C1F80007E1FC000FE1EC000F31EE000F31C6000F19C7000F1BC3000F0F
  C3801F0FC1801E07C1C01E07C0C01E0380E01E07C0F0FF9FF3FCFF9FF3FC> 30 20 -4 0 33] 119 @dc
[<FFFE00FFFE0007E00003E00003E00003E00003E00003E00003F00001F00001F00001F00001F00001F00001F80000F80000F8
  0000F80000F80000F80000FC00007C00007C00007C00007C00007C00007E00003E00003E0007FFF007FFF0> 20 31 0 0 16] 73 @dc
[<FFFF00007FFF000007E0000003E0000003E0000003E0000003E0000003E0000003F0000001F0000001F0180001F0180001F0
  180001F0180001F8380000FFFC0000FFFC0000F83C0000F80C0000F80C0000FC0C18007C0618007C001C007C001C007C001C
  007C001C007E001C003E003C003E007C07FFFFFC07FFFFFC> 30 31 -1 0 30] 70 @dc
[<00C000C0000000E000E0000000E000E0000001F001F0000001F001F0000001F801F8000001F801F8000001FC01FC000001FC
  01FC000003F601F6000003E603E6000003E303E3000003E303E3000003E183E1800003E183E1800007E1C3E0C00007C0C7E0
  C00007C0E7C0600007C067C0600007C077C0300007C037C0300007C03FC018000FC01FC018000F801F801C000F800F800C00
  0F800F800E000F800F8006000F800F8007001F801F800F80FFF8FFF83FF0FFF8FFF83FF0> 44 31 -6 0 47] 87 @dc
[<0FE0003FF8003C7C00781E00700F00F00F00F00780F00780F00780F007C0F003C0F003C0F803C0F803C0FC07807E07807BFF
  0079FE007830003C00003C00001E00001E03E00F03E00783E003C3E001F0E0007FC0001F80> 19 29 -4 0 23] 54 @dc
[<0FE0003FF800787E00E01F00F00F80F80F80F807C0F807C07807E00007E00003E00003E00003C00C03C00E07C00F8F800FFF
  000CFC000C00000E000006000006000006000006000007FE0007FF8003FFE003FFF0038070> 20 29 -3 0 23] 53 @dc
[<FFFC7FFE007FFE7FFE0007E007E00003E007E00003E003E00003E003E00003E003E00003E003E00003F003F00001F003F000
  01F001F00001F001F00001F001F00001F001F00001F801F80000FFFFF80000FFFFF80000F800F80000F800F80000F800F800
  00FC00FC00007C00FC00007C007C00007C007C00007C007C00007C007C00007E007E00003E007E00003E003E0007FFE3FFF0
  07FFE3FFF0> 36 31 -1 0 34] 72 @dc
[<C3F800EFFE00FE0F007807807003C07003C06001E06001E06001E00001F00001F00001F00003F0003FE001FFE007FFE007FF
  C00FFF800FFC000FC0000F80000F00000F00180F00180F001807803807803C03C07C01E1FC00FFDC003F8C> 22 31 -3 0 25] 83 @dc
[<3C0000007F000000E3800000C1800000F8C00000786000000060000000300000003000000038000000380000003C0000003C
  0000003E0000007E0000007B0000007B0000007980000079C00000F8C00000F0E00000F0600000F0300000F0300001F01800
  01E0180001F03E000FFC7F800FFC7F80> 25 29 0 9 24] 121 @dc
[<FFFFF000007FFFFE000007E01F800003E007C00003E001E00003E000F00003E000780003E000780003F0003C0001F0003E00
  01F0001E0001F0001F0001F0001F0001F0001F0001F8000F0000F8000F8000F8000F8000F8000F8000F8000F8000F8000F80
  00FC000F80007C000F80007C000F00007C000F00007C000F00007C001E00007E003E00003E007C00003E00F80007FFFFF000
  07FFFF8000> 33 31 -1 0 35] 68 @dc
[<C1F800E7FE00FE1F007C0F807807C07803C07801E07801E07C01E03C01F03C00F03C00F03C00F03C00F03E00F01E00E01F01
  E01FC3E01FFFC01E3F001F00000F00000F00000F00000F00000F00000F80000780000F80007F80003F8000078000> 20 32 -4 0 25] 98 @dc
cmr10.329 @sf
[<000600060000000E00070000000F000F0000000F000F0000001F000F8000001F801F8000001F801F8000003F801FC000003E
  C03EC000003EC03EC000003EC03EC000007C607C6000007C607C6000007C607C600000F830F8300000F830F8300000F830F8
  300001F019F0180001F019F0180001F019F0180003E00FF00C0003E00FE00C0003E00FE00C0007C00FE0060007C007C00600
  07C007C006000F800FC003000F800F8007001F800F800F80FFF8FFF83FF0FFF8FFF83FF0> 44 31 -1 0 47] 87 @dc
[<FFFFF8FFFFF8FC01F87E00787E00383F00181F00181F801C0FC01C0FC00C07E00C07E00C03F00C03F00001F80000F80000FC
  00007E00007E00003F00C03F00C01F80C01F80E00FC0E007C06007E07003F07803F07E01F87FFFF87FFFF8> 22 31 -3 0 28] 90 @dc
cmsl10.329 @sf
[<FFFFFE007FFFFE0007E03F0003E00F0003E0070003E0030003E0038003E0018003F0018001F0018001F000C001F000C001F0
  000001F0000001F8000000F8000000F8000000F8000000F8000000F8000000FC0000007C0000007C0000007C0000007C0000
  007C0000007E0000003E0000003E000007FFF80007FFF800> 26 31 -1 0 28] 76 @dc
[<FFCFF8FFCFF81F07E00F07C00F0FC00F0F800F0F800F1F000F9F0007FE0007FE0007FC0007BC00079E0007C70003C38003C1
  C003C1F003C3FE03C3FE03E00001E00001E00001E00001E00001E00001F00000F00001F0000FF00007F00000F000> 23 32 -1 0 24] 107 @dc
cmr10.329 @sf
[<00000F8000001FC000001FE000001FE000003FF000003C70000038300000301000003010003FE01000FFF00003F87C0007F0
  7E000F38CF001F3F8F801E0F07803C0003C07C0003E07C0003E0780001E0F80001F0F80001F0F80001F0F80001F0F80001F0
  F80001F0F80001F0F80001F0F80001F0780001E07C0003E03C0003C03C0003C01E0007801F000F800F000F0007C03E0003E0
  7C0000FFF000003FC000> 28 40 -3 9 35] 81 @dc
[<FFC3FF00FFC3FF000F00F0000F00F0000F00F0000F00F0000F00F0000F00F0000F00F0000F00F0000F00F0000F00F0000F00
  F0000F00F0000F00F0000F00F0000F00F0000F00F000FFFFFF80FFFFFF800F00F0000F00F0000F00F0000F00F0000F00F000
  0F00F0000F01F07C0781F87C07C1FC7C03F1FE7C00FFFFF8001FC3F0> 30 32 0 0 27] 11 @dc
cmsl10.329 @sf
[<3F8000FFE000E1F000F07800F83C00F81E00F80F00000F000007800007800107C00FF3C01FFBC03C1FC03807C07807E07803
  E07803E07801E07801E07C01E03C01E03C01E03C01E01E01C00F03C007C78003FF0000FE00> 19 29 -4 0 23] 57 @dc
[<0FE0003FFC00787E00F81F00FC0F80FC0FC0FC07C0FC07C07807C00007C00003C00003C00007C0000780000F0000FC0000FE
  00001F800007C00003E00003E00F81F00F81F00F81F00F81F00F81F007C3E003FFC0007F00> 20 29 -3 0 23] 51 @dc
[<03FFE003FFE0003E00003E00001E00001E00001E00001E00FFFFF07FFFF8600F00300F00180F000C0F000E0F000707800307
  8001878000C780006780007780003BC0001BC0000FC00007C00003C00003C00001E00000E0> 21 29 -2 0 23] 52 @dc
cmr10.329 @sf
[<FFC3FFFFC3FF0F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00
  F00F00F0FFFFF0FFFFF00F00000F00000F00000F00000F00000F01F00F01F00781F007C1F003F0F000FFE0001FC0> 24 32 0 0 25] 12 @dc
13 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
216 195 p ([2]) s
21 r (CCITT) s
20 r (SG) s
19 r (5/VI) s
1 r (I,) s
20 r (\\Recommendati) s
-1 r (ons) s
18 r (X.400,") s
17 r (Message) s
19 r (Handling) s
19 r (Sys-) s
286 252 p (tems:) s
14 r (System) s
14 r (Mo) s
1 r (del) s
15 r 45 c
15 r (Service) s
16 r (Elemen) s
-1 r (ts,) s
13 r (Octob) s
1 r (er) s
15 r (1984.) s
216 320 p ([3]) s
21 r (CCITT) s
16 r (SG) s
15 r (5/VI) s
1 r (I,) s
16 r (\\CCITT) s
15 r (Recommendations) s
14 r (X.400/) s
14 r (ISO) s
17 r (DIS) s
16 r (10021,") s
286 377 p (Message) s
15 r (Handling:) s
14 r (System) s
14 r (and) s
16 r (Service) s
15 r (Ov) s
(erview) s
14 r 44 c
15 r (April) s
14 r (1988.) s
216 446 p ([4]) s
21 r (D.H.) s
21 r (Cro) s
1 r 99 c
107 c
-1 r (er,) s
19 r (\\Standard) s
21 r (of) s
20 r (the) s
22 r 70 c
-3 r (orm) s
-1 r (at) s
18 r (of) s
21 r (ARP) s
-3 r 65 c
21 r (In) s
(ternet) s
21 r 84 c
-3 r (ext) s
20 r (Mes-) s
286 502 p (sages,") s
14 r (RF) s
67 c
14 r (822,) s
14 r (August) s
15 r (1982.) s
216 571 p ([5]) s
21 r (D.P) s
-3 r 46 c
10 r (Kingston,) s
10 r (\\MMDFI) s
1 r (I:) s
11 r 65 c
11 r 84 c
-3 r (ec) s
(hnical) s
9 r (Review,") s
cmsl10.329 @sf
11 r (Usenix) s
11 r (Conference) s
cmr10.329 @sf
3 r 44 c
11 r (Salt) s
286 628 p (Lak) s
101 c
15 r (Cit) s
121 c
5 r (\(August) s
15 r (1984\).) s
216 697 p ([6]) s
21 r (E.) s
15 r (Allman,) s
13 r (\\SENDMAIL) s
16 r 45 c
15 r (An) s
15 r (In) s
(ternet) s
119 c
-1 r (or) s
-1 r 107 c
14 r (Mail) s
13 r (Router,") s
15 r 80 c
(ap) s
1 r (er,) s
14 r (1983.) s
216 765 p ([7]) s
21 r (D.L.) s
22 r (Presotto,) s
21 r (\\Upas) s
22 r 45 c
23 r 97 c
22 r (simpler) s
21 r (approac) s
104 c
21 r (to) s
21 r (net) s
119 c
(or) s
-1 r 107 c
21 r (mail,) s
-1 r 34 c
cmsl10.329 @sf
20 r (Usenix) s
286 822 p (Pro) s
1 r (ceedings) s
cmr10.329 @sf
10 r (\(1985\).) s
216 891 p ([8]) s
21 r (CCITT/ISO,) s
18 r (\\The) s
18 r (Directory) s
16 r 45 c
18 r (Ov) s
(erview) s
16 r (of) s
17 r (Concepts,) s
17 r (Mo) s
1 r (dels) s
17 r (and) s
18 r (Ser-) s
286 947 p (vices,") s
15 r (CCITT) s
15 r (Recommendation) s
13 r (X.500) s
14 r 47 c
15 r (ISO) s
16 r (DIS) s
16 r (9594-1) s
14 r 44 c
14 r (Ma) s
121 c
14 r (1988.) s
216 1016 p ([9]) s
21 r (S.E.) s
12 r (Kille,) s
11 r (\\Mapping) s
12 r (Bet) s
119 c
-1 r (een) s
11 r (X.400) s
11 r (and) s
13 r (RF) s
67 c
11 r (822,") s
11 r (UK) s
12 r (Academic) s
12 r (Com-) s
286 1073 p 109 c
(unit) s
-1 r 121 c
13 r (Rep) s
1 r (ort) s
15 r (\(MG.19\)) s
13 r 47 c
15 r (RF) s
67 c
14 r (987,) s
14 r (June) s
16 r (1986.) s
193 1142 p ([10]) s
21 r (J.B.) s
13 r 80 c
(ostel) s
-1 r 44 c
10 r (\\SIMPLE) s
13 r (MAIL) s
13 r (TRANSFER) s
13 r (PR) s
(OTOCOL,") s
12 r (RF) s
67 c
11 r (821,) s
11 r (Au-) s
286 1198 p (gust) s
15 r (1982.) s
193 1267 p ([11]) s
21 r (S.E.) s
17 r (Kille,) s
17 r (ed.,) s
cmsl10.329 @sf
17 r (JNT) s
18 r (Mail) s
16 r (Proto) s
1 r (col) s
17 r (\(revision) s
16 r (1.0\)) s
cmr10.329 @sf
1 r 44 c
16 r (Join) s
116 c
16 r (Net) s
119 c
-1 r (or) s
-1 r 107 c
16 r 84 c
-3 r (eam,) s
286 1323 p (Rutherford) s
16 r (Appleton) s
15 r (Lab) s
1 r (oratory,) s
14 r (Marc) s
104 c
14 r (1984.) s
193 1392 p ([12]) s
21 r (S.E.) s
19 r (Kille,) s
19 r (\\Addressing) s
19 r (in) s
20 r (MMDF) s
18 r 73 c
1 r (I,") s
cmsl10.329 @sf
20 r (Pro) s
1 r (c.) s
19 r (EUUG) s
19 r (Conference,) s
20 r 80 c
(aris) s
cmr10.329 @sf
286 1449 p (\(April) s
15 r (1985\).) s
193 1518 p ([13]) s
21 r (S.E.) s
14 r (Kille,) s
12 r (\\MHS) s
14 r (Use) s
14 r (of) s
13 r (Directory) s
12 r (Service) s
14 r 70 c
-3 r (or) s
13 r (Routing,") s
12 r (North) s
14 r (Holland,) s
286 1574 p (IFIP) s
16 r (6.5) s
14 r (Conference) s
16 r (on) s
15 r (Message) s
15 r (Handling,) s
14 r (Munic) s
(h,) s
14 r (April) s
14 r (1987.) s
193 1643 p ([14]) s
21 r (S.E.) s
21 r (Kille) s
21 r 38 c
21 r (D.H.) s
20 r (Brink,) s
21 r (\\A) s
21 r (Mo) s
1 r (del) s
21 r (of) s
21 r (Message) s
20 r (Flo) s
119 c
19 r (Con) s
(trol,) s
-1 r 34 c
cmsl10.329 @sf
19 r 65 c
(CM) s
286 1699 p (Computer) s
14 r (Comm) s
-2 r (unicatio) s
-1 r 110 c
14 r (Review) s
cmr10.329 @sf
11 r (\(Octob) s
1 r (er) s
16 r (1985\).) s
193 1768 p ([15]) s
21 r (S.E.) s
20 r (Kille,) s
19 r (\\Gatew) s
-1 r 97 c
-1 r (ying) s
18 r 98 c
1 r (et) s
119 c
-1 r (een) s
19 r (X.400) s
19 r (and) s
20 r (other) s
20 r (message) s
18 r (proto) s
1 r (cols,") s
286 1825 p (ONLINE,) s
19 r (Online) s
17 r (Conference) s
18 r (on) s
18 r (Electronic) s
16 r (Messaging) s
17 r (Systems,) s
16 r (August) s
286 1881 p (1986.) s
193 1950 p ([16]) s
21 r (J.P) s
-3 r 46 c
15 r (Onions) s
16 r 38 c
16 r (M.T.) s
14 r (Rose,) s
16 r (\\The) s
15 r (Applications) s
15 r (Co) s
1 r (okb) s
1 r 111 c
1 r (ok,") s
cmsl10.329 @sf
15 r (IFIP) s
16 r 87 c
71 c
13 r (6.5) s
286 2007 p (Conference) s
17 r (on) s
16 r (Message) s
15 r (Handling) s
16 r (Systems) s
14 r (and) s
17 r (Distributed) s
15 r (Applications) s
cmr10.329 @sf
286 2063 p (\(Septem) s
98 c
1 r (er) s
14 r (1988\).) s
193 2132 p ([17]) s
21 r (D.B.) s
14 r 84 c
-3 r (erry,) s
13 r (M.) s
14 r 80 c
(ain) s
-1 r (t) s
-1 r (er,) s
12 r (D.W.) s
14 r (Riggle) s
13 r 38 c
15 r (S.) s
14 r (Zhou,) s
14 r (\\The) s
15 r (Berk) s
(eley) s
13 r (In) s
(ternet) s
286 2188 p (Name) s
14 r (Domain) s
14 r (Serv) s
(er,") s
cmsl10.329 @sf
13 r (Usenix,) s
15 r (Salt) s
14 r (Lak) s
101 c
15 r (Cit) s
-1 r 121 c
cmr10.329 @sf
10 r (\(August) s
15 r (1984\).) s
193 2257 p ([18]) s
21 r (S.E.) s
13 r (Kille,) s
12 r (\\The) s
13 r (QUIPU) s
14 r (Directory) s
12 r (Service,") s
cmsl10.329 @sf
12 r (IFIP) s
14 r 87 c
71 c
11 r (6.5) s
12 r (Conference) s
14 r (on) s
286 2314 p (Message) s
11 r (Handling) s
11 r (Systems) s
11 r (and) s
12 r (Distributed) s
10 r (Applications) s
cmr10.329 @sf
9 r (\(Octob) s
1 r (er) s
11 r (1988\).) s
193 2383 p ([19]) s
21 r (M.T.) s
14 r (Rose) s
15 r 38 c
15 r (E.A.) s
14 r (Ste\013erud,) s
14 r (\\Prop) s
1 r (osed) s
15 r (Standard) s
15 r (for) s
14 r (Message) s
14 r (Encapsu-) s
286 2439 p (lation,") s
cmsl10.329 @sf
14 r (RF) s
67 c
13 r (934) s
cmr10.329 @sf
11 r (\(Jan) s
(uary) s
13 r (1985\).) s
193 2508 p ([20]) s
21 r (ISO/CCITT,) s
16 r (\\Remote) s
14 r (Op) s
1 r (erations:) s
15 r (Mo) s
1 r (del,) s
14 r (Notation) s
14 r (and) s
16 r (Service) s
15 r (De\014ni-) s
286 2564 p (tion,") s
12 r (CCITT) s
12 r (Recommendation) s
11 r (X.219) s
11 r 47 c
12 r (ISO) s
14 r (IS) s
13 r (9072-1) s
12 r 44 c
12 r (No) s
118 c
-1 r (em) s
-2 r 98 c
1 r (er) s
11 r (1987.) s
193 2633 p ([21]) s
21 r (CCITT) s
10 r (SG) s
10 r (5/VI) s
1 r (I,) s
11 r (\\CCITT) s
9 r (Recommendations) s
8 r (X.413/) s
9 r (ISO) s
11 r (nnnn-1,") s
11 r (Mes-) s
286 2690 p (sage) s
11 r (Handling) s
10 r (Systems:) s
9 r (Message) s
10 r (Store:) s
9 r (Abstract) s
10 r (Service) s
11 r (De\014nition) s
10 r (\(Final) s
286 2746 p (Draft) s
14 r 45 c
16 r (Gloucester\),) s
13 r (No) s
118 c
(em) s
-2 r 98 c
1 r (er) s
13 r (1987.) s
949 2916 p (13) s
@eop
12 @bop0
/cmbx10.432 @newfont
cmbx10.432 @sf
[<01FE000007FFC0000FFFE0001F07F8001F01FC003F80FE003FC0FF003FC07F003FC07F803FC07F801F803FC00F003FC00000
  3FC000003FC000083FE001FF3FE007FFBFE00FFFFFE01FC0FFE03F80FFE07F807FE07F807FE0FF807FE0FF803FE0FF803FE0
  FF803FE0FF803FE0FF803FC0FF803FC0FF803FC0FF803FC07F803F807F803F803F807F001FC07E000FE1FE0007FFFC0003FF
  F000007FC000> 27 39 -3 0 34] 57 @dc
[<FFFFF800FFF0FFFFF807FFFCFFFFF80FFFFE03FE001FFE1F03FE003FFC0F03FE003FF80703FE003FF00703FE003FF00003FE
  003FF00003FE003FF00003FE003FF00003FE003FE00003FE003FE00003FE003FE00003FE003FE00003FE007FC00003FE007F
  C00003FE00FF800003FE01FF000003FFFFFE000003FFFFFE000003FFFFFF800003FE007FE00003FE001FF00003FE000FF800
  03FE0007FC0003FE0007FC0003FE0007FE0003FE0007FE0003FE0007FE0003FE0007FE0003FE0007FE0003FE0007FE0003FE
  0007FC0003FE0007FC0003FE000FF80003FE001FF00003FE007FE000FFFFFFFF8000FFFFFFFE0000FFFFFFE00000> 48 41 -3 0 52] 82 @dc
[<003FF00001FFFE0003FFFF800FFC0FC01FF003C03FC001E03F8000E07F8000007F800000FF000000FF000000FF000000FF00
  0000FFFFFFE0FFFFFFE0FFFFFFE0FF0007E0FF0007E07F0007E07F800FC03F800FC03FC01FC01FC03F800FF07F0007FFFE00
  01FFFC00003FE000> 27 27 -2 0 32] 101 @dc
[<E1FF00FFFFC0FFFFF0FF03F8FC00F8F8007CF0007CE0007CE000FC0001FC0007FC01FFFC0FFFF81FFFF03FFFE07FFFC0FFFF
  00FFE000FE0000FC0070F80070F800707C00F07E03F03FFFF01FFFF003FE70> 22 27 -2 0 27] 115 @dc
[<007FC3FF8001FFF3FF8007FFFBFF8007F03FF8000FE00FF8000FE007F8000FE007F8000FE003F8000FE003F8000FE003F800
  0FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F800
  0FE003F8000FE003F8000FE003F8000FE003F800FFE03FF800FFE03FF800FFE03FF800> 33 27 -3 0 38] 117 @dc
[<FFFEFFFEFFFE0FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE0
  0FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE0FFE0FFE0FFE0> 15 42 -3 0 20] 108 @dc
[<003F8000FFE001FFF003F8F007F87807F03807F03807F03807F03807F03807F03807F00007F00007F00007F00007F00007F0
  0007F00007F00007F00007F00007F00007F00007F000FFFFF0FFFFF0FFFFF01FF00007F00003F00001F00001F00000F00000
  F000007000007000007000007000> 21 38 -1 0 27] 116 @dc
[<03FC07FC0FFF0FFC3FFFDFFC7FC3FFC07F00FF80FF007F80FE003F80FE003F80FE003F80FF003F807F003F803F803F801FE0
  3F800FFE3F8003FFFF80003FFF8000003F8000003F801F803F803FC03F803FC03F803FC07F003FC0FF003FC1FE001FFFFC00
  0FFFF00003FFC000> 30 27 -2 0 33] 97 @dc
[<FFFE3FFF80FFFE3FFF80FFFE3FFF800FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F800
  0FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FF003F8000FF003F800
  0FF803F8000FF803F8000FDE07F8000FCF87F000FFC7FFE000FFC3FFC000FFC0FF0000> 33 27 -3 0 38] 110 @dc
[<007FC3FF8001FFF3FF8007FFFBFF800FF07FF8001FC01FF8003FC00FF8003F8007F8007F8003F8007F0003F800FF0003F800
  FF0003F800FF0003F800FF0003F800FF0003F800FF0003F800FF0003F800FF0003F800FF0003F8007F0003F8007F8003F800
  3F8003F8003FC007F8001FE00FF8000FF83FF80007FFFFF80001FFFBF800003FC3F800000003F800000003F800000003F800
  000003F800000003F800000003F800000003F800000003F800000003F800000003F800000003F800000003F80000003FF800
  00003FF80000003FF800> 33 42 -2 0 38] 100 @dc
[<FFFF003FFFFEFFFF003FFFFEFFFF003FFFFE03E00001FF8001E00001FF0001F00003FF0000F00003FE0000F00003FE0000F8
  0007FE0000780007FC00007C000FFC00003FFFFFF800003FFFFFF800003FFFFFF800001E001FF000001F003FF000000F003F
  E000000F003FE000000F807FE0000007807FC0000007C0FFC0000003C0FF80000003C0FF80000003E1FF80000001E1FF0000
  0001F3FF00000000F3FE00000000F3FE00000000FFFE000000007FFC000000007FFC000000003FF8000000003FF800000000
  3FF8000000001FF0000000001FF0000000000FE0000000000FE0000000000FE00000000007C00000000007C00000> 47 41 -2 0 52] 65 @dc
[<0001C000000003E000000003E000000007F000000007F00000000FF80000000FF80000000FF80000001FDC0000001FDC0000
  003FDE0000003F8E0000007F8F0000007F070000007F07000000FE03800000FE03800001FC01C00001FC01C00003FC01E000
  03F800E00007F800F00007F000700007F0007000FFFE03FF80FFFE03FF80FFFE03FF80> 33 27 -1 0 36] 118 @dc
[<FFFEFFFEFFFE0FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE00FE0FFE0
  FFE0FFE00000000000000000000000000F801FC03FE03FF03FF03FF03FF03FE01FC00F80> 15 43 -3 0 20] 105 @dc
[<0E01FE00000F07FFC0000F9FFFF0000FFE0FF8000FF803FC000FF001FE000FE000FE000FE000FF000FE0007F000FE0007F80
  0FE0007F800FE0007F800FE0007F800FE0007F800FE0007F800FE0007F800FE0007F800FE0007F800FE0007F000FE000FF00
  0FE000FE000FF001FE000FF801FC000FFF07F8000FFFFFF0000FE7FFC0000FE1FF00000FE00000000FE00000000FE0000000
  0FE00000000FE00000000FE00000000FE00000000FE00000000FE00000000FE00000000FE00000000FE0000000FFE0000000
  FFE0000000FFE0000000> 33 42 -2 0 38] 98 @dc
[<0F800000003FE00000007FF8000000F67C000000FE1C000000FE0E000000FE0F0000007C0700000038078000000003800000
  00038000000001C000000001C000000003E000000003E000000007F000000007F00000000FF80000000FF80000000FF80000
  001FDC0000001FDC0000003FDE0000003F8E0000007F8F0000007F070000007F07000000FE03800000FE03800001FC01C000
  01FC01C00003FC01E00003F800E00007F800F00007F000700007F0007000FFFE03FF80FFFE03FF80FFFE03FF80> 33 39 -1 12 36] 121 @dc
cmr10.329 @sf
[<3F807FE0F9F0F8F0F87870780078007800780078007800780078007800780078007800780078007800780078007800780078
  00F807F807F80078000000000000000000000000007000F800F800F80070> 13 40 3 9 14] 106 @dc
[<70F8F8F870000000000070707070707070707070707070F8F8F8F8F8F8F8F870> 5 32 -4 0 13] 33 @dc
cmbx10.432 @sf
[<7FFFFE7FFFFE7FFFFE00FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF
  0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF00F8FF00FF
  FF00FFFF0007FF00007F00000F00000700> 23 39 -5 0 34] 49 @dc
[<003F800001FFF00007FFFC000FE0FE001FC07F001F803F003F803F803F001F807F001FC07F001FC07F001FC07F001FC0FF00
  1FE0FF001FE0FF001FE0FF001FE0FF001FE0FF001FE0FF001FE0FF001FE0FF001FE0FF001FE0FF001FE0FF001FE0FF001FE0
  FF001FE0FF001FE07F001FC07F001FC07F001FC07F001FC03F001F803F803F801F803F001FC07F000FE0FE0007FFFC0001FF
  F000003F8000> 27 39 -3 0 34] 48 @dc
[<00007FF800000007FFFF0000001FFFFFC000007FF807F00000FFC000F80003FF00007C0007FE00001E000FFC00000F000FF8
  000007001FF0000007803FF0000003803FE0000003807FE0000003807FE0000003807FC000000000FFC000000000FFC00000
  0000FFC000000000FFC000000000FFC000000000FFC000000000FFC000000000FFC000000000FFC000000000FFC000000000
  FFC0000000007FC0000007807FE0000007807FE0000007803FE000000F803FF000000F801FF000001F800FF800001F800FFC
  00003F8007FE00007F8003FF0000FF8000FFC003FF80007FF80FFF80001FFFFF8F800007FFFE078000007FF00380> 41 41 -4 0 50] 67 @dc
[<003FE00001FFFC0007FFFF000FF07F801FC01FC03F800FE03F800FE07F0007F07F0007F0FF0007F8FF0007F8FF0007F8FF00
  07F8FF0007F8FF0007F8FF0007F8FF0007F87F0007F07F0007F07F0007F03F800FE03F800FE01FC01FC00FF07F8003FFFE00
  01FFFC00003FE000> 29 27 -2 0 34] 111 @dc
[<003FF00001FFFC0007FFFF000FFC0F801FF003803FC003C03FC001C07F8000007F800000FF000000FF000000FF000000FF00
  0000FF000000FF000000FF000000FF000000FF0000007F003F007F807F803F807F803FC07F801FE07F800FF87F8007FFFF00
  01FFFE00003FF800> 26 27 -2 0 31] 99 @dc
/cmsy10.329 @newfont
cmsy10.329 @sf
[<07E01FF83FFC7FFE7FFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7FFE7FFE3FFC1FF807E0> 16 18 -3 -2 23] 15 @dc
cmr10.329 @sf
[<003FFC003FFC0003C00003C00003C00003C00003C00003C00003C007E3C01FFBC03E1FC03C0FC07C07C07803C0F803C0F003
  C0F003C0F003C0F003C0F003C0F003C0F803C07803C07C07C03C0FC01F1DC00FF9C007F0C0> 22 29 -2 9 24] 113 @dc
cmbx10.432 @sf
[<FFFC3FFEFFFC3FFEFFFC3FFE0FC00FF00FC01FE00FC03FC00FC03F800FC07F800FC0FF000FC0FE000FE1FE000FF3FC000FFF
  F8000FFFF0000FFFF0000FEFE0000FE7C0000FE3E0000FE1F0000FE0F8000FE07C000FE01E000FE00F800FE007C00FE01FFC
  0FE01FFC0FE01FFC0FE000000FE000000FE000000FE000000FE000000FE000000FE000000FE000000FE000000FE000000FE0
  00000FE00000FFE00000FFE00000FFE00000> 31 42 -2 0 36] 107 @dc
[<00078003C00000078003C000000FC007E000000FC007E000000FC007E000001FE00FF000001FE00FF000003FF01FF800003F
  F01FB800003FF01FB800007F783F3C00007F383F1C0000FF383F1E0000FE1C7E0E0000FE1C7E0E0001FE1EFC0F0001FC0EFC
  070001FC0EFC070003F807F8038003F807F8038007F807F803C007F003F001C007F003F001C00FE007E000E0FFFE7FFC0FFE
  FFFE7FFC0FFEFFFE7FFC0FFE> 47 27 -1 0 50] 119 @dc
[<00FFF80007FFFF001FFFFFC03FE03FE07F0007F07E0003F0FE0003F8FC0001F8FC0001F8FC0001F8FE0003F87E000FF83FFF
  FFF01FFFFFF00FFFFFE01FFFFFC03FFFFF003FFFFC003E0000003C0000003C0000003800000038FF80001FFFF0000FFFF800
  1FC1FC003F80FE003F007E007F007F007F007F007F007F007F007F007F007F007F007F003F007E783F80FE7C1FC1FEFC0FFF
  FFFC07FFF7FC00FF81F0> 30 40 -2 13 34] 103 @dc
[<FFFE1FFFC3FFF8FFFE1FFFC3FFF8FFFE1FFFC3FFF80FE001FC003F800FE001FC003F800FE001FC003F800FE001FC003F800F
  E001FC003F800FE001FC003F800FE001FC003F800FE001FC003F800FE001FC003F800FE001FC003F800FE001FC003F800FE0
  01FC003F800FE001FC003F800FE001FC003F800FE001FC003F800FF001FE003F800FF001FE003F800FF801FF003F800FFC01
  FF803F800FDE03FBC07F000FCF87F9F0FF00FFC7FFF0FFFE00FFC1FFE03FFC00FFC07F800FF000> 53 27 -3 0 60] 109 @dc
cmr10.329 @sf
[<FFFEFFFEF81E7C063E063F071F030F830FC307C003E003F061F060F860FC707C303E381F3FFF3FFF> 16 20 -1 0 20] 122 @dc
cmbx10.432 @sf
[<FFFFFF80FFFFFF80FFFFFF807FFFFF803FFFFFC01FFFFFC00FFFFFC007FFFFC003C001C003E001C001F001C000F800E0007E
  00E0003F00E0001F8000000FC0000007F0000003F8000003FC000001FE000000FF000000FF8000007FC000007FC000003FE0
  00003FE03E003FE07F003FE0FF803FE0FF803FE0FF803FE0FF807FC0FF807FC07F00FF807E01FF803F83FF001FFFFC0007FF
  F80000FFC000> 27 39 -3 0 34] 50 @dc
[<7FFF80007FFF80007FFF800007F0000007F0000007F0000007F0000007F0000007F0000007F0000007F0000007F0000007F0
  000007F0000007F0000007F0000007F0000007F0000007F0000007F0000007F0000007F0000007F0000007F00000FFFFC000
  FFFFC000FFFFC00007F0000007F0000007F0000007F0000007F0000007F0000007F03F0007F07F8007F87F8003F87F8003FC
  7F8001FF7F80007FFF00003FFE000007FC00> 25 42 -2 0 21] 102 @dc
[<FFFF00FFFF00FFFF000FE0000FE0000FE0000FE0000FE0000FE0000FE0000FE0000FE0000FE0000FE0000FE0000FE0000FE0
  000FE0000FF07E0FF0FF0FF0FF0FF8FF0FF8FF0FDCFFFFDFFEFFCFFCFFC7F0> 24 27 -2 0 28] 114 @dc
cmr10.329 @sf
[<60F07038181C0C0C0C7CFCFCF870> 6 14 -4 -18 13] 39 @dc
12 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
cmbx10.432 @sf
224 195 p 57 c
69 r (Results) s
21 r (and) s
24 r (Av) s
-3 r (ailabil) s
-1 r (i) s
-1 r 116 c
-2 r 121 c
cmr10.329 @sf
224 297 p (The) s
20 r (PP) s
18 r (pro) s
3 r (ject) s
19 r (is) s
18 r (no) s
119 c
18 r 111 c
118 c
-1 r (er) s
17 r (three) s
20 r 121 c
(ears) s
17 r (old.) s
31 r (Dev) s
(elopmen) s
-1 r 116 c
17 r (of) s
18 r (this) s
19 r (system) s
224 353 p (has) s
21 r (in) s
118 c
-1 r (olv) s
-2 r (ed) s
20 r 109 c
(uc) s
-1 r 104 c
19 r (more) s
20 r 119 c
(ork) s
19 r (than) s
21 r (originall) s
-1 r 121 c
19 r (an) s
(ticipated!) s
36 r (Although) s
20 r (the) s
224 409 p (system) s
13 r (is) s
14 r (not) s
14 r (fully) s
14 r (op) s
1 r (erational,) s
13 r (the) s
15 r (ma) s
3 r (jor) s
12 r (comp) s
1 r (onen) s
(ts) s
12 r (are) s
15 r (all) s
13 r 119 c
(orking.) s
17 r (It) s
15 r (is) s
224 466 p (exp) s
1 r (ected) s
19 r (that) s
18 r (PP) s
18 r (will) s
16 r 98 c
1 r 101 c
19 r (brough) s
116 c
17 r (in) s
(to) s
16 r (service) s
18 r (shortly) s
17 r 98 c
1 r (efore) s
18 r (this) s
17 r (pap) s
1 r (er) s
19 r (is) s
224 522 p (presen) s
(ted.) s
295 579 p (PP) s
21 r (has) s
21 r 98 c
1 r (een) s
22 r (released) s
22 r (in) s
(to) s
19 r (the) s
21 r (Public) s
21 r (Domain,) s
21 r (and) s
21 r (will) s
20 r 98 c
1 r 101 c
22 r 97 c
21 r (part) s
21 r (of) s
224 635 p (the) s
17 r (ISODE) s
18 r ([1].) s
24 r (This) s
17 r (will) s
15 r 98 c
1 r 101 c
18 r (for) s
16 r (release) s
17 r (5.0,) s
16 r (whic) s
104 c
16 r (is) s
16 r (curren) s
(tly) s
16 r (planned) s
17 r (for) s
224 692 p (Jan) s
(uary) s
14 r (1989.) s
cmbx10.432 @sf
224 835 p (10) s
70 r (Conclusions) s
cmr10.329 @sf
224 936 p (This) s
14 r (pap) s
1 r (er) s
16 r (has) s
14 r (giv) s
(en) s
13 r (an) s
15 r 111 c
118 c
-1 r (erview) s
12 r (of) s
15 r (the) s
15 r (PP) s
14 r (system.) s
18 r (It) s
15 r (is) s
14 r (hop) s
1 r (ed) s
16 r (that) s
14 r (it) s
14 r (will) s
224 993 p (pro) s
118 c
-1 r 101 c
14 r (to) s
14 r 98 c
1 r 101 c
16 r 97 c
15 r (useful) s
15 r (basis) s
15 r (for) s
15 r 119 c
(ork) s
13 r (in) s
15 r 97 c
15 r 110 c
(um) s
-1 r 98 c
1 r (er) s
13 r (of) s
15 r (directions:) s
cmsy10.329 @sf
292 1099 p 15 c
cmr10.329 @sf
23 r (Pro) s
(visi) s
-1 r (on) s
17 r (of) s
18 r (service,) s
19 r (particularly) s
17 r (in) s
19 r (the) s
19 r (academic) s
17 r (and) s
19 r (researc) s
104 c
18 r (com-) s
338 1156 p 109 c
-1 r (unit) s
-1 r 121 c
-4 r 46 c
cmsy10.329 @sf
292 1249 p 15 c
cmr10.329 @sf
23 r (Dev) s
(elopm) s
-1 r (en) s
-1 r 116 c
16 r (of) s
17 r 97 c
18 r 110 c
(um) s
-2 r 98 c
1 r (er) s
17 r (of) s
17 r (adv) s
-2 r (anced) s
18 r (User) s
18 r (Agen) s
(ts,) s
16 r (esp) s
1 r (ecially) s
18 r (those) s
338 1306 p (with) s
14 r (supp) s
1 r (ort) s
16 r (for) s
14 r 109 c
(ult) s
-1 r (im) s
-1 r (edia) s
13 r (capabilit) s
-1 r 121 c
-4 r 46 c
cmsy10.329 @sf
292 1400 p 15 c
cmr10.329 @sf
23 r (Supp) s
1 r (ort) s
12 r (for) s
11 r (Message) s
11 r (Store) s
11 r (proto) s
1 r (cols,) s
11 r (and) s
12 r (in) s
11 r (particular) s
11 r (P7[21].) s
17 r (It) s
11 r (is) s
12 r 98 c
1 r (e-) s
338 1456 p (liev) s
(ed) s
12 r (that) s
13 r (the) s
14 r (QMGR) s
13 r (system) s
12 r (can) s
14 r 98 c
1 r 101 c
14 r (quite) s
14 r (easily) s
12 r (extended) s
15 r (to) s
13 r (pro) s
(vide) s
338 1513 p (these) s
15 r (services) s
15 r (in) s
15 r (an) s
15 r (elegan) s
116 c
14 r (manner.) s
cmsy10.329 @sf
292 1606 p 15 c
cmr10.329 @sf
23 r 70 c
-3 r (uller) s
9 r (use) s
11 r (of) s
10 r (X.500) s
10 r (directory) s
9 r (services.) s
19 r (This) s
10 r (migh) s
-1 r 116 c
8 r 98 c
1 r 101 c
12 r (done) s
11 r 98 c
121 c
9 r (extending) s
338 1663 p (Submit) s
15 r (to) s
15 r (recognise) s
15 r (Directory) s
15 r (Names) s
14 r (as) s
16 r 97 c
15 r (third) s
16 r (name) s
15 r (form,) s
13 r (and) s
17 r (also) s
338 1719 p (to) s
14 r (utilise) s
14 r (Directory) s
14 r (Services) s
16 r (for) s
14 r (routing.) s
cmbx10.432 @sf
224 1863 p (11) s
70 r (Ac) s
-1 r (kno) s
-1 r (wl) s
-1 r (e) s
-1 r (dgem) s
-2 r (e) s
-1 r 110 c
-1 r (t) s
-1 r 115 c
cmr10.329 @sf
224 1964 p (There) s
15 r (are) s
14 r (man) s
121 c
12 r 112 c
1 r (eople) s
15 r (who) s
15 r (deserv) s
101 c
13 r (credit) s
15 r (for) s
13 r (PP) s
-3 r 46 c
14 r 73 c
15 r (ap) s
1 r (ologise) s
13 r (to) s
14 r (an) s
121 c
13 r 73 c
15 r (ha) s
118 c
-1 r 101 c
224 2020 p (omitted.) s
31 r (Julian) s
19 r (Onions) s
20 r (of) s
19 r (Nottingham) s
17 r (Univ) s
(ersit) s
-1 r 121 c
17 r (has) s
20 r 119 c
(ork) s
-1 r (ed) s
18 r (on) s
19 r (the) s
20 r (PP) s
224 2077 p (design) s
12 r (from) s
10 r (the) s
13 r (start,) s
11 r (and) s
12 r (has) s
12 r (done) s
12 r 109 c
(uc) s
-1 r 104 c
10 r (of) s
12 r (the) s
12 r (implemen) s
-1 r (t) s
-1 r (ati) s
-1 r (on.) s
17 r (Phil) s
12 r (Co) s
1 r 99 c
(k-) s
224 2133 p (croft,) s
18 r (formerly) s
16 r (of) s
17 r (UCL,) s
18 r 119 c
(ork) s
-1 r (ed) s
17 r (on) s
17 r (the) s
19 r (early) s
17 r (design,) s
18 r (and) s
19 r (pro) s
1 r (duced) s
19 r 97 c
17 r (large) s
224 2190 p (amoun) s
116 c
14 r (of) s
16 r (the) s
17 r (\014rst) s
16 r (system.) s
23 r (Alina) s
16 r (daCruz,) s
17 r (Marshall) s
15 r (Rose,) s
17 r (Doug) s
16 r (Kingston,) s
224 2246 p (Bruce) s
20 r (Wilford,) s
19 r (Jim) s
18 r (Craigie,) s
19 r (Prof.) s
31 r 80 c
(eter) s
18 r (Kirstein,) s
20 r (Caroline) s
18 r (Platt,) s
19 r (Adrian) s
224 2303 p (Joseph,) s
16 r (George) s
15 r (Mic) s
(haelson,) s
14 r (and) s
16 r (Daniel) s
15 r (Karren) s
98 c
1 r (erg) s
15 r (ha) s
118 c
-1 r 101 c
14 r (all) s
15 r (con) s
(tributed) s
14 r (to) s
224 2359 p (PP) s
15 r (in) s
15 r 118 c
-2 r (arious) s
14 r 119 c
97 c
-1 r (ys.) s
18 r 80 c
(art) s
13 r (of) s
14 r (PP) s
15 r 119 c
(as) s
14 r (funded) s
16 r 98 c
121 c
14 r (the) s
15 r (Join) s
116 c
14 r (Net) s
119 c
-1 r (ork) s
13 r 84 c
-3 r (eam.) s
cmbx10.432 @sf
224 2502 p (12) s
70 r (Reference) s
-1 r 115 c
cmr10.329 @sf
216 2660 p ([1]) s
21 r (M.T.) s
11 r (Rose,) s
11 r (\\The) s
12 r (ISO) s
13 r (Dev) s
(elopm) s
-1 r (en) s
-1 r 116 c
9 r (En) s
(vironmen) s
-1 r (t:) s
9 r (User's) s
11 r (Man) s
(ual) s
10 r (\(v) s
(ersio) s
-1 r 110 c
286 2717 p (4.0\),") s
14 r (July) s
15 r (1988.) s
949 2916 p (12) s
@eop
11 @bop0
cmsy10.329 @sf
[<000000060000000007000000000300000000038000000001C000000000E00000000070FFFFFFFFFCFFFFFFFFFC0000000070
  00000000E000000001C00000000380000000030000000007000000000600> 38 16 -3 -3 45] 33 @dc
cmbx10.432 @sf
[<00FFC00007FFF8000FFFFE001FC07F003F000F807E0007807C0007C0FC0003C0F80003E0F80003E0F80007E0F80007E0F800
  1FE0FC007FE07C00FFE07E03FFE03F0FFFC01FBFFFC00FFFFF8007FFFF0003FFFE0007FFFC000FFFF8001FFFFC001FFFFE00
  3FFE3F003FF81F003FF01F803FC00F803F000F803E000F803E000F801E001F801E001F000F003F000FC0FE0007FFFC0001FF
  F800007FC000> 27 39 -3 0 34] 56 @dc
[<0000FFE00000000FFFFE0000003FFFFF800000FFC07FE00001FF001FF00003FE000FF80007FC0007FC000FF80003FE001FF0
  0001FF001FF00001FF003FE00000FF803FE00000FF807FE00000FFC07FE00000FFC07FC000007FC0FFC000007FE0FFC00000
  7FE0FFC000007FE0FFC000007FE0FFC000007FE0FFC000007FE0FFC000007FE0FFC000007FE0FFC000007FE0FFC000007FE0
  7FC000007FC07FC000007FC07FC000007FC07FE00000FFC03FE00000FF803FE00000FF801FF00001FF001FF00001FF000FF8
  0003FE0007F80003FC0003FC0007F80001FF001FF000007FC07FC000003FFFFF80000007FFFC00000000FFE00000> 43 41 -4 0 52] 79 @dc
[<FFFE3FFF80FFFE3FFF80FFFE3FFF800FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F800
  0FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FE003F8000FF003F8000FF003F800
  0FF803F8000FF803F8000FFE07F8000FEF87F0000FE7FFE0000FE3FFC0000FE0FF00000FE00000000FE00000000FE0000000
  0FE00000000FE00000000FE00000000FE00000000FE00000000FE00000000FE00000000FE00000000FE0000000FFE0000000
  FFE0000000FFE0000000> 33 42 -3 0 38] 104 @dc
[<FFFFFEFFFFFEFFFFFE01FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF
  0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001FF0001
  FF0001FF0001FF0001FF0001FF00FFFFFEFFFFFEFFFFFE> 23 41 -1 0 26] 73 @dc
[<FFFE000000FFFE000000FFFE0000000FE00000000FE00000000FE00000000FE00000000FE00000000FE00000000FE0000000
  0FE00000000FE00000000FE1FE00000FE7FFC0000FFFFFF0000FFE0FF8000FF803FC000FF001FE000FE001FE000FE000FF00
  0FE000FF000FE000FF800FE0007F800FE0007F800FE0007F800FE0007F800FE0007F800FE0007F800FE0007F800FE0007F80
  0FE000FF000FE000FF000FE001FE000FF001FE000FF803FC000FFF0FF800FFFFFFF000FFE7FFC000FFE1FF0000> 33 39 -2 12 38] 112 @dc
cmr10.329 @sf
[<007FFF0000007FFF00000003E000000003E000000003E000000003E000000003E000000003E000000003E000000003E00000
  0003E000000003E000000003E000000007E000000007F00000000FF80000000F980000001F9C0000001F0C0000003F060000
  007E060000007E03000000FC03800000F801800001F801C00001F000C00003F000600003E000700007E000F800FFFC03FF80
  FFFC03FF80> 33 31 0 0 34] 89 @dc
[<FFE7FFFFE7FF0F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00F00
  F00F00F0FFFFF0FFFFF00F00F00F00F00F00F00F00F00F00F00F00F00F01F00781F007C1F003F1F000FFF0001FF0> 24 32 0 0 25] 13 @dc
11 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
cmsy10.329 @sf
292 195 p 15 c
cmr10.329 @sf
23 r (Single) s
15 r (Message.) s
cmsy10.329 @sf
292 289 p 15 c
cmr10.329 @sf
23 r (An) s
15 r (MT) s
-3 r (A/Channel) s
14 r (pair.) s
cmsy10.329 @sf
292 383 p 15 c
cmr10.329 @sf
23 r 65 c
15 r (whole) s
15 r 99 c
(hannel.) s
cmsy10.329 @sf
292 477 p 15 c
cmr10.329 @sf
23 r (The) s
15 r (en) s
(tire) s
14 r (queue.) s
295 583 p (The) s
15 r (manager) s
13 r (or) s
15 r (user) s
15 r (can) s
15 r (read) s
15 r (selected) s
15 r (informati) s
-1 r (on) s
13 r (ab) s
1 r (out) s
15 r (progress.) s
19 r (Fil-) s
224 639 p (ters) s
15 r (ma) s
-1 r 121 c
13 r (also) s
14 r 98 c
1 r 101 c
16 r (applied,) s
15 r (so) s
15 r (that) s
14 r (the) s
15 r (user) s
16 r (can) s
15 r (ask) s
15 r (\\T) s
-3 r (ell) s
13 r (me) s
14 r (ab) s
1 r (out) s
15 r (all) s
14 r (of) s
15 r 109 c
121 c
224 696 p (messages".) s
32 r (The) s
20 r (manager) s
18 r (can) s
20 r (turn) s
20 r (pro) s
1 r (cessing) s
20 r (on) s
19 r (or) s
20 r (o\013) s
19 r (at) s
19 r (an) s
121 c
18 r (lev) s
(el,) s
19 r (and) s
224 752 p (mo) s
1 r (dify) s
17 r (the) s
18 r (con) s
(ten) s
(ts) s
16 r (of) s
18 r 112 c
1 r (er) s
19 r (message) s
17 r (and) s
18 r 112 c
1 r (er) s
19 r (host) s
18 r (cac) s
(hes.) s
29 r (There) s
18 r (is) s
18 r (almost) s
224 809 p (certainly) s
19 r (scop) s
1 r 101 c
21 r (for) s
19 r (co) s
1 r (op) s
1 r (erating) s
19 r (QMGRs) s
20 r (to) s
19 r (optimise) s
18 r (pro) s
1 r (cessing) s
20 r (in) s
20 r 97 c
19 r (lo) s
1 r (cal) s
224 865 p (en) s
(vironmen) s
-1 r 116 c
14 r (\(e.g.) s
23 r 98 c
121 c
16 r (sharing) s
16 r (informat) s
-1 r (ion) s
15 r (on) s
16 r (MT) s
-3 r 65 c
16 r (failures\).) s
22 r (The) s
17 r (manager) s
224 922 p (ma) s
-1 r 121 c
14 r (also) s
14 r 99 c
(ho) s
1 r (ose) s
14 r (to) s
15 r (force) s
15 r (imm) s
-1 r (ediat) s
-1 r 101 c
14 r (pro) s
1 r (cessing) s
15 r (of) s
15 r (sp) s
1 r (eci\014c) s
16 r (messages.) s
295 978 p (The) s
14 r (Queue) s
15 r (Manger) s
14 r (will) s
12 r (also) s
13 r (con) s
(trol) s
12 r (clean) s
(up) s
13 r (of) s
14 r (the) s
14 r (queue.) s
20 r (Sp) s
1 r (ecial) s
14 r 99 c
(han-) s
224 1034 p (nels) s
14 r (will) s
12 r 98 c
1 r 101 c
14 r (in) s
118 c
(o) s
-1 r 107 c
-1 r (ed) s
12 r (to) s
13 r (generate) s
14 r 119 c
(arning) s
11 r (messages,) s
12 r (and) s
14 r (to) s
13 r (generate) s
14 r (Deliv) s
(ery) s
224 1091 p (Noti\014cations) s
11 r (on) s
13 r (message) s
11 r (timeout.) s
17 r 65 c
12 r 99 c
(hannel) s
12 r (will) s
11 r 98 c
1 r 101 c
13 r (used) s
13 r (to) s
12 r (delete) s
13 r (messages) s
224 1147 p (whic) s
104 c
14 r (do) s
15 r (not) s
15 r (require) s
15 r (further) s
15 r (pro) s
1 r (cessing.) s
295 1204 p (The) s
16 r (full) s
16 r (functionalit) s
-1 r 121 c
15 r (of) s
16 r (the) s
16 r (QMGR) s
17 r (proto) s
1 r (cols) s
15 r (will) s
15 r 98 c
1 r 101 c
17 r (used) s
17 r 98 c
121 c
15 r (an) s
17 r (MT) s
-3 r 65 c
224 1260 p (Console) s
19 r (pro) s
1 r (cess.) s
35 r (This) s
19 r (will) s
19 r (giv) s
101 c
18 r 97 c
20 r 112 c
1 r (ermanen) s
116 c
18 r (displa) s
121 c
18 r (of) s
20 r (the) s
20 r (status) s
19 r (of) s
19 r (the) s
224 1317 p (MT) s
-3 r (A,) s
16 r (refreshed) s
17 r (at) s
17 r (appropriate) s
16 r (in) s
(terv) s
-2 r (al) s
-1 r (s.) s
24 r (This) s
17 r (will) s
15 r 98 c
1 r 101 c
18 r (orien) s
(ted) s
16 r (to) s
16 r 97 c
17 r (colour) s
224 1373 p (displa) s
121 c
-3 r 44 c
19 r (so) s
20 r (that) s
19 r (error) s
20 r (states) s
19 r (can) s
21 r 98 c
1 r 101 c
21 r (indicated) s
20 r 98 c
121 c
19 r 97 c
21 r 99 c
(hange) s
19 r (from) s
19 r (green) s
cmsy10.329 @sf
20 r 33 c
cmr10.329 @sf
224 1430 p 121 c
(ello) s
-1 r 119 c
cmsy10.329 @sf
13 r 33 c
cmr10.329 @sf
15 r (red.) s
20 r (This) s
15 r (should) s
15 r (substan) s
(tiall) s
-1 r 121 c
14 r (help) s
15 r (pro) s
(vision) s
13 r (of) s
15 r (MT) s
-3 r 65 c
14 r (services.) s
cmbx10.432 @sf
224 1573 p 56 c
69 r (Other) s
23 r (Im) s
-1 r (pl) s
-1 r (em) s
-3 r (e) s
-1 r 110 c
-1 r (t) s
-1 r (ation) s
21 r (Asp) s
2 r (ects) s
cmr10.329 @sf
224 1674 p (There) s
16 r (are) s
15 r 97 c
15 r (few) s
15 r (asp) s
1 r (ects) s
15 r (of) s
15 r (the) s
15 r (implemen) s
-1 r (ta) s
-1 r (tio) s
-1 r 110 c
14 r (whic) s
104 c
14 r (are) s
15 r 119 c
(orth) s
-1 r 121 c
13 r (of) s
15 r (sp) s
1 r (eci\014c) s
224 1731 p (note.) s
22 r (The) s
16 r (\014rst) s
16 r (is) s
15 r (the) s
16 r (use) s
16 r (of) s
16 r (the) s
16 r (ISODE) s
16 r (pac) s
107 c
-2 r (age) s
14 r ([1].) s
21 r (This) s
16 r (has) s
16 r (pro) s
(vided) s
14 r (the) s
224 1787 p (underlying) s
17 r (OSI) s
19 r (required) s
17 r (for) s
17 r (X.400,) s
16 r (and) s
18 r (the) s
17 r (QMGR) s
18 r 82 c
(OS.) s
17 r (The) s
17 r (PEPY) s
17 r (to) s
1 r (ol) s
224 1844 p (has) s
17 r 98 c
1 r (een) s
18 r (used) s
18 r (to) s
16 r (automati) s
-1 r (call) s
-1 r 121 c
15 r (generate) s
17 r (the) s
17 r (X.400) s
16 r (ASN.1,) s
16 r (and) s
18 r (the) s
17 r 82 c
(OSY) s
224 1900 p (and) s
16 r (POSY) s
15 r (to) s
1 r (ols) s
14 r (ha) s
118 c
101 c
13 r 98 c
1 r (een) s
17 r (used) s
15 r (to) s
15 r (automate) s
13 r (the) s
15 r (QMGR) s
15 r (proto) s
1 r (cols.) s
295 1957 p (The) s
17 r (whole) s
17 r (PP) s
18 r (system) s
16 r (uses) s
17 r 97 c
17 r (\015exible) s
18 r (run) s
(tim) s
-1 r 101 c
16 r (tailoring) s
15 r (system.) s
25 r (This) s
17 r (is) s
224 2013 p (used) s
16 r (to) s
14 r (set) s
15 r 118 c
-2 r (arious) s
14 r (things) s
15 r (whic) s
104 c
14 r (ma) s
-1 r 121 c
13 r 118 c
-2 r (ary) s
14 r (across) s
15 r 97 c
15 r (site.) s
19 r (In) s
16 r (particular:) s
cmsy10.329 @sf
292 2119 p 15 c
cmr10.329 @sf
23 r (Proto) s
1 r (col) s
14 r (and) s
15 r 70 c
-3 r (orm) s
-1 r (att) s
-1 r (ing) s
13 r (Channel) s
15 r (De\014nitions) s
cmsy10.329 @sf
292 2213 p 15 c
cmr10.329 @sf
23 r (Addressing) s
15 r 84 c
-3 r (ables) s
cmsy10.329 @sf
292 2307 p 15 c
cmr10.329 @sf
23 r (De\014nitions) s
14 r (of) s
15 r (Enco) s
1 r (ded) s
16 r (Information) s
13 r 84 c
(yp) s
1 r (es) s
295 2413 p (This) s
12 r (allo) s
(w) s
-1 r 115 c
11 r (for) s
13 r 118 c
(ery) s
11 r (rapid) s
13 r (extension) s
12 r (to) s
13 r (add) s
13 r (in) s
12 r (new) s
14 r (net) s
119 c
-1 r (ork) s
-1 r 115 c
11 r (or) s
13 r (reformat) s
-1 r 45 c
224 2470 p (ting.) s
295 2526 p (Extensiv) s
101 c
13 r (use) s
16 r (is) s
15 r (made) s
14 r (of) s
15 r (logging,) s
13 r (whic) s
104 c
14 r (can) s
15 r 98 c
1 r 101 c
16 r (tailored) s
14 r (to) s
15 r 97 c
15 r (wide) s
15 r (range) s
224 2583 p (of) s
13 r (lev) s
(els.) s
18 r (Besides) s
14 r (basic) s
13 r (logs) s
13 r (for) s
13 r (op) s
1 r (erator,) s
13 r (statisti) s
-1 r (cs,) s
12 r (and) s
14 r (debugging,) s
13 r (it) s
13 r (is) s
14 r (also) s
224 2639 p 112 c
1 r (ossible) s
15 r (to) s
15 r (select) s
15 r (logs) s
14 r (for) s
14 r (sp) s
1 r (eci\014c) s
16 r 99 c
(hannels) s
15 r (or) s
14 r (sp) s
1 r (eci\014c) s
17 r (messages.) s
949 2916 p (11) s
@eop
10 @bop0
cmr10.329 @sf
[<FFE3FF3FF8FFE3FF3FF80F007807800F007807800F007807800F007807800F007807800F007807800F007807800F00780780
  0F007807800F007807800F007807800F007807800F007807800F007807800F007807800F00780780FFFFFFFF80FFFFFFFF80
  0F007800000F007800000F007800000F007800000F007800000F00F80F800F00F80F800780FC0F8007C0FE0F8003F07F8780
  00FFF7FF00001FC0FE00> 37 32 0 0 38] 14 @dc
cmbx10.432 @sf
[<0078000000FC000001FE000001FE000001FE000001FE000001FE000001FE000001FE000001FE000000FE000000FE000000FE
  000000FE0000007E0000007E0000007F0000003F0000003F0000003F0000001F0000000F8000000F80000007C0000007C000
  E003E000E001F000E000F8007000780070007C0070003E007FFFFF007FFFFF807FFFFFC07FFFFFC03FFFFFE03FFFFFF03FFF
  FFF03FFFFFF03E00000038000000> 28 41 -4 0 34] 55 @dc
[<00000000FC0000000001FE0000000003FF0000000007FF8000000007FFC00000000FFFC00000000FFFC00000000FFFE00000
  000FFFE00000001FFFE00000001FC1E00000001F80600000FFFF0060000FFFFE0000003FFFFF800000FFE07FE00001FFC07F
  F00003FFC07FF80007FDE1F7FC000FF8FFE3FE001FF07FC1FF001FF03F01FF003FE00000FF803FE00000FF807FE00000FFC0
  7FC000007FC07FC000007FC0FFC000007FE0FFC000007FE0FFC000007FE0FFC000007FE0FFC000007FE0FFC000007FE0FFC0
  00007FE0FFC000007FE0FFC000007FE0FFC000007FE07FC000007FC07FC000007FC07FC000007FC07FE00000FFC03FE00000
  FF803FE00000FF801FF00001FF001FF00001FF000FF80003FE0007FC0007FC0003FE000FF80001FF001FF000007FC07FC000
  003FFFFF80000007FFFC00000000FFE00000> 43 53 -4 12 52] 81 @dc
[<FFFE007003FFFFC0FFFE00F803FFFFC0FFFE00F803FFFFC0038000F8000FF000038001FC000FF000038001FC000FF0000380
  03FE000FF000038003FE000FF000038007FF000FF000038007FF000FF000038007FF000FF00003800FF3800FF00003800FF3
  800FF00003801FE1C00FF00003801FE1C00FF00003803FC0E00FF00003803FC0E00FF00003803FC0E00FF00003807F80700F
  F00003807F80700FF0000380FF00380FF0000380FF00380FF0000381FE001C0FF0000381FE001C0FF0000381FE001C0FF000
  0383FC000E0FF0000383FC000E0FF0000387F800070FF0000387F800070FF000038FF000038FF000038FF000038FF000038F
  F000038FF000039FE00001CFF000039FE00001CFF00003BFC00000EFF00003BFC00000EFF00003FF8000007FF00003FF8000
  007FF000FFFF8000007FFFC0FFFF0000003FFFC0FFFF0000003FFFC0> 58 41 -3 0 65] 77 @dc
/cmti10.329 @newfont
cmti10.329 @sf
[<1C01F03E03F81E079C1E078E1E07C61F03C60F03C70F03E00F01E00F81E00781F00780F00780F0E7C0F063C0F863E07873F0
  7833F8F03F9FF01F0FC0> 24 20 -3 0 26] 110 @dc
[<0FE0001FFC003C3E00780700700300700000F00000F00000F00000F800007FF0007FFC00781F007C07003C03801E01800F03
  8007C78003FF0000FE00> 17 20 -4 0 21] 101 @dc
[<03F00007F8000F9C000F0E000F07000F03000F03000F01800F01800F81800781C00780C007C0C0E3C0C063C0E063E1E071E3
  E039E3E01FC3E00F81C0> 19 20 -3 0 21] 118 @dc
[<1C00003E00001E00001E00001E00001F00000F00000F00000F00000F8000078000078000078000E7C38063C3C063C3C073E3
  C033F1C03FBF801F1F00> 18 20 -3 0 19] 114 @dc
[<1F0F803FDFC079FCC0707CE0F03C60F03C60F03E70F01E00F01E00F81E00781F00780F00780F003C0F003C0F801E07800F0F
  80079FC003FFC000FB80> 20 20 -4 0 23] 97 @dc
[<3E007F00F300F380F180F980F9C0780078007C007C003C003C003E003E001E001E001F001F000F000F000F800F8007800780
  07C007C003C003C01FE01FE003E0> 11 32 -3 0 12] 108 @dc
[<0FFF000FFF0000F800007800007800007800007C00003C00003C001FBC003FFE0078FE00707E00F03E00F01F00F00F00F00F
  00F00F00F80F807807807807807807803C07C03C03C01E07C00F07C0078FE003FFE000F860> 19 29 -4 9 21] 113 @dc
[<03E3E007FFF00F3F300F0F380F0F180F0F180F0F9C0F07800F07800F87800787C00783C007C3C0E3C3C063C3E063E1E071E1
  E039E1F01FC1F00F80E0> 22 20 -3 0 24] 117 @dc
[<0FC0003FF8003C7C00781E00700F00F00780F00780F003C0F003C0F803E07803E07801E07801E03C01E03C01E01E01C00F03
  C007C78003FF80007E00> 19 20 -4 0 23] 111 @dc
[<1F003F8079C078E078607C607C703C003C003E003E001E001E001F001F000F000F000F80FFF0FFF0078007C007C003C003C0
  03E003E001C0> 12 28 -4 0 15] 116 @dc
[<7007C0F80FE0781E70781E38781F187C0F183C0F1C3C0F803C07803E07801E07C01E03C01E03C01F03C00F03E00F81E00FC1
  E00FE3C007FFC007BF0007800007C00003C00003C00003C00003E00001E00001E00001E0000FF0000FF00001F000> 22 32 -2 0 23] 104 @dc
[<1F0F803FDFC079FCC0707CE0F03C60F03E60F03E70F01E00F01E00F81F00781F00780F00780F003C0F803C0F801E07800F0F
  80079FC003FFC000FBC00003C00003E00003E00001E00001E00001F00001F00000F00000F00007F80007F80000F8> 21 32 -4 0 23] 100 @dc
[<1F003F803DC03CE03C603E601E701E001F000F000F000F800780E78067C063C073C03BC01FC00F8000000000000000000000
  0000000000E000F000F000F0> 12 31 -3 0 14] 105 @dc
[<1FC07FF0F078F03CF01CF01C701E003E03FE07FE0FFC1FF81FF01F000E0F0E0F0F0F078703FE00FC> 16 20 -3 0 19] 115 @dc
[<700F80F81FC0783DC0783CE0783C607C3C603C3E703C3E003C7C003FFC001FF0001FC0001FC0001FE0E00F70F00F38F00F1C
  F00F8E300787F00781E007800007C00003C00003C00003C00003E00001E00001E00001E0000FF0000FF00001F000> 20 32 -2 0 21] 107 @dc
10 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
224 195 p (to) s
17 r (asso) s
1 r (ciate) s
16 r (it) s
16 r (with,) s
17 r (so) s
17 r (that) s
17 r 97 c
16 r (giv) s
(en) s
16 r (recipien) s
116 c
16 r (can) s
17 r 98 c
1 r 101 c
18 r (mark) s
-1 r (ed) s
16 r (as) s
17 r (\\deliv) s
(ered) s
224 252 p (and) s
14 r (con\014rmed".) s
19 r (This) s
13 r (is) s
13 r (lik) s
(ely) s
12 r (to) s
13 r (sa) s
118 c
-1 r 101 c
12 r (on) s
13 r (net) s
119 c
(o) s
-1 r (rk) s
12 r (tra\016c.) s
18 r (More) s
13 r (imp) s
1 r (ortan) s
-1 r (tly) s
-4 r 44 c
224 308 p (it) s
13 r (will) s
12 r (pro) s
(vide) s
13 r (the) s
13 r (return) s
14 r (of) s
13 r (con) s
(ten) s
116 c
11 r (service) s
14 r (\(whic) s
104 c
12 r (is) s
13 r (particularly) s
12 r (imp) s
1 r (ortan) s
-1 r 116 c
224 364 p (for) s
18 r (proto) s
1 r (col) s
18 r (con) s
118 c
-1 r (ersion\)) s
16 r (in) s
19 r (the) s
18 r (ligh) s
116 c
17 r (of) s
18 r (man) s
-1 r 121 c
17 r (X.400) s
17 r (services) s
19 r (whic) s
104 c
17 r (do) s
19 r (not) s
224 421 p (pro) s
(vide) s
20 r (it) s
20 r (as) s
21 r 97 c
20 r (remote) s
20 r (service.) s
37 r (The) s
21 r (utilit) s
121 c
18 r (of) s
21 r (this) s
21 r (approac) s
104 c
19 r (will) s
20 r (dep) s
1 r (end) s
224 477 p 118 c
(ery) s
18 r 109 c
-1 r (uc) s
104 c
17 r (on) s
19 r (PTT) s
18 r (MHS) s
19 r 99 c
(harging) s
18 r (strategies.) s
29 r (Asso) s
1 r (ciated) s
19 r (with) s
18 r (this) s
19 r (is) s
18 r (the) s
224 534 p (option) s
13 r (to) s
12 r (sp) s
1 r (ecify) s
14 r (\\reliable) s
12 r (deliv) s
(ery",) s
11 r (where) s
14 r (PP) s
13 r (will) s
12 r (sp) s
1 r (ecify) s
13 r 112 c
1 r (ositiv) s
101 c
12 r (Deliv) s
(ery) s
224 590 p (Noti\014cation,) s
14 r (and) s
16 r (not) s
16 r (dequeue) s
17 r (the) s
16 r (message) s
14 r (un) s
(til) s
14 r (it) s
15 r (arriv) s
(es.) s
20 r (This) s
15 r (ma) s
-1 r 121 c
14 r (pro) s
118 c
-1 r 101 c
224 647 p (to) s
15 r 98 c
1 r 101 c
16 r 97 c
14 r 118 c
-2 r (aluable) s
14 r (user) s
16 r (service.) s
cmbx10.432 @sf
224 787 p 55 c
69 r (Queue) s
22 r (Managemen) s
-2 r 116 c
cmr10.329 @sf
224 888 p (The) s
13 r (QMGR) s
12 r (\(queue) s
12 r (manager\)) s
10 r (is) s
12 r (an) s
12 r (in) s
(teresting) s
10 r (comp) s
1 r (onen) s
116 c
10 r (of) s
12 r (PP) s
-3 r 46 c
11 r (Exp) s
1 r (erience) s
224 945 p (has) s
16 r (sho) s
(wn) s
15 r (that) s
16 r (disk) s
16 r (based) s
16 r (queues) s
17 r (cause) s
17 r (serious) s
16 r 112 c
1 r (erformance) s
15 r (problems) s
15 r (due) s
224 1001 p (to) s
20 r (the) s
20 r 111 c
118 c
(erhead) s
18 r (of) s
20 r (reading) s
20 r 97 c
21 r (large) s
19 r (queue,) s
22 r 116 c
(ypically) s
18 r (stored) s
20 r (in) s
20 r 97 c
20 r (directory) s
-3 r 46 c
224 1057 p (Therefore) s
20 r (the) s
19 r (QMGR) s
20 r (is) s
19 r (memo) s
-1 r (ry) s
18 r (based,) s
20 r (and) s
cmti10.329 @sf
20 r (never) s
cmr10.329 @sf
19 r (accesses) s
20 r (the) s
19 r (queue) s
21 r (on) s
224 1114 p (disk.) s
26 r (The) s
17 r (QMGR) s
17 r (accesses) s
17 r (the) s
18 r (external) s
16 r 119 c
(orld) s
15 r 98 c
121 c
16 r (use) s
18 r (of) s
16 r 97 c
17 r 82 c
(OS) s
17 r (based) s
18 r (pro-) s
224 1170 p (to) s
1 r (col) s
16 r (whic) s
104 c
15 r (supp) s
1 r (orts) s
17 r 97 c
17 r (wide) s
16 r (range) s
16 r (of) s
17 r (functionalit) s
-1 r (y[) s
-1 r (20].) s
22 r (This) s
16 r (usage) s
16 r (of) s
17 r 82 c
(OS) s
224 1227 p (means) s
15 r (that) s
15 r (the) s
15 r (QMGR) s
16 r (do) s
1 r (es) s
16 r (not) s
15 r (ev) s
(en) s
15 r (ha) s
118 c
101 c
14 r (to) s
15 r 98 c
1 r 101 c
16 r (residen) s
116 c
14 r (on) s
16 r (the) s
15 r (same) s
15 r (ma-) s
224 1283 p 99 c
(hine) s
12 r (as) s
13 r (the) s
13 r (queue,) s
14 r (and) s
13 r (one) s
13 r (QMGR) s
13 r (could) s
13 r (manage) s
11 r (sev) s
(eral) s
11 r (queues.) s
20 r (Pro) s
1 r (cesses) s
224 1340 p (whic) s
104 c
13 r (are) s
14 r (in) s
118 c
-1 r (ok) s
-1 r (ed) s
12 r 98 c
121 c
13 r (the) s
14 r (QMGR) s
14 r (are) s
14 r (initialised) s
12 r (in) s
14 r (exactly) s
14 r (the) s
14 r (same) s
12 r (manner) s
224 1396 p (as) s
16 r (an) s
121 c
15 r (other) s
15 r (ISODE) s
17 r (serv) s
(er.) s
21 r (The) s
16 r (\014rst) s
16 r (op) s
1 r (eration) s
15 r 97 c
16 r (QMGR) s
16 r 109 c
(ust) s
14 r (apply) s
16 r (is) s
15 r (to) s
224 1453 p (access) s
13 r 97 c
11 r (pro) s
1 r (cess) s
13 r (whic) s
104 c
11 r (will) s
11 r (read) s
12 r (the) s
12 r (queue.) s
20 r (Whilst) s
11 r (this) s
12 r 109 c
(ust) s
10 r 98 c
1 r 101 c
12 r (sync) s
(hronous) s
224 1509 p (at) s
17 r (startup,) s
18 r (later) s
17 r (usage) s
18 r (ma) s
-1 r 121 c
16 r 98 c
1 r 101 c
19 r (async) s
(hronous,) s
17 r (to) s
17 r (ensure) s
19 r (that) s
17 r (the) s
18 r (QMGR's) s
224 1566 p (list) s
14 r (re\015ects) s
15 r (the) s
cmti10.329 @sf
15 r 114 c
-1 r 101 c
-2 r (al) s
14 r (queue) s
17 r (on) s
15 r (the) s
17 r (disk) s
cmr10.329 @sf
46 c
19 r (It) s
15 r (ma) s
-1 r 121 c
13 r (then) s
15 r (start) s
14 r 99 c
(hannels) s
14 r 98 c
121 c
14 r (estab-) s
224 1622 p (lishing) s
16 r (asso) s
1 r (ciations,) s
15 r (and) s
17 r (sending) s
17 r (comma) s
-1 r (nds) s
16 r (to) s
16 r (deliv) s
(er) s
15 r (messages) s
15 r (to) s
16 r (explicit) s
224 1678 p (recipien) s
(ts.) s
19 r (As) s
15 r (the) s
15 r (QMGR) s
16 r (do) s
1 r (es) s
15 r (no) s
15 r (pro) s
1 r (cessing) s
15 r (of) s
15 r (messages,) s
14 r (it) s
14 r (should) s
15 r 98 c
1 r 101 c
16 r (fast) s
224 1735 p (and) s
16 r (able) s
14 r (to) s
15 r (con) s
(trol) s
13 r (man) s
121 c
13 r 99 c
(hannels.) s
295 1791 p (The) s
16 r (queue) s
17 r (message) s
15 r (con) s
(trol) s
14 r (\014les) s
16 r (are) s
16 r (read) s
16 r (in) s
(to) s
14 r (primary) s
14 r (memor) s
-1 r 121 c
-3 r 44 c
14 r (and) s
16 r (or-) s
224 1848 p (dered) s
17 r (in) s
(to) s
14 r 97 c
17 r (link) s
(ed) s
15 r (structure) s
16 r (with) s
15 r (complex) s
15 r (cross-links) s
16 r (sorting) s
15 r (the) s
16 r (queue) s
18 r (in) s
224 1904 p (sev) s
(eral) s
18 r (fashions.) s
32 r (The) s
20 r (queue) s
21 r (can) s
20 r (then) s
20 r 98 c
1 r 101 c
20 r (sc) s
(heduled) s
19 r (on) s
20 r (the) s
20 r (basis) s
19 r (of) s
19 r (man) s
121 c
224 1961 p (criteria,) s
14 r (including:) s
cmsy10.329 @sf
292 2042 p 15 c
cmr10.329 @sf
23 r (Main) s
(t) s
-1 r (aining) s
15 r 97 c
17 r (connection) s
17 r (with) s
17 r 97 c
17 r (remote) s
15 r (MT) s
-3 r 65 c
16 r (\(this) s
17 r (can) s
17 r 98 c
1 r 101 c
18 r 97 c
(wkw) s
-1 r (a) s
-1 r (rd) s
338 2099 p (with) s
14 r 97 c
15 r (disk) s
15 r (based) s
16 r (queue\).) s
cmsy10.329 @sf
292 2185 p 15 c
cmr10.329 @sf
23 r (Keeping) s
16 r 97 c
14 r 99 c
(hannel) s
15 r (asso) s
1 r (ciation) s
13 r (op) s
1 r (en.) s
cmsy10.329 @sf
292 2272 p 15 c
cmr10.329 @sf
23 r (Pro) s
1 r (cessing) s
15 r (high) s
15 r (priorit) s
-1 r 121 c
13 r (messages) s
14 r (rapidly) s
-3 r 46 c
cmsy10.329 @sf
292 2359 p 15 c
cmr10.329 @sf
23 r (Handling) s
14 r (timeouts/connection) s
13 r (failures) s
15 r (to) s
14 r (remote) s
14 r (MT) s
-3 r (As.) s
cmsy10.329 @sf
292 2446 p 15 c
cmr10.329 @sf
23 r (Reducing) s
16 r (pro) s
1 r (cessing) s
15 r (e\013ort) s
14 r (when) s
16 r (the) s
15 r (CPU) s
15 r (load) s
14 r 97 c
118 c
(era) s
-1 r (ge) s
13 r (increases.) s
cmsy10.329 @sf
292 2532 p 15 c
cmr10.329 @sf
23 r (Sc) s
(heduling) s
14 r (to) s
15 r (optimise) s
13 r (PTT) s
15 r (MHS) s
15 r (tari\013s.) s
295 2614 p (The) s
16 r (QMGR) s
16 r (also) s
15 r (allo) s
(w) s
-1 r 115 c
14 r (for) s
16 r 97 c
15 r 110 c
(um) s
-1 r 98 c
1 r (er) s
15 r (of) s
15 r (useful) s
17 r (managem) s
-1 r (en) s
-1 r 116 c
14 r (asp) s
1 r (ects.) s
22 r (It) s
224 2670 p (will) s
11 r (con) s
(trol) s
10 r (queue) s
13 r (main) s
(t) s
-1 r (enance,) s
11 r (and) s
13 r (send) s
13 r (o\013) s
11 r 119 c
(arning) s
11 r (and) s
12 r (timeout) s
10 r (messages.) s
224 2727 p (This) s
19 r (is) s
19 r 97 c
19 r 118 c
-2 r (aluable) s
18 r (function) s
20 r (not) s
19 r (pro) s
(vided) s
18 r 98 c
121 c
18 r (X.400.) s
32 r (In) s
20 r (addition) s
18 r (there) s
20 r (are) s
224 2783 p (op) s
1 r (erations) s
14 r (to) s
15 r (monitor) s
13 r (and) s
15 r (con) s
(trol) s
13 r (the) s
15 r (queue) s
16 r (at) s
15 r 118 c
-2 r (arious) s
14 r (lev) s
(els:) s
949 2916 p (10) s
@eop
9 @bop0
cmbx10.432 @sf
[<007FE00001FFF80003FFFC0007F0FF000FE07F801FC03F803F803FC03F803FC07F803FE07F803FE07F803FE07F803FE0FF80
  3FE0FF803FE0FF803FE0FF803FE0FFC03FE0FFC03FC0FFC03FC0FFE03F80FFE07F00FFFFFE00FFBFFC00FF9FF000FF820000
  7F8000007F8000007F801E007FC03F003FC07F803FC07F801FC07F800FE07F800FF03F8007F81F0001FE1F0000FFFE00003F
  FC000007F800> 27 39 -3 0 34] 54 @dc
[<FFFFF83FFFFEFFFFF83FFFFEFFFFF83FFFFE03FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE
  0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000
  FF8003FE0000FF8003FE0000FF8003FE0000FF8003FFFFFFFF8003FFFFFFFF8003FFFFFFFF8003FE0000FF8003FE0000FF80
  03FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE
  0000FF8003FE0000FF8003FE0000FF8003FE0000FF8003FE0000FF80FFFFF83FFFFEFFFFF83FFFFEFFFFF83FFFFE> 47 41 -3 0 54] 72 @dc
[<FFFFFFFC0000FFFFFFFF8000FFFFFFFFE00003FE003FF80003FE0007FC0003FE0001FE0003FE0000FF0003FE00007F8003FE
  00007FC003FE00003FC003FE00003FE003FE00001FE003FE00001FF003FE00001FF003FE00001FF003FE00001FF803FE0000
  1FF803FE00001FF803FE00001FF803FE00001FF803FE00001FF803FE00001FF803FE00001FF803FE00001FF803FE00001FF8
  03FE00001FF003FE00001FF003FE00001FF003FE00001FF003FE00001FE003FE00003FE003FE00003FC003FE00007FC003FE
  00007F8003FE0000FF0003FE0001FE0003FE0007FC0003FE003FF800FFFFFFFFE000FFFFFFFF8000FFFFFFF80000> 45 41 -3 0 53] 68 @dc
[<FFFE00000380FFFE00000780FFFE00000F80038000000F80038000001F80038000003F80038000007F8003800000FF800380
  0001FF8003800001FF8003800003FF8003800007FF800380000FFF800380001FFF800380001FFB800380003FF3800380007F
  E380038000FFE380038001FFC380038003FF8380038003FF0380038007FE038003800FFE038003801FFC038003803FF80380
  03807FF0038003807FE003800380FFC003800381FFC003800383FF8003800387FF0003800387FE000380038FFC000380039F
  FC00038003BFF800038003FFF000038003FFE000038003FFC0000380FFFF8000FFFEFFFF8000FFFEFFFF0000FFFE> 47 41 -3 0 54] 78 @dc
[<7FFF1FFFC07FFF1FFFC07FFF1FFFC007F001FC0007F001FC0007F001FC0007F001FC0007F001FC0007F001FC0007F001FC00
  07F001FC0007F001FC0007F001FC0007F001FC0007F001FC0007F001FC0007F001FC0007F001FC0007F001FC0007F001FC00
  07F001FC0007F001FC0007F001FC0007F001FC00FFFFFFFC00FFFFFFFC00FFFFFFFC0007F000000007F000000007F0000000
  07F000000007F000F00007F001F80007F003FC0007F003FC0007F803FC0003F803FC0001FE01FC0000FF80F800007FFFF000
  001FFFE0000001FF8000> 34 42 -1 0 38] 12 @dc
9 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
295 195 p (In) s
15 r (terms) s
12 r (of) s
14 r (the) s
15 r 111 c
118 c
-1 r (eral) s
-1 r 108 c
12 r (queue) s
15 r (structure,) s
14 r 97 c
14 r (reformatti) s
-1 r (ng) s
13 r 99 c
(hannel) s
13 r (will) s
13 r (tak) s
101 c
224 252 p (the) s
16 r (message) s
14 r (directory) s
14 r (of) s
15 r 97 c
15 r (giv) s
(en) s
14 r (name,) s
14 r (and) s
15 r (then) s
16 r (create) s
15 r 97 c
15 r (parallel) s
14 r (directory) s
224 308 p (with) s
17 r (the) s
18 r (new,) s
17 r (reformatted) s
16 r (message) s
16 r (in) s
17 r (it.) s
26 r (The) s
18 r (new) s
17 r (directory) s
17 r (will) s
16 r (ha) s
118 c
101 c
15 r (the) s
224 364 p (name) s
19 r (of) s
19 r (the) s
20 r (old) s
19 r (one,) s
21 r (with) s
19 r (the) s
20 r (name) s
19 r (of) s
19 r (the) s
20 r (formatti) s
-1 r (ng) s
18 r 99 c
(hannel) s
19 r (app) s
1 r (ended) s
224 421 p (\(e.g.) s
21 r (when) s
17 r 99 c
(hannel) s
15 r (\\bitmap2f) s
-1 r (ax") s
14 r (is) s
15 r (applied) s
16 r (to) s
15 r (message) s
14 r (directory) s
16 r (\\base",) s
15 r 97 c
224 477 p (directory) s
16 r (\\base.bitma) s
-1 r (p2fax") s
14 r (is) s
16 r (created\).) s
23 r (This) s
17 r (will) s
15 r (ensure) s
17 r (uniqueness) s
17 r (when) s
224 534 p 97 c
17 r (message) s
16 r (undergo) s
1 r (es) s
18 r (sev) s
(eral) s
15 r (complex) s
16 r (sequences) s
18 r (of) s
17 r (transformat) s
-1 r (ions) s
15 r (for) s
17 r (dif-) s
224 590 p (feren) s
116 c
14 r (recipien) s
(ts,) s
13 r (and) s
16 r (will) s
14 r (optimi) s
-1 r (se) s
14 r (reformat) s
-1 r (ting) s
13 r (for) s
14 r 109 c
(ulti) s
-1 r (ple) s
13 r (recipien) s
(ts.) s
295 647 p (There) s
22 r (are) s
21 r 116 c
119 c
-1 r 111 c
20 r (basic) s
22 r 116 c
(yp) s
1 r (es) s
20 r (of) s
22 r (reformatt) s
-1 r (ing:) s
31 r (those) s
22 r (whic) s
104 c
20 r (preserv) s
101 c
21 r (the) s
224 703 p (directory) s
19 r (structure,) s
20 r (and) s
19 r (those) s
20 r (whic) s
104 c
18 r (do) s
19 r (not.) s
33 r (The) s
19 r 99 c
(hannels) s
19 r (whic) s
104 c
18 r 99 c
(hange) s
224 760 p (directory) s
12 r (structure) s
12 r (are) s
12 r 116 c
(ypicall) s
-1 r 121 c
10 r (single) s
12 r (pro) s
1 r (cesses) s
13 r (sp) s
1 r (eci\014cally) s
12 r (designed) s
13 r (for) s
11 r (the) s
224 816 p (transformat) s
-1 r (ion) s
9 r (in) s
12 r (question.) s
18 r (Channels) s
11 r (in) s
12 r (PP) s
11 r (whic) s
104 c
10 r (fall) s
10 r (in) s
(to) s
10 r (this) s
11 r (category) s
10 r (are:) s
cmsy10.329 @sf
292 922 p 15 c
cmr10.329 @sf
23 r (Mapping) s
22 r (from) s
21 r 97 c
22 r (\015at) s
22 r (\(i.e.) s
41 r (ASN.1) s
22 r (enco) s
1 r (ded\)) s
24 r (P2) s
22 r (message) s
21 r (in) s
(to) s
20 r 97 c
23 r (PP) s
338 979 p (directory) s
14 r (hierarc) s
104 c
121 c
-4 r 46 c
cmsy10.329 @sf
292 1073 p 15 c
cmr10.329 @sf
23 r (The) s
15 r (rev) s
(erse) s
14 r (of) s
15 r (the) s
15 r (previous) s
15 r (mapping) s
cmsy10.329 @sf
292 1166 p 15 c
cmr10.329 @sf
23 r (Mapping) s
12 r (from) s
10 r 97 c
12 r (directory) s
12 r (hierarc) s
104 c
-1 r 121 c
11 r (in) s
(to) s
10 r (an) s
13 r (RF) s
67 c
11 r (822) s
11 r (message,) s
11 r (accord-) s
338 1223 p (ing) s
15 r (to) s
14 r (RF) s
67 c
14 r (934[19].) s
295 1329 p (Man) s
121 c
13 r (other) s
15 r (reformatti) s
-1 r (ng) s
13 r 99 c
(hannels) s
15 r (preserv) s
101 c
14 r (the) s
15 r (directory) s
15 r (hierarc) s
104 c
-1 r 121 c
-4 r 44 c
13 r (and) s
224 1386 p (simply) s
20 r (map) s
20 r (from) s
20 r (one) s
22 r 98 c
1 r 111 c
1 r (dy) s
22 r (part) s
21 r (syn) s
(tax) s
20 r (to) s
21 r (another.) s
38 r (These) s
22 r 99 c
(hannels) s
21 r (are) s
224 1442 p (supp) s
1 r (orted) s
14 r 98 c
121 c
11 r (use) s
13 r (of) s
13 r 97 c
12 r (generic) s
13 r (reformatti) s
-1 r (ng) s
11 r 99 c
(hannel) s
12 r (whic) s
104 c
12 r (will) s
11 r (driv) s
101 c
11 r 97 c
13 r (simple) s
224 1499 p (\014lter) s
19 r (to) s
19 r 112 c
1 r (erform) s
18 r (the) s
19 r (mapping.) s
31 r (Bo) s
1 r (dy) s
20 r (parts) s
18 r (whic) s
104 c
18 r (are) s
19 r (not) s
19 r (a\013ected) s
19 r 98 c
121 c
18 r (the) s
224 1555 p (\014lter) s
13 r (are) s
13 r (simply) s
12 r (link) s
(ed) s
12 r (across,) s
12 r (whic) s
104 c
13 r 97 c
118 c
-1 r (o) s
-1 r (ids) s
12 r (excess) s
13 r 98 c
(yte) s
13 r (cop) s
(ying.) s
17 r (It) s
14 r (is) s
13 r (hop) s
1 r (ed) s
224 1611 p (that) s
15 r (this) s
14 r (will) s
14 r (enable) s
16 r 97 c
14 r (wide) s
15 r (range) s
15 r (of) s
15 r 98 c
1 r 111 c
1 r (dy) s
16 r (part) s
15 r 116 c
(yp) s
1 r (es) s
14 r (to) s
14 r 98 c
1 r 101 c
16 r (supp) s
1 r (orted.) s
295 1668 p (Proto) s
1 r (col) s
20 r (Con) s
118 c
-1 r (ersion) s
20 r (is) s
21 r (an) s
22 r (imp) s
1 r (ortan) s
-1 r 116 c
19 r (form) s
20 r (of) s
22 r (reformatt) s
-1 r (ing.) s
38 r (In) s
22 r (terms) s
224 1724 p (of) s
17 r (the) s
18 r (PP) s
17 r (mo) s
1 r (del,) s
17 r (proto) s
1 r (col) s
17 r (con) s
118 c
-1 r (ersion) s
16 r 99 c
(hannels) s
16 r (are) s
18 r (no) s
17 r (di\013eren) s
116 c
16 r (from) s
16 r (other) s
224 1781 p (reformatti) s
-1 r (ng) s
21 r 99 c
(hannels.) s
41 r (The) s
23 r (initial) s
21 r 118 c
(ersion) s
20 r (of) s
23 r (PP) s
22 r (will) s
21 r (supp) s
1 r (ort) s
23 r (RF) s
67 c
21 r (987) s
224 1837 p (reformatti) s
-1 r (ng) s
16 r 98 c
1 r (et) s
119 c
(een) s
17 r (X.400) s
16 r (and) s
19 r (RF) s
67 c
16 r (822,) s
18 r (and) s
18 r (also) s
17 r (reformatti) s
-1 r (ng) s
16 r 98 c
1 r (et) s
119 c
(een) s
224 1894 p 118 c
-2 r (arian) s
-1 r (ts) s
18 r (of) s
20 r (RF) s
67 c
19 r (822.) s
35 r (Reformatti) s
-1 r (ng) s
18 r 98 c
1 r (et) s
119 c
(een) s
19 r (P22\(88\)) s
19 r (and) s
20 r (P2\(84\)) s
19 r (will) s
19 r 98 c
1 r 101 c
224 1950 p (added) s
16 r (later.) s
cmbx10.432 @sf
224 2093 p 54 c
69 r (Handling) s
23 r (Deli) s
-1 r 118 c
-2 r (ery) s
21 r (Noti\014cations) s
cmr10.329 @sf
224 2195 p (The) s
19 r (manner) s
17 r (in) s
19 r (whic) s
104 c
17 r (deliv) s
(ery) s
17 r (noti\014cations) s
17 r (are) s
18 r (handled) s
19 r (is) s
18 r (particularly) s
16 r (in-) s
224 2251 p (teresting.) s
27 r (Deliv) s
-1 r (ery) s
16 r (noti\014cations) s
16 r (are) s
18 r (stored) s
17 r (with) s
17 r (the) s
18 r (message,) s
16 r (as) s
18 r (describ) s
1 r (ed) s
224 2308 p (ab) s
1 r 111 c
118 c
(e.) s
33 r (The) s
21 r (queue) s
21 r (in) s
(terface) s
19 r (library) s
19 r (allo) s
(w) s
-1 r 115 c
19 r (an) s
121 c
19 r 99 c
(hannel) s
20 r (to) s
19 r (add) s
21 r 97 c
20 r (deliv) s
(ery) s
224 2364 p (noti\014cation.) s
29 r (Th) s
(us,) s
18 r (if) s
18 r 97 c
18 r (pro) s
1 r (cessing) s
18 r (error) s
18 r 111 c
1 r (ccurs,) s
20 r (the) s
18 r 99 c
(hannel) s
18 r (detecting) s
18 r (the) s
224 2421 p (error) s
14 r (will) s
14 r (up) s
1 r (date) s
15 r (the) s
15 r (queue) s
16 r (and) s
15 r (create) s
15 r (an) s
15 r (appropriate) s
14 r (deliv) s
(ery) s
13 r (noti\014cation.) s
224 2477 p 65 c
116 c
15 r 97 c
15 r (later) s
15 r (stage,) s
15 r (the) s
16 r (QMGR) s
16 r (can) s
16 r (in) s
118 c
(o) s
-1 r 107 c
-1 r 101 c
14 r (pro) s
1 r (cessing) s
16 r (of) s
16 r (deliv) s
(ery) s
14 r (noti\014cations) s
224 2534 p (through) s
15 r 97 c
15 r 99 c
(hannel) s
14 r (appropriate) s
14 r (for) s
15 r (the) s
15 r (message) s
14 r (originator.) s
295 2590 p (When) s
12 r (return) s
13 r (of) s
11 r (con) s
(ten) s
116 c
10 r (is) s
12 r (requested) s
13 r (as) s
12 r 97 c
12 r (service,) s
12 r (the) s
12 r (QMGR) s
13 r (ma) s
-1 r 121 c
10 r (ask) s
12 r (for) s
224 2646 p (deliv) s
(ery) s
14 r (ac) s
(kno) s
-1 r (wledgem) s
-1 r (en) s
-1 r 116 c
13 r (and) s
15 r (hold) s
15 r (the) s
15 r (message) s
14 r (lo) s
1 r (cally) s
-3 r 44 c
13 r (rather) s
14 r (than) s
15 r (asking) s
224 2703 p (for) s
19 r (return) s
19 r (of) s
18 r (con) s
(ten) s
116 c
17 r (as) s
18 r 97 c
19 r (remote) s
17 r (service.) s
32 r (When) s
19 r (the) s
19 r (appropriate) s
18 r (deliv) s
(ery) s
224 2759 p (noti\014cation) s
9 r (returns,) s
11 r (Submit) s
9 r (will) s
9 r (determine) s
9 r (\(from) s
8 r (the) s
10 r (QMGR\)) s
10 r (whic) s
104 c
9 r (message) s
960 2916 p 57 c
@eop
8 @bop0
cmr10.329 @sf
[<40E07030303818181878F8F8F8700000000000000000000070F8F8F870> 5 29 -4 9 13] 59 @dc
/cmbx10.360 @newfont
cmbx10.360 @sf
[<07FC001FFF803FFFC07E1FF07C07F0FE07F8FF03F8FF03FCFF03FCFF03FC7E03FC3C03FC0003FC0003FC0003F83803F83C07
  F03F0FE03FFFC039FE003800003800003800003800003FF8003FFE003FFF803FFFC03FFFE03FFFF03E01F0300030> 22 32 -3 0 29] 53 @dc
[<3C7EFFFFFFFF7E3C> 8 8 -4 0 16] 46 @dc
[<01FF000FFFE03FFFF87F07FC7F01FEFF81FEFF80FFFF80FFFF80FFFF80FF7F00FF3E00FF0000FE0001FE0001FC0007F000FF
  8000FF80000FE00007F00003F81F83F83FC3FC3FC1FC3FC1FC3FC1FC3FC1FC3F83F81F87F80FFFF007FFE001FF00> 24 32 -2 0 29] 51 @dc
[<FFFFF0FFFFF0FFFFF003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC
  0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC0003FC00FFFFF0FFFFF0FF
  FFF0> 20 34 -1 0 22] 73 @dc
[<FFF1FFE0FFF1FFE0FFF1FFE01F803F001F803F001F803F001F803F001F803F001F803F001F803F001F803F001F803F001F80
  3F001F803F001F803F001FC03F001FC03F001FE03F001F707F00FF3C7E00FF1FFC00FF07F800> 27 22 -3 0 32] 110 @dc
[<1C1FF0001E7FFC001FF07F001FE03F801FC01F801F801FC01F800FC01F800FE01F800FE01F800FE01F800FE01F800FE01F80
  0FE01F800FE01F800FE01F800FC01F800FC01F801F801FE03F801FF87F001FFFFC001F8FF0001F8000001F8000001F800000
  1F8000001F8000001F8000001F8000001F8000001F8000001F800000FF800000FF800000FF800000> 27 35 -2 0 32] 98 @dc
[<00FF0007FFE00FC3F01F00F83F00FC7E007E7E007EFE007FFE007FFE007FFE007FFE007FFE007FFE007F7E007E7E007E7E00
  7E3F00FC1F00F80FC3F007FFE000FF00> 24 22 -2 0 29] 111 @dc
[<01FE3FE007FFBFE00FC3FFE01F80FF001F807F001F807F001F803F001F803F001F803F001F803F001F803F001F803F001F80
  3F001F803F001F803F001F803F001F803F001F803F001F803F00FF81FF00FF81FF00FF81FF00> 27 22 -3 0 32] 117 @dc
[<01FE3FE007FFBFE01FC3FFE03F80FF003F007F007E003F007E003F00FE003F00FE003F00FE003F00FE003F00FE003F00FE00
  3F00FE003F00FE003F007E003F007F003F003F003F003F807F001FC1FF0007FFFF0001FF3F0000003F0000003F0000003F00
  00003F0000003F0000003F0000003F0000003F0000003F0000003F000001FF000001FF000001FF00> 27 35 -2 0 32] 100 @dc
[<0003FF0000003FFFE00000FFFFF80001FF80FC0007FC003E000FF8000F001FE00007801FC00003803FC00003C07F800001C0
  7F800001C07F800001C0FF00000000FF00000000FF00000000FF00000000FF00000000FF00000000FF00000000FF00000000
  FF00000000FF000001C07F800001C07F800003C07F800003C03FC00003C01FC00007C01FE0000FC00FF8001FC007FC003FC0
  01FF80FFC000FFFFF7C0003FFFE1C00003FF00C0> 34 34 -3 0 41] 67 @dc
[<FFF1FFE0FFF1FFE0FFF1FFE01F803F001F803F001F803F001F803F001F803F001F803F001F803F001F803F001F803F001F80
  3F001F803F001F803F001FC03F001FC03F001FE03F001FF07F001FBC7E001F9FFC001F87F8001F8000001F8000001F800000
  1F8000001F8000001F8000001F8000001F8000001F8000001F800000FF800000FF800000FF800000> 27 35 -3 0 32] 104 @dc
[<0FF03F803FFCFF807F0FFF80FE07F800FC03F800FC01F800FC01F800FE01F8007E01F8007F81F8001FE1F8000FFFF80000FF
  F8000001F8003E01F8007F01F8007F01F8007F03F8007F03F0007F0FE0003FFFC0000FFE0000> 25 22 -2 0 28] 97 @dc
[<00FF0007FFE00FC1F01F80703F00387F00387E0000FE0000FE0000FE0000FE0000FFFFF8FFFFF8FE00F8FE00F87E01F87E01
  F03F01F01F03E00FC7E007FF8001FE00> 21 22 -2 0 26] 101 @dc
[<FFF0FFF0FFF01F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F80
  1F801F801F801F801F801F801F80FF80FF80FF80> 12 35 -2 0 15] 108 @dc
[<C7FC00FFFF00FE0F80F80780F003C0F003C0E007C0E00FC0003FC007FFC01FFF807FFF007FFE00FFF800FF8000F80000F003
  80F003807807807C0F803FFF800FFB80> 18 22 -2 0 23] 115 @dc
[<01FFFF01FFFF01FFFF0007F00007F00007F00007F00007F00007F0FFFFFFFFFFFFFFFFFFF003F07003F03803F01C03F00E03
  F00703F00783F00383F001C3F000E3F00073F0003BF0001FF0001FF0000FF00007F00003F00001F00000F00000F0> 24 32 -2 0 29] 52 @dc
[<FFF80700FFFF80FFF80F80FFFF80FFF80F80FFFF8007000F8007F00007001FC007F00007001FC007F00007003FE007F00007
  003FE007F00007007F7007F00007007F7007F00007007F7007F0000700FE3807F0000700FE3807F0000701FC1C07F0000701
  FC1C07F0000703F80E07F0000703F80E07F0000703F80E07F0000707F00707F0000707F00707F000070FE00387F000070FE0
  0387F000071FC001C7F000071FC001C7F000071FC001C7F000073F8000E7F000073F8000E7F000077F000077F000077F0000
  77F00007FE00003FF00007FE00003FF000FFFE00003FFF80FFFC00001FFF80FFFC00001FFF80> 49 34 -2 0 54] 77 @dc
[<01FFE0000FFFFC003F807F007E001F80FC000FC0F80007C0F80007C0F80007C0FC000FC07E003FC03FFFFF800FFFFF800FFF
  FF001FFFFE001FFFF0001E0000001C0000001C0000001DFF00000FFFC0000FC7E0001F83F0003F01F8003F01F8003F01F800
  3F01F8003F01F8003F01F9003F01FFC01F83F7C00FC7F9C007FFFFC001FF1F80> 26 33 -1 11 29] 103 @dc
[<FFFFC00FF8FFFFC07FFEFFFFC0FF8E07F800FF0707F801FE0707F801FE0707F801FE0007F801FE0007F801FE0007F801FE00
  07F801FE0007F801FE0007F801FE0007F803FC0007F807FC0007F80FF80007FFFFE00007FFFFE00007F807F80007F801FE00
  07F800FF0007F8007F0007F8007F8007F8007F8007F8007F8007F8007F8007F8007F8007F8007F0007F800FF0007F800FE00
  07F803FC00FFFFFFF800FFFFFFE000FFFFFE0000> 40 34 -2 0 43] 82 @dc
[<7FFC007FFC007FFC000FC0000FC0000FC0000FC0000FC0000FC0000FC0000FC0000FC0000FC0000FC0000FC0000FC0000FC0
  000FC0000FC000FFFE00FFFE00FFFE000FC0000FC0000FC0000FC0000FC0000FC1F00FC3F80FC3F807E3F807F3F803F9F800
  FFF0001FE0> 21 35 -2 0 18] 102 @dc
[<FFF800FFF800FFF8001F80001F80001F80001F80001F80001F80001F80001F80001F80001F80001F80001F83E01F87F01FC7
  F01FC7F01FE7F0FF77F0FF7FE0FF1F80> 20 22 -2 0 24] 114 @dc
[<FFF0FFF0FFF0FFF0FFF0FFF0FFF0FFF0FFF01F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F80
  1F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801FC01FC01F801FC01FC0
  1F801FE01FE01F801F703FF03F80FF3C7F3C7F00FF1FFE1FFE00FF07FC07FC00> 44 22 -3 0 51] 109 @dc
[<01FC0003FE0007E7000FC7800FC3800FC3800FC3800FC3800FC3800FC0000FC0000FC0000FC0000FC0000FC0000FC0000FC0
  000FC0000FC000FFFF00FFFF003FFF000FC00007C00007C00003C00003C00003C00001C00001C00001C00001C000> 17 32 -1 0 22] 116 @dc
[<FFF0FFF0FFF01F801F801F801F801F801F801F801F801F801F801F801F801F801F801F801F80FF80FF80FF80000000000000
  0000000000001E003F807F807F807F807F803F801E00> 12 36 -2 0 15] 105 @dc
[<FFFFC00000FFFFC00000FFFFC0000007F800000007F800000007F800000007F800000007F800000007F800000007F8000000
  07F800000007F800000007F800000007F800000007F800000007FFFFC00007FFFFF80007F803FC0007F800FE0007F800FF00
  07F8007F0007F8007F8007F8007F8007F8007F8007F8007F8007F8007F8007F8007F8007F8007F0007F800FF0007F800FE00
  07F803FC00FFFFFFF800FFFFFFE000FFFFFF8000> 33 34 -2 0 39] 80 @dc
[<01FF0007FFC01FE1E03F80F03F80707F00007F0000FE0000FE0000FE0000FE0000FE0000FE0000FE0000FE03E07E07F07F07
  F03F07F03F87F01FC7F007FFE001FF80> 20 22 -2 0 25] 99 @dc
[<000E0000001F0000001F0000003F8000003F8000007FC000007FC000007FC00000FCE00000FCE00001FCF00001F8700003F8
  780003F0380003F0380007E01C0007E01C000FE01E000FC00E00FFF03FE0FFF03FE0FFF03FE0> 27 22 -1 0 30] 118 @dc
8 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
224 195 p 112 c
1 r (ermission) s
12 r (to) s
13 r (send) s
14 r (to) s
13 r (the) s
13 r (list\),) s
12 r (and) s
14 r (resubmit) s
12 r (the) s
13 r (message) s
12 r (as) s
13 r 97 c
14 r (new) s
13 r (message.) s
224 252 p (This) s
13 r (means) s
13 r (that) s
13 r (lists) s
12 r (will) s
12 r 98 c
1 r 101 c
14 r (pro) s
(vided) s
13 r (at) s
12 r (the) s
14 r (P2) s
13 r (lev) s
(el,) s
12 r (rather) s
13 r (than) s
13 r (at) s
13 r (the) s
14 r (P1) s
224 308 p (lev) s
(el.) s
18 r 70 c
-3 r (or) s
13 r (PP) s
-3 r 44 c
14 r (the) s
14 r (ma) s
3 r (jor) s
13 r (consideration) s
13 r (is) s
14 r (the) s
15 r (abilit) s
-1 r 121 c
13 r (to) s
14 r (supp) s
1 r (ort) s
15 r (distribution) s
224 364 p (lists) s
15 r (for) s
16 r (RF) s
67 c
16 r (822) s
15 r (and) s
17 r (X.400\(1984\)) s
14 r (systems.) s
23 r (There) s
16 r (are) s
17 r (man) s
-1 r 121 c
15 r (reasons) s
16 r (as) s
16 r (to) s
224 421 p (wh) s
121 c
12 r (this) s
13 r (is) s
13 r (preferable,) s
14 r (and) s
13 r (the) s
14 r (author) s
13 r (is) s
13 r (disapp) s
1 r (oin) s
(ted) s
12 r (in) s
13 r (the) s
14 r 99 c
(hoice) s
12 r (made) s
13 r 98 c
121 c
224 477 p (CCITT/ISO.) s
16 r (The) s
16 r (X.400\(88\)) s
13 r (list) s
15 r (mec) s
(hanism) s
-1 r 115 c
13 r (will) s
15 r 98 c
1 r 101 c
16 r (supp) s
1 r (orted) s
17 r (in) s
15 r (as) s
16 r (far) s
15 r (as) s
224 534 p (PP) s
14 r (will) s
13 r 98 c
1 r (eha) s
118 c
101 c
13 r (correctly) s
13 r (in) s
15 r (relation) s
12 r (to) s
14 r (other) s
14 r (systems) s
13 r (follo) s
-1 r (wing) s
12 r (the) s
14 r (sp) s
1 r (eci\014ed) s
224 590 p (pro) s
1 r (cedures.) s
22 r (PP) s
16 r (will) s
14 r (also) s
14 r (align) s
15 r (to) s
15 r (the) s
16 r (X.400\(88\)) s
13 r (list) s
14 r (informatio) s
-1 r 110 c
14 r (structures,) s
224 647 p (when) s
16 r (using) s
15 r (the) s
15 r (OSI) s
16 r (Directory) s
295 703 p (There) s
15 r (is) s
15 r (also) s
15 r (an) s
15 r (outb) s
1 r (ound) s
16 r 99 c
(hannel) s
15 r (to) s
14 r (supp) s
1 r (ort) s
16 r (the) s
15 r (RF) s
67 c
14 r (987) s
15 r (mapping) s
14 r (of) s
224 760 p (deliv) s
(ery) s
16 r (noti\014cations) s
16 r (on) s
(to) s
16 r (text) s
17 r (messages.) s
25 r (PP) s
17 r (will) s
16 r (map) s
17 r (all) s
16 r (errors) s
17 r (on) s
(to) s
15 r (De-) s
224 816 p (liv) s
(ery) s
13 r (Noti\014cations;) s
13 r (Therefore) s
15 r (this) s
14 r 99 c
(hannel) s
14 r (is) s
14 r 97 c
15 r (critical) s
13 r (comp) s
1 r (onen) s
(t,) s
12 r 98 c
1 r (ecause) s
224 873 p (RF) s
67 c
17 r (822) s
17 r (based) s
18 r (systems) s
17 r (do) s
18 r (not) s
17 r (supp) s
1 r (ort) s
19 r (Deliv) s
-1 r (ery) s
16 r (Noti\014cations) s
17 r (as) s
18 r (proto) s
1 r (col) s
224 929 p (elemen) s
(ts.) s
295 985 p (Use) s
19 r (is) s
20 r (made) s
18 r (of) s
19 r (appropriate) s
19 r (Nameserv) s
-1 r (ers) s
18 r (and) s
20 r (Directories.) s
31 r (The) s
20 r (SMTP) s
224 1042 p 99 c
(hannel) s
19 r (uses) s
20 r (the) s
19 r (BIND) s
20 r (Nameserv) s
(er[) s
-1 r (17]) s
-1 r 46 c
31 r (The) s
20 r (X.400\(88\)) s
17 r 99 c
(hannels) s
19 r (and) s
19 r (the) s
224 1098 p (list) s
13 r 99 c
(hannel) s
14 r (utilise) s
13 r (X.500) s
13 r (Directory) s
13 r (Services) s
15 r 98 c
121 c
13 r (means) s
13 r (of) s
14 r (the) s
15 r (QUIPU) s
15 r (imple-) s
224 1155 p (men) s
(tat) s
-1 r (ion) s
13 r ([18].) s
cmbx10.360 @sf
224 1277 p (5.3) s
57 r (In) s
-1 r 98 c
2 r (ou) s
-1 r (nd) s
17 r (Channels) s
cmr10.329 @sf
224 1362 p (In) s
98 c
1 r (ound) s
17 r 99 c
(hannels) s
16 r 112 c
1 r (erform) s
15 r (the) s
17 r (opp) s
1 r (osite) s
16 r (to) s
16 r (outb) s
1 r (ound) s
17 r 99 c
(hannels.) s
23 r 70 c
-3 r (or) s
15 r (eac) s
104 c
16 r (of) s
224 1419 p (the) s
20 r (proto) s
1 r (cols) s
18 r (supp) s
1 r (orted,) s
21 r (there) s
19 r (is) s
19 r 97 c
20 r 99 c
(hannel) s
18 r (whic) s
104 c
19 r (will) s
18 r (act) s
19 r (as) s
19 r 97 c
19 r (serv) s
(er) s
18 r (for) s
224 1475 p (the) s
14 r (proto) s
1 r (col.) s
18 r (When) s
15 r (an) s
13 r (in) s
98 c
1 r (ound) s
14 r (message) s
12 r (is) s
13 r (receiv) s
(ed,) s
13 r (the) s
14 r 99 c
(hannel) s
13 r (will) s
13 r (in) s
118 c
-1 r (ok) s
-2 r 101 c
224 1532 p (Submit.) s
19 r (The) s
15 r 99 c
(hannel) s
15 r (will) s
14 r (then) s
15 r (pass) s
15 r (messages) s
14 r (in) s
(to) s
13 r (Submit.) s
295 1588 p (It) s
17 r (is) s
17 r (also) s
17 r 112 c
1 r (ossible) s
17 r (for) s
17 r 97 c
18 r 99 c
(hannel) s
16 r (to) s
17 r (act) s
17 r (as) s
18 r 98 c
1 r (oth) s
17 r (in) s
98 c
1 r (ound) s
17 r (and) s
18 r (outb) s
1 r (ound.) s
224 1645 p (This) s
20 r (is) s
19 r (necessary) s
21 r (to) s
19 r (supp) s
1 r (ort) s
20 r (Tw) s
111 c
18 r 87 c
-3 r 97 c
121 c
18 r (Alternate) s
19 r (mo) s
1 r (de) s
19 r (Reliable) s
20 r 84 c
-3 r (ransfer) s
224 1701 p (for) s
15 r (X.400,) s
13 r (or) s
15 r (to) s
14 r (supp) s
1 r (ort) s
15 r (P3/P7) s
14 r (st) s
(yle) s
14 r (message) s
13 r (retriev) s
-2 r (al) s
13 r (whic) s
104 c
14 r (is) s
15 r (User) s
15 r (Agen) s
116 c
224 1758 p (initiated.) s
17 r (In) s
12 r (this) s
10 r (case,) s
12 r (the) s
11 r 99 c
(hannel) s
10 r (will) s
9 r (start) s
10 r (as) s
11 r (an) s
11 r (in) s
98 c
1 r (ound) s
10 r 99 c
(hannel) s
10 r (\(t) s
(ypically) s
224 1814 p 98 c
121 c
15 r 98 c
1 r (eing) s
16 r (in) s
118 c
-1 r (ok) s
-1 r (ed) s
14 r (as) s
15 r 97 c
15 r (proto) s
1 r (col) s
15 r (serv) s
(er\).) s
20 r (Then) s
16 r (it) s
15 r (will) s
14 r (connect) s
17 r (to) s
15 r (the) s
15 r (QMGR) s
224 1870 p (using) s
14 r 82 c
(OS.) s
14 r (The) s
15 r (QMGR) s
15 r (can) s
14 r 99 c
(ho) s
1 r (ose) s
14 r (to) s
14 r (accept) s
14 r (or) s
14 r (reject) s
15 r (the) s
14 r (asso) s
1 r (ciation.) s
18 r (If) s
15 r (it) s
224 1927 p (accepts) s
17 r (the) s
16 r (asso) s
1 r (ciation,) s
15 r (the) s
17 r (QMGR) s
17 r (will) s
15 r (tak) s
101 c
15 r (con) s
(trol) s
14 r (and) s
17 r (then) s
17 r (the) s
16 r 99 c
(hannel) s
224 1983 p (then) s
18 r 98 c
1 r (eha) s
118 c
(es) s
15 r (as) s
17 r (an) s
18 r (outb) s
1 r (ound) s
18 r 99 c
(hannel.) s
25 r (In) s
18 r (terms) s
16 r (of) s
17 r (the) s
18 r (QMGR,) s
17 r (the) s
17 r (use) s
18 r (of) s
224 2040 p 82 c
(OS) s
15 r (mak) s
-1 r (es) s
14 r (this) s
14 r (con) s
(trol) s
13 r (mo) s
1 r (del) s
14 r (straigh) s
(tf) s
-1 r (orw) s
-2 r (ard) s
13 r (to) s
15 r (pro) s
(vide.) s
cmbx10.360 @sf
224 2161 p (5.4) s
57 r (Message) s
18 r (Reform) s
-2 r (atting) s
18 r (and) s
18 r (Proto) s
2 r (col) s
19 r (Con) s
-1 r 118 c
-1 r (ersion) s
cmr10.329 @sf
224 2247 p (The) s
17 r (\015exible) s
16 r (handling) s
16 r (of) s
16 r (reformatt) s
-1 r (ing) s
14 r (is) s
16 r (one) s
17 r (of) s
16 r (the) s
16 r (most) s
14 r (imp) s
1 r (ortan) s
-1 r 116 c
14 r (comp) s
1 r (o-) s
224 2304 p (nen) s
(ts) s
13 r (of) s
14 r (PP) s
-3 r 46 c
13 r (This) s
13 r (is) s
14 r (ac) s
(hiev) s
-1 r (ed) s
13 r 98 c
121 c
13 r (reformatt) s
-1 r (ing) s
12 r 99 c
(hannels) s
13 r (whic) s
104 c
13 r (map) s
13 r (from) s
12 r (one) s
224 2360 p (message) s
15 r (form) s
14 r (in) s
(to) s
15 r (another.) s
22 r (As) s
16 r (discussed,) s
17 r (Submit) s
15 r (will) s
15 r (calculate) s
15 r 97 c
16 r (sequence) s
224 2417 p (of) s
17 r (reformat) s
-1 r (ting) s
15 r 99 c
(hannels) s
16 r (for) s
16 r (eac) s
104 c
16 r (recipien) s
(t,) s
16 r (in) s
17 r (order) s
17 r (to) s
16 r (pro) s
(vide) s
16 r (the) s
17 r (correct) s
224 2473 p (con) s
118 c
(ersi) s
-1 r (ons.) s
26 r (In) s
(ternally) s
-3 r 44 c
16 r (PP) s
18 r (represen) s
(ts) s
17 r (di\013eren) s
116 c
16 r 98 c
1 r 111 c
1 r (dy) s
19 r (part) s
17 r (and) s
19 r (header) s
18 r (for-) s
224 2530 p (mats) s
13 r (as) s
15 r (strings) s
13 r (\(e.g.) s
19 r (\\IA5") s
15 r (or) s
14 r (\\JNT-Mail"\).) s
18 r (Channels) s
15 r (will) s
14 r (supp) s
1 r (ort) s
15 r 97 c
14 r (set) s
15 r (of) s
224 2586 p (formats,) s
10 r (represen) s
(ted) s
12 r 98 c
121 c
11 r (these) s
13 r (strings.) s
18 r (The) s
12 r 99 c
(hannel) s
12 r (ma) s
-1 r 121 c
10 r 99 c
(ho) s
1 r (ose) s
11 r (to) s
12 r (map) s
11 r (these) s
224 2642 p (in) s
(to) s
12 r (proto) s
1 r (col) s
12 r (sp) s
1 r (eci\014c) s
14 r (enco) s
1 r (dings) s
14 r (\(e.g.) s
19 r (P1) s
13 r (Enco) s
1 r (ded) s
14 r (Information) s
12 r 84 c
(yp) s
1 r (es\).) s
18 r (This) s
224 2699 p (approac) s
104 c
19 r (lets) s
20 r (complex) s
19 r (mappings) s
19 r (to) s
20 r 98 c
1 r 101 c
21 r (calculated,) s
20 r (and) s
21 r (allo) s
-1 r (ws) s
18 r (for) s
20 r (general) s
224 2755 p (extension.) s
960 2916 p 56 c
@eop
7 @bop0
cmbx10.432 @sf
[<00FF800007FFF0000FFFFC003F83FE003C00FF007C007F80FF007FC0FF003FC0FF803FE0FF803FE0FF803FE0FF003FE07F00
  3FE03C003FE000003FE000003FE000003FC000003FC01E007F801F007F801FC1FF001FFFFE001FFFF8001E7FC0001E000000
  1E0000001E0000001E0000001E0000001FFE00001FFF80001FFFE0001FFFF0001FFFFC001FFFFC001FFFFE001FFFFF001F80
  3F001C000700> 27 39 -3 0 34] 53 @dc
cmti10.329 @sf
[<0FE0003FFC003C3E00780700700300F00000F00000F00000F00000F800007800007800007800003C02003C07801E07800F07
  8007C38003FF00007E00> 17 20 -4 0 21] 99 @dc
cmbx10.360 @sf
[<7FFFF07FFFF07FFFF001FC0001FC0001FC0001FC0001FC0001FC0001FC0001FC0001FC0001FC0001FC0001FC0001FC0001FC
  0001FC0001FC0001FC0001FC0001FC0001FC0001FC0001FC0001FC00FFFC00FFFC00FFFC0001FC00007C00001C00> 20 32 -4 0 29] 49 @dc
[<0007FE0000003FFFC00000FE07F00003F801FC0007F000FE000FE0007F001FC0003F801FC0003F803F80001FC07F80001FE0
  7F80001FE07F00000FE0FF00000FF0FF00000FF0FF00000FF0FF00000FF0FF00000FF0FF00000FF0FF00000FF0FF00000FF0
  FF00000FF07F00000FE07F00000FE07F00000FE03F80001FC03F80001FC01F80001F801FC0003F800FE0007F0007F000FE00
  03F801FC0000FE07F000003FFFC0000007FE0000> 36 34 -3 0 43] 79 @dc
[<FFFFF0FFFFF0FFFFF0FFFFF87FFFF83FFFF81FFFF80F003807803803C01C01E01C00F81C007C00003E00001F80001FC0000F
  E00007F00007F80003F80003FC3C03FC7E01FCFF01FCFF03FCFF03FCFF07F8FF0FF87E3FF03FFFE01FFFC003FE00> 22 32 -3 0 29] 50 @dc
7 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
cmbx10.432 @sf
224 195 p 53 c
69 r (Channels) s
cmr10.329 @sf
224 297 p (The) s
14 r (term) s
cmti10.329 @sf
13 r (channel) s
cmr10.329 @sf
13 r (is) s
13 r (used) s
15 r (in) s
13 r (PP) s
14 r (to) s
13 r (describ) s
1 r 101 c
15 r 97 c
13 r 110 c
(um) s
-1 r 98 c
1 r (er) s
13 r (of) s
13 r (somewhat) s
12 r (di\013eren) s
116 c
224 353 p (comp) s
1 r (onen) s
(ts.) s
26 r (In) s
19 r (essence,) s
19 r (eac) s
104 c
17 r 99 c
(hannel) s
17 r (tak) s
(es) s
16 r 97 c
18 r (message) s
16 r (as) s
18 r (input,) s
18 r (and) s
18 r (then) s
224 409 p (pro) s
(vides) s
10 r 97 c
11 r (di\013eren) s
116 c
9 r (message) s
10 r (as) s
11 r (output.) s
19 r (Channels) s
11 r (whic) s
104 c
10 r (are) s
11 r (con) s
(troll) s
-1 r (ed) s
10 r 98 c
121 c
10 r (the) s
224 466 p (QMGR,) s
12 r (access) s
13 r (the) s
13 r (queue) s
13 r (through) s
13 r 97 c
12 r (library) s
-3 r 46 c
17 r (They) s
13 r (are) s
12 r (trusted) s
12 r (pro) s
1 r (cesses,) s
13 r (and) s
224 522 p (will) s
11 r (up) s
1 r (date) s
13 r (the) s
13 r (queue.) s
20 r 65 c
12 r 99 c
(hannel) s
12 r (will) s
11 r (op) s
1 r (erate) s
12 r (en) s
(tirely) s
10 r (under) s
14 r (the) s
12 r (direction) s
12 r (of) s
224 579 p (the) s
15 r (QMGR,) s
14 r (and) s
15 r (no) s
14 r (in) s
(telligence) s
13 r (is) s
14 r (in) s
118 c
-1 r (olv) s
-2 r (ed.) s
19 r (The) s
14 r 118 c
-2 r (arious) s
13 r (\015a) s
118 c
(ours) s
12 r (of) s
14 r 99 c
(hannel) s
224 635 p (are) s
15 r (no) s
119 c
14 r (describ) s
1 r (ed.) s
cmbx10.360 @sf
224 757 p (5.1) s
57 r (Outb) s
2 r (ound) s
17 r (Proto) s
2 r (col) s
19 r (Channels) s
cmr10.329 @sf
224 843 p (This) s
17 r (basic) s
17 r 116 c
(yp) s
1 r 101 c
16 r (of) s
17 r 99 c
(hannel) s
16 r (will) s
16 r (tak) s
101 c
16 r 97 c
17 r (message) s
16 r (from) s
15 r (the) s
17 r (queue,) s
19 r (and) s
17 r (trans-) s
224 899 p (fer) s
17 r (it) s
16 r (using) s
16 r 97 c
17 r (Message) s
16 r 84 c
-3 r (ransfer) s
16 r (Proto) s
1 r (col.) s
23 r (The) s
17 r (outb) s
1 r (ound) s
18 r (proto) s
1 r (col) s
15 r 99 c
(hannels) s
224 955 p (supp) s
1 r (orted) s
16 r 98 c
121 c
14 r (PP) s
15 r (initially) s
13 r (will) s
14 r 98 c
1 r (e:) s
cmsy10.329 @sf
292 1060 p 15 c
cmr10.329 @sf
23 r (X.400/P1) s
13 r (\(1984\)) s
cmsy10.329 @sf
292 1153 p 15 c
cmr10.329 @sf
23 r (X.400/P1) s
13 r (\(1988\)) s
cmsy10.329 @sf
292 1246 p 15 c
cmr10.329 @sf
23 r (JNT) s
15 r (Mail) s
cmsy10.329 @sf
292 1340 p 15 c
cmr10.329 @sf
23 r (SMTP) s
cmsy10.329 @sf
292 1433 p 15 c
cmr10.329 @sf
23 r (UUCP) s
295 1537 p (In) s
(terestingly) s
-4 r 44 c
17 r (the) s
19 r (implem) s
-1 r (en) s
(t) s
-1 r (ati) s
-1 r (on) s
17 r (of) s
18 r (the) s
19 r (X.400) s
17 r (proto) s
1 r (col) s
18 r (is) s
18 r (not) s
18 r (really) s
18 r 97 c
224 1594 p (ma) s
3 r (jor) s
17 r (problem.) s
31 r (The) s
19 r (use) s
20 r (of) s
19 r (the) s
19 r (PEPY) s
20 r (ASN.1) s
19 r (Compiler) s
17 r (from) s
17 r (the) s
20 r (ISODE) s
224 1650 p (pac) s
107 c
-2 r (age) s
16 r (has) s
18 r (made) s
17 r (the) s
18 r (mapping) s
17 r (of) s
18 r (the) s
18 r (PP) s
18 r (Message) s
17 r 84 c
-3 r (ransfer) s
17 r (Services) s
18 r (on) s
(to) s
224 1707 p (OSI) s
16 r 97 c
15 r (relativ) s
(el) s
-1 r 121 c
14 r (mec) s
(hani) s
-1 r (cal) s
13 r (op) s
1 r (eration[1],) s
13 r ([16].) s
cmbx10.360 @sf
224 1828 p (5.2) s
57 r (Other) s
19 r (Outb) s
2 r (ound) s
17 r (Channels) s
cmr10.329 @sf
224 1914 p (An) s
18 r (outb) s
1 r (ound) s
18 r 99 c
(hannel) s
16 r (is) s
17 r (alw) s
97 c
-2 r (ys) s
15 r (the) s
18 r (last) s
16 r (applied) s
17 r (to) s
17 r 97 c
17 r (recipien) s
(t,) s
15 r (and) s
18 r (so) s
17 r (will) s
224 1970 p (cause) s
20 r (the) s
20 r (message) s
19 r (to) s
19 r (lea) s
118 c
-1 r 101 c
18 r (the) s
20 r (queue) s
20 r (\(for) s
19 r (that) s
19 r (recipien) s
(t\).) s
32 r (This) s
20 r (do) s
1 r (es) s
20 r (not) s
224 2027 p (include) s
17 r (handling) s
17 r (of) s
16 r (deliv) s
(ery) s
15 r (noti\014cations,) s
15 r (whic) s
104 c
16 r (will) s
15 r (happ) s
1 r (en) s
18 r (after) s
16 r (the) s
17 r (out-) s
224 2083 p 98 c
1 r (ound) s
21 r 99 c
(hannel) s
19 r (is) s
20 r (run.) s
34 r (There) s
20 r (are) s
20 r 97 c
19 r 110 c
(um) s
-1 r 98 c
1 r (er) s
19 r (of) s
19 r (outb) s
1 r (ound) s
21 r 99 c
(hannels,) s
20 r (whic) s
104 c
224 2140 p (are) s
17 r (not) s
16 r (proto) s
1 r (col) s
16 r 99 c
(hannels.) s
24 r (The) s
17 r (\014rst) s
17 r (is) s
16 r (lo) s
1 r (cal) s
16 r (deliv) s
(ery) s
-3 r 44 c
14 r (where) s
18 r (the) s
17 r (message) s
15 r (is) s
224 2196 p (simply) s
14 r (deliv) s
(ered) s
14 r (in) s
(to) s
13 r 97 c
15 r (lo) s
1 r (cal) s
14 r (\014le.) s
20 r (PP) s
15 r (supp) s
1 r (orts) s
15 r 116 c
119 c
-1 r 111 c
13 r (st) s
(yles) s
13 r (of) s
15 r (lo) s
1 r (cal) s
14 r (deliv) s
(ery:) s
280 2301 p (1.) s
22 r (Deliv) s
(ery) s
13 r (in) s
(to) s
13 r 97 c
15 r (UNIX/MMDF) s
14 r (st) s
(yle) s
13 r (\014le) s
16 r (with) s
14 r (CTRL-A) s
16 r (separators.) s
280 2394 p (2.) s
22 r (Deliv) s
(ery) s
19 r (in) s
(to) s
20 r 97 c
21 r (directory) s
20 r (structured) s
22 r (message,) s
21 r (equiv) s
-2 r (alen) s
116 c
19 r (to) s
21 r (the) s
22 r (PP) s
338 2450 p (queue) s
16 r (mo) s
1 r (del.) s
295 2555 p (Lo) s
1 r (cal) s
13 r (deliv) s
(ery) s
13 r (will) s
12 r (implemen) s
-1 r 116 c
11 r (the) s
14 r (deliv) s
(ery) s
12 r (time) s
12 r (pro) s
1 r (cessing) s
14 r (supp) s
1 r (orted) s
15 r 98 c
121 c
224 2611 p (MMDF[5].) s
295 2668 p (An) s
22 r (outb) s
1 r (ound) s
22 r 99 c
(hannel) s
20 r (is) s
21 r (also) s
21 r (used) s
22 r (to) s
21 r (supp) s
1 r (ort) s
21 r (distribution) s
21 r (lists.) s
37 r (The) s
224 2724 p (destination,) s
21 r (in) s
20 r (terms) s
20 r (of) s
20 r (the) s
21 r (incoming) s
19 r (message,) s
20 r (will) s
20 r 98 c
1 r 101 c
21 r (the) s
21 r (list.) s
36 r (The) s
21 r (list) s
224 2781 p 99 c
(hannel) s
11 r (will) s
11 r (expand) s
12 r (the) s
12 r (list,) s
11 r 112 c
1 r (erform) s
10 r (an) s
121 c
11 r (necessary) s
12 r (managem) s
-1 r (en) s
-1 r 116 c
10 r 99 c
(hec) s
(ks) s
10 r (\(e.g.) s
960 2916 p 55 c
@eop
6 @bop0
6 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
280 195 p (1.) s
22 r (An) s
121 c
20 r (errors) s
21 r (will) s
20 r 98 c
1 r 101 c
22 r (passed) s
22 r (bac) s
107 c
20 r (to) s
21 r (the) s
22 r (submitting) s
19 r (pro) s
1 r (cess.) s
39 r (This) s
21 r (is) s
338 252 p (useful) s
15 r (for) s
15 r (User) s
15 r (Agen) s
(ts,) s
13 r (and) s
16 r (in) s
(teracti) s
-1 r 118 c
-1 r 101 c
13 r (proto) s
1 r (cols) s
14 r (suc) s
104 c
15 r (as) s
15 r (SMTP[10].) s
280 345 p (2.) s
22 r 65 c
13 r (successful) s
14 r (resp) s
1 r (onse) s
13 r (is) s
13 r (alw) s
97 c
-2 r (ys) s
11 r (passed) s
14 r (bac) s
107 c
12 r (to) s
12 r (the) s
14 r (submitti) s
-1 r (ng) s
12 r (pro) s
1 r (cess,) s
338 402 p (and) s
19 r (deliv) s
(ery) s
18 r (noti\014cations) s
18 r (are) s
19 r (generated) s
19 r (if) s
19 r (necessary) s
19 r (\(these) s
19 r (reside) s
20 r (in) s
338 458 p (the) s
13 r (queue) s
13 r (for) s
13 r (later) s
11 r (pro) s
1 r (cessing\).) s
19 r (This) s
13 r (mec) s
(hani) s
-1 r (sm) s
10 r (is) s
12 r (useful) s
13 r (for) s
13 r (proto) s
1 r (cols) s
338 515 p (suc) s
104 c
14 r (as) s
15 r (X.400) s
14 r (or) s
15 r (JNT) s
15 r (Mail) s
14 r ([11].) s
295 621 p (Address) s
21 r 99 c
(hec) s
(king) s
18 r (will) s
19 r (\014rst) s
20 r (determine) s
20 r (the) s
21 r (MT) s
-3 r 65 c
19 r (to) s
20 r (whic) s
104 c
19 r (the) s
21 r (message) s
224 677 p (should) s
23 r 98 c
1 r 101 c
24 r (sen) s
(t.) s
41 r (Curren) s
(tly) s
-3 r 44 c
22 r (PP) s
23 r (uses) s
23 r (table) s
22 r (based) s
23 r (address) s
23 r (handling,) s
24 r (with) s
224 734 p (access) s
20 r (optimi) s
-1 r (sed) s
18 r 98 c
121 c
18 r (the) s
19 r (use) s
20 r (of) s
19 r (hashed) s
19 r (lo) s
1 r (okup.) s
32 r (In) s
(ternally) s
-3 r 44 c
17 r (MT) s
-3 r (As) s
18 r (are) s
19 r (rep-) s
224 790 p (resen) s
(ted) s
15 r 98 c
121 c
14 r (domain) s
14 r (names) s
14 r (\(e.g.) s
20 r (CS.UCL.A) s
(C.) s
-1 r (UK\).) s
13 r 70 c
-3 r (or) s
14 r (RF) s
67 c
14 r (822) s
15 r (addresses,) s
224 847 p (this) s
20 r (utilises) s
20 r (the) s
21 r (MMDF) s
19 r (approac) s
104 c
19 r (whic) s
104 c
20 r (is) s
20 r (describ) s
1 r (ed) s
22 r (elsewhere[12].) s
35 r (This) s
224 903 p (pap) s
1 r (er) s
19 r (also) s
16 r (giv) s
(es) s
17 r 97 c
17 r (more) s
17 r (detailed) s
17 r (analysis) s
17 r (of) s
17 r (the) s
18 r 98 c
1 r (ene\014ts) s
19 r (of) s
18 r (using) s
17 r 99 c
(hannels) s
224 960 p (to) s
21 r (giv) s
101 c
20 r 97 c
22 r 116 c
119 c
-1 r 111 c
19 r (phase) s
22 r (address) s
22 r (binding.) s
40 r 70 c
-3 r (or) s
20 r (O/R) s
22 r (Addresses,) s
23 r (an) s
22 r (hierarc) s
(hi-) s
224 1016 p (cally) s
15 r (structured) s
17 r (table) s
15 r (is) s
16 r (used) s
17 r (to) s
15 r (determine) s
15 r (the) s
17 r (MT) s
-3 r (A.) s
14 r (This) s
16 r (is) s
16 r 97 c
16 r (table) s
15 r (based) s
224 1073 p (implemen) s
-1 r (t) s
-1 r (atio) s
-1 r 110 c
18 r (of) s
19 r (an) s
19 r (approac) s
104 c
18 r (describ) s
1 r (ed) s
20 r 98 c
121 c
19 r (Kille[13].) s
30 r (After) s
19 r (the) s
20 r (MT) s
-3 r 65 c
18 r (is) s
224 1129 p (determined,) s
16 r (an) s
16 r (outb) s
1 r (ound) s
17 r 99 c
(hannel) s
15 r (is) s
16 r (selected.) s
23 r (Where) s
17 r (an) s
16 r (MT) s
-3 r 65 c
15 r (is) s
16 r (accessible) s
224 1186 p 98 c
121 c
17 r 109 c
(ult) s
-1 r (iple) s
16 r 99 c
(hannels,) s
18 r 99 c
(hannel) s
17 r (binding) s
18 r (attempts) s
16 r (to) s
17 r (minim) s
-1 r (ise) s
16 r (proto) s
1 r (col) s
17 r (con-) s
224 1242 p 118 c
(ersions.) s
34 r (Managemen) s
-1 r 116 c
18 r 99 c
(hec) s
(king) s
19 r (is) s
19 r (also) s
20 r (carried) s
20 r (out) s
20 r (at) s
20 r (this) s
19 r 112 c
1 r (oin) s
(t,) s
20 r (as) s
20 r (it) s
20 r (is) s
224 1298 p (sometim) s
-1 r (es) s
16 r (imp) s
1 r (ortan) s
-1 r 116 c
16 r (to) s
17 r (con) s
(trol) s
16 r 99 c
(hannel) s
17 r 99 c
(hoice) s
17 r (for) s
18 r (administr) s
-1 r (ati) s
-1 r 118 c
-1 r 101 c
16 r (reasons.) s
224 1355 p (This) s
17 r (includes) s
18 r (application) s
16 r (of) s
18 r (authorisatio) s
-1 r 110 c
16 r (con) s
(trols,) s
15 r (so) s
18 r (that) s
17 r (routing) s
16 r (can) s
18 r 98 c
1 r 101 c
224 1411 p (regulated) s
20 r (on) s
21 r (the) s
21 r (basis) s
20 r (host,) s
21 r (net) s
119 c
(o) s
-1 r (rk) s
19 r (and) s
21 r (user.) s
36 r 65 c
21 r (full) s
20 r (description) s
20 r (of) s
20 r (the) s
224 1468 p (general) s
15 r (approac) s
104 c
14 r (is) s
14 r (giv) s
(en) s
14 r (in) s
15 r 97 c
15 r (pap) s
1 r (er) s
16 r 98 c
121 c
14 r (Kille) s
14 r (and) s
16 r (Brink[14].) s
295 1524 p 70 c
-3 r (ollo) s
-2 r (wing) s
14 r (the) s
16 r (determinatio) s
-1 r 110 c
15 r (of) s
15 r (the) s
16 r (outb) s
1 r (ound) s
17 r 99 c
(hannel,) s
15 r (the) s
17 r (reformat) s
-1 r (ting) s
224 1581 p (requiremen) s
(ts) s
12 r (are) s
14 r (calculated.) s
19 r (This) s
13 r (will) s
13 r 98 c
1 r 101 c
15 r 97 c
14 r (sequence) s
15 r (of) s
14 r 99 c
(hannels,) s
13 r (whic) s
104 c
13 r (will) s
224 1637 p 112 c
1 r (erform) s
19 r (the) s
19 r (correct) s
19 r (series) s
20 r (of) s
19 r (proto) s
1 r (col) s
18 r (and) s
20 r 98 c
1 r 111 c
1 r (dy) s
20 r (part) s
19 r (reformatti) s
-1 r (ng) s
18 r (on) s
19 r (the) s
224 1694 p (message) s
12 r (in) s
13 r (question.) s
19 r (This) s
13 r (is) s
13 r 97 c
13 r (surprisingly) s
13 r (di\016cult) s
13 r (calculation) s
12 r (to) s
12 r (mak) s
(e,) s
11 r (but) s
224 1750 p (it) s
15 r (can) s
15 r 98 c
1 r 101 c
16 r (done) s
15 r (in) s
15 r 97 c
15 r (reasonably) s
15 r (general) s
14 r (manner.) s
295 1807 p (When) s
17 r (all) s
15 r (of) s
17 r (the) s
17 r (recipien) s
116 c
15 r (addresses) s
17 r (ha) s
118 c
-1 r 101 c
15 r 98 c
1 r (een) s
18 r (accepted,) s
17 r (the) s
17 r (transfer) s
16 r (of) s
224 1863 p (the) s
15 r (con) s
(ten) s
116 c
13 r (tak) s
(es) s
14 r (place.) s
19 r (There) s
16 r (is) s
15 r 97 c
15 r (separate) s
14 r (transfer) s
15 r (for) s
14 r (eac) s
104 c
15 r (comp) s
1 r (onen) s
116 c
13 r (of) s
224 1919 p (the) s
15 r (initial) s
14 r (message) s
13 r (stored) s
15 r (in) s
15 r (the) s
16 r (queue.) s
20 r 70 c
-3 r (or) s
14 r (example:) s
cmsy10.329 @sf
292 2026 p 15 c
cmr10.329 @sf
23 r (An) s
15 r (RF) s
67 c
14 r (822) s
14 r (message) s
14 r (is) s
15 r (submitted) s
14 r (as) s
15 r 116 c
119 c
-1 r 111 c
12 r (\014les.) s
cmsy10.329 @sf
292 2120 p 15 c
cmr10.329 @sf
23 r 65 c
15 r (P2) s
15 r (message) s
14 r (is) s
14 r (submitted) s
14 r (as) s
15 r (one) s
15 r (\014le.) s
cmsy10.329 @sf
292 2213 p 15 c
cmr10.329 @sf
23 r 65 c
17 r (lo) s
1 r (cal) s
16 r (submission) s
16 r (of) s
17 r 97 c
17 r (complex) s
16 r (directory) s
16 r (hierarc) s
104 c
121 c
15 r (is) s
17 r (submitted) s
15 r (di-) s
338 2270 p (rectly) s
-3 r 46 c
295 2376 p (Finally) s
-4 r 44 c
18 r (an) s
121 c
18 r (remaining) s
17 r (managem) s
-1 r (en) s
-1 r 116 c
17 r 99 c
(hec) s
(ks) s
17 r (are) s
19 r (done) s
19 r (\(for) s
18 r (example) s
18 r (an) s
121 c
224 2432 p (required) s
21 r 99 c
(hec) s
(ks) s
19 r (on) s
21 r (message) s
20 r (con) s
(ten) s
-1 r (t\),) s
20 r (and) s
21 r (the) s
21 r (message) s
19 r (is) s
21 r (lo) s
1 r 99 c
107 c
-1 r (ed) s
19 r (in) s
(to) s
19 r (the) s
224 2489 p (queue.) s
295 2545 p (In) s
20 r (addition,) s
19 r (Submit) s
18 r (understands) s
20 r (the) s
19 r (P1) s
19 r (lev) s
(el) s
17 r (asp) s
1 r (ects) s
20 r (of) s
19 r (RF) s
67 c
18 r (987) s
18 r ([9],) s
224 2602 p ([15].) s
18 r (This) s
14 r (means) s
12 r (that) s
13 r (it) s
13 r (can) s
14 r (map) s
13 r (addresses) s
14 r 98 c
1 r (et) s
119 c
-1 r (een) s
13 r (the) s
14 r 116 c
119 c
-1 r 111 c
11 r (forms,) s
12 r (and) s
14 r (will) s
224 2658 p (correctly) s
15 r (handle) s
15 r (trace) s
15 r (and) s
15 r (Message) s
15 r (Ids.) s
960 2916 p 54 c
@eop
5 @bop0
cmbx10.432 @sf
[<007FFFF8007FFFF8007FFFF80000FF000000FF000000FF000000FF000000FF000000FF000000FF00FFFFFFF8FFFFFFF8FFFF
  FFF8F0007F0078007F003C007F001E007F001E007F000F007F0007807F0003C07F0001C07F0001E07F0000F07F0000787F00
  003C7F00003C7F00001E7F00000F7F000007FF000003FF000003FF000001FF000000FF0000007F0000007F0000003F000000
  1F0000000F00> 29 39 -2 0 34] 52 @dc
[<E03FF800F1FFFE00FFFFFF80FFE01FC0FF0007E0FC0003E0F80003F0F00001F0F00001F8E00001F8E00001F8E00001F8E000
  03F8000007F800000FF800001FF80003FFF8003FFFF001FFFFF007FFFFE00FFFFFC01FFFFFC03FFFFF807FFFFE007FFFF800
  FFFFC000FFF80000FF800000FF0000E0FE0000E0FC0000E0FC0001E0FC0001E07C0003E07E0003E03E0007E03F001FE01FC0
  FFE00FFFFFE003FFF1E000FF80E0> 29 41 -4 0 38] 83 @dc
/cmr8.300 @newfont
cmr8.300 @sf
[<1F807FE0F1F0F0F0F078F078707800780078607078F07FE06F8060006000600060007FC07FE07FF07030> 13 21 -2 0 18] 53 @dc
/cmr6.300 @newfont
cmr6.300 @sf
[<3F007FC0E1C0E0E0E0E000E000E060E071C07F806000600078007F007F806180> 11 16 -1 0 15] 53 @dc
/cmr9.300 @newfont
cmr9.300 @sf
[<007F000001FFC00007C1F0000F0078001E003C003C001E0038000E0078000F0078000F00F0000780F0000780F0000780F000
  0780F0000780F0000780F0000780F00007807000070078000F0038000E003C001E001E003C000F00780007C1F00001FFC000
  007F0000> 25 26 -2 0 30] 79 @dc
[<FF9FC0FF9FC01C0E001C0E001C0E001C0E001C0E001C0E001C0E001C0E001C0E001C0E001E0E001F1E00FDFC00FCF800> 18 16 -1 0 21] 110 @dc
[<FFFFC0FFFFF00F00F80F003C0F003C0F001E0F001E0F001E0F001E0F001E0F003C0F00780F01F00FFFE00FFFE00F00F00F00
  780F003C0F003C0F003C0F003C0F003C0F00780F00F0FFFFE0FFFF80> 23 26 -1 0 27] 66 @dc
[<C7F0DFFCFC3CF00EE00FE007C007C007000F000F003F01FE0FFE3FFC7FF07F80FC00F000E006E006E00EE00E701E787E3FF6
  0FC6> 16 26 -2 0 21] 83 @dc
[<FFFF8000FFFFE0000F00F8000F003C000F001C000F000E000F000F000F0007000F0007000F0007800F0007800F0007800F00
  07800F0007800F0007800F0007800F0007800F0007000F000F000F000E000F000E000F001C000F0038000F00F000FFFFE000
  FFFF8000> 25 26 -1 0 29] 68 @dc
[<003F800000FFE00001E0F00003C03800078018000F801C000F000C000F000C000F000C000F000C000F000C000F000C000F00
  0C000F000C000F000C000F000C000F000C000F000C000F000C000F000C000F000C000F000C000F000C000F001E00FFF0FFC0
  FFF0FFC0> 26 26 -1 0 29] 85 @dc
[<FFC00C00FFC01C001E003C000C003C000C007C000C00FC000C00FC000C01FC000C01EC000C03EC000C07CC000C078C000C0F
  8C000C1F0C000C1F0C000C3E0C000C7C0C000C7C0C000CF80C000CF00C000DF00C000FE00C000FC00C000FC01E00FF80FFC0
  FF00FFC0> 26 26 -1 0 29] 78 @dc
[<FFF0FFF00F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F00FFF0
  FFF0> 12 26 -1 0 14] 73 @dc
[<FFC1FFC0FFC1FFC01F007C00070078000300F8000380F0000181F00000C3E00000E3C0000067C00000778000003F0000001F
  0000001E0000003E0000007E0000007F000000FB000000F3800001E1800003E0C00003C0E00007C070000F80F800FFE1FF00
  FFE1FF00> 26 26 -1 0 29] 88 @dc
[<6060703038181878F8F8F870> 5 12 -3 7 11] 44 @dc
[<07800FC01EE01C601C601C601C601C001C001C001C001C001C001C00FFC0FFC03C001C001C000C000C000C000C00> 11 23 -1 0 15] 116 @dc
[<FF9FC0FF9FC01C0E001C0E001C0E001C0E001C0E001C0E001C0E001C0E001C0E001C0E001E0E001F1E001DFC001CF8001C00
  001C00001C00001C00001C00001C00001C00001C0000FC0000FC0000> 18 26 -1 0 21] 104 @dc
[<FF80FF801C001C001C001C001C001C001C001C001C001C001C001C00FC00FC00000000000000000000003C007C007C007C00
  3C00> 9 26 0 0 10] 105 @dc
[<DF80FFE0F0F0E070E070C0F001F01FE07FC0FF80F000E060E060F0E07FE03FE0> 12 16 -1 0 15] 115 @dc
[<07E01FF83C3C781E700EF00FF00FF00FF00FF00FF00F700E781E3C3C1FF807E0> 16 16 -1 0 19] 111 @dc
[<07E01FF83E1C7C0C7800F000F000F000F000F000F0007818783C3E3C1FFC07F8> 14 16 -1 0 17] 99 @dc
[<FF3FC0FF3FC01C1E001C3C001C3C001C78001CF0001EF0001FE0001FC0001DC0001CE0001C70001C7C001C7F801C7F801C00
  001C00001C00001C00001C00001C00001C00001C0000FC0000FC0000> 18 26 -1 0 20] 107 @dc
[<07E01FF83E1C7C0C7800F000F000F000F000FFFCFFFC703C783C3C781FF007E0> 14 16 -1 0 17] 101 @dc
[<70F8F8F870> 5 5 -3 0 11] 46 @dc
5 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
224 195 p (constrained) s
14 r (to) s
14 r (In) s
(ter) s
14 r 80 c
(ersonal) s
13 r (Messaging) s
13 r (\(IPM\),) s
14 r (although) s
13 r (IPM) s
15 r (is) s
14 r (the) s
15 r (ma) s
3 r (jor) s
224 252 p (target.) s
21 r (The) s
17 r (main) s
14 r (reason) s
16 r (for) s
15 r (this) s
16 r (structure) s
16 r (is) s
15 r (to) s
16 r (facilitate) s
14 r (reformatt) s
-1 r (ing.) s
20 r (By) s
224 308 p (putting) s
14 r (the) s
14 r 118 c
-2 r (arious) s
12 r (comp) s
1 r (onen) s
(ts) s
12 r (in) s
(to) s
12 r (separate) s
13 r (\014les,) s
14 r (reformatt) s
-1 r (ing) s
12 r (can) s
14 r 98 c
1 r 101 c
15 r (car-) s
224 364 p (ried) s
14 r (out) s
14 r (in) s
15 r 97 c
14 r (straigh) s
-1 r (tfo) s
-1 r (rw) s
-1 r (ar) s
-1 r 100 c
13 r (manner.) s
19 r (It) s
14 r (is) s
14 r (exp) s
1 r (ected) s
16 r (that) s
13 r 97 c
14 r (PP) s
15 r (related) s
13 r (User) s
224 421 p (Agen) s
116 c
15 r 119 c
(ould) s
15 r (also) s
16 r (use) s
17 r (this) s
16 r (structure,) s
16 r (as) s
16 r (it) s
16 r (app) s
1 r (ears) s
17 r (to) s
16 r (o\013er) s
15 r 97 c
17 r (go) s
1 r 111 c
1 r 100 c
16 r (basis) s
16 r (for) s
224 477 p (\015exible) s
15 r (construction) s
15 r (of) s
15 r (Multim) s
-1 r (edia) s
13 r (Messages.) s
cmbx10.432 @sf
224 621 p 52 c
69 r (Message) s
23 r (Submi) s
-1 r (ssion) s
cmr10.329 @sf
224 722 p (The) s
20 r (pro) s
1 r (cess) s
19 r (of) s
19 r (message) s
18 r (submission) s
17 r (is) s
19 r (no) s
119 c
18 r (considered) s
19 r (in) s
20 r 97 c
18 r (little) s
18 r (more) s
18 r (de-) s
224 778 p (tail.) s
25 r 65 c
17 r (message) s
16 r (is) s
17 r (submitted) s
16 r 98 c
121 c
16 r (either) s
17 r 97 c
17 r (User) s
17 r (Agen) s
116 c
16 r (\(lo) s
1 r (cal) s
16 r (submission\)) s
15 r (or) s
224 835 p 98 c
121 c
19 r (an) s
20 r (in) s
98 c
1 r (ound) s
20 r 99 c
(hannel,) s
20 r (whic) s
104 c
19 r (in) s
(teract) s
-1 r 115 c
18 r (with) s
20 r (Submit) s
19 r (through) s
19 r 97 c
20 r (proto) s
1 r (col.) s
224 891 p (The) s
13 r (curren) s
116 c
12 r 118 c
(ersion) s
11 r (of) s
12 r (this) s
12 r (proto) s
1 r (col) s
12 r (uses) s
13 r 97 c
12 r (UNIX) s
13 r (In) s
(ter-Pro) s
1 r (cess) s
12 r (Comm) s
-2 r (unica-) s
224 948 p (tion) s
13 r (\(IPC\)) s
13 r (mec) s
(hanism) s
cmr8.300 @sf
666 933 p 53 c
cmr10.329 @sf
684 948 p 44 c
13 r (although) s
13 r (in) s
13 r 97 c
14 r (later) s
12 r 118 c
(ersion) s
12 r (OSI) s
15 r (Remote) s
13 r (Op) s
1 r (erations) s
224 1004 p (\(R) s
(OS\)) s
18 r (ma) s
121 c
17 r 98 c
1 r 101 c
19 r (used.) s
33 r (The) s
19 r (enco) s
1 r (ding) s
19 r (of) s
19 r (the) s
19 r (curren) s
116 c
18 r (proto) s
1 r (col) s
18 r (is) s
19 r (text) s
19 r (based,) s
224 1061 p (with) s
19 r (parameters) s
17 r (equiv) s
-2 r (alen) s
116 c
17 r (to) s
18 r (the) s
19 r (queue) s
20 r (structure.) s
31 r 70 c
-3 r (or) s
18 r 97 c
19 r 110 c
(um) s
-2 r 98 c
1 r (er) s
18 r (of) s
19 r (rea-) s
224 1117 p (sons,) s
14 r (it) s
14 r 119 c
(as) s
13 r (decided) s
16 r (not) s
14 r (to) s
14 r (use) s
15 r 97 c
14 r (standard) s
14 r (proto) s
1 r (col,) s
14 r (in) s
14 r (particular) s
14 r (P3,) s
14 r (whic) s
104 c
224 1174 p (is) s
15 r (the) s
15 r (ob) s
(vious) s
14 r (candidate:) s
cmsy10.329 @sf
292 1280 p 15 c
cmr10.329 @sf
23 r (There) s
17 r (is) s
17 r 97 c
17 r (need) s
18 r (for) s
17 r (services) s
17 r (not) s
17 r (supp) s
1 r (orted) s
18 r 98 c
121 c
16 r (the) s
17 r (standard) s
17 r (proto) s
1 r (col.) s
338 1336 p (P3) s
15 r (do) s
1 r (es) s
15 r (not) s
15 r (allo) s
-1 r 119 c
13 r (for) s
15 r (this) s
15 r (form) s
13 r (of) s
15 r (priv) s
-2 r (ate) s
14 r (extension.) s
cmsy10.329 @sf
292 1430 p 15 c
cmr10.329 @sf
23 r (There) s
17 r (is) s
17 r 97 c
17 r (di\016cult) s
121 c
16 r (with) s
16 r (migrati) s
-1 r (on.) s
25 r (Migrati) s
-1 r (on) s
16 r 98 c
1 r (ecomes) s
17 r 109 c
-1 r (uc) s
-1 r 104 c
16 r (easier) s
338 1487 p (when) s
15 r (this) s
14 r 107 c
(ey) s
13 r (in) s
(ternal) s
13 r (proto) s
1 r (col) s
14 r (is) s
14 r 97 c
14 r (sup) s
1 r (erset) s
15 r (of) s
14 r (all) s
14 r (supp) s
1 r (orted) s
15 r (external) s
338 1543 p (proto) s
1 r (cols.) s
cmsy10.329 @sf
292 1637 p 15 c
cmr10.329 @sf
23 r 70 c
-3 r (or) s
9 r (user) s
12 r (agen) s
(ts,) s
10 r (and) s
11 r (some) s
10 r (proto) s
1 r (cols,) s
11 r (it) s
10 r (is) s
11 r (imp) s
1 r (ortan) s
-1 r 116 c
9 r (to) s
10 r 112 c
1 r (erform) s
10 r (address) s
338 1693 p 99 c
(hec) s
(king) s
11 r (prior) s
13 r (to) s
13 r (transferring) s
13 r (the) s
13 r (con) s
(ten) s
116 c
12 r (of) s
13 r (the) s
14 r (message.) s
18 r (P3) s
13 r (do) s
1 r (es) s
14 r (not) s
338 1750 p (supp) s
1 r (ort) s
15 r (this,) s
14 r (whic) s
104 c
14 r (is) s
15 r 97 c
15 r (ma) s
3 r (jor) s
13 r (de\014ciency) s
-3 r 46 c
cmsy10.329 @sf
292 1844 p 15 c
cmr10.329 @sf
23 r (Submit) s
13 r (supp) s
1 r (orts) s
15 r 98 c
1 r (oth) s
15 r (the) s
15 r (Message) s
14 r 84 c
-3 r (ransfer) s
13 r (Abstract) s
14 r (Service) s
15 r (and) s
15 r (the) s
338 1900 p (MT) s
-3 r 65 c
14 r (Abstract) s
14 r (Service,) s
15 r (whereas) s
15 r (P3) s
15 r (only) s
15 r (supp) s
1 r (orts) s
15 r (the) s
15 r (former.) s
295 2006 p (The) s
19 r (\014rst) s
18 r (stage) s
18 r (of) s
19 r (message) s
17 r (submission) s
18 r (is) s
18 r (for) s
18 r (the) s
19 r (submitting) s
17 r (pro) s
1 r (cess) s
19 r (to) s
224 2063 p (either) s
14 r (in) s
118 c
-1 r (ok) s
-1 r 101 c
12 r 97 c
14 r (Submit) s
13 r (pro) s
1 r (cess,) s
14 r (or) s
14 r (to) s
14 r (connect) s
14 r (to) s
14 r (an) s
14 r (already) s
13 r (existing) s
14 r (Submit) s
224 2119 p (daemon.) s
30 r (An) s
19 r (in) s
118 c
-1 r (ok) s
-2 r (ed) s
18 r (Submit) s
17 r (pro) s
1 r (cess) s
19 r (will) s
18 r (then) s
19 r (connect) s
19 r (to) s
18 r (the) s
19 r (QMGR,) s
18 r (to) s
224 2176 p (ensure) s
20 r (that) s
18 r (message) s
17 r (submission) s
18 r (is) s
18 r (not) s
19 r (curren) s
(tly) s
17 r (prohibited.) s
31 r (Then) s
19 r (all) s
18 r (the) s
224 2232 p 112 c
1 r (er-message) s
14 r (parameters) s
14 r (will) s
13 r 98 c
1 r 101 c
16 r (sen) s
116 c
14 r (to) s
15 r (Submit,) s
13 r (whic) s
104 c
14 r (will) s
14 r 112 c
1 r (erform) s
14 r 118 c
-2 r (alidit) s
-1 r 121 c
224 2289 p 99 c
(hec) s
(ks) s
20 r (and) s
22 r (initial) s
20 r (managemen) s
-1 r 116 c
19 r 99 c
(hec) s
(ks.) s
38 r (Submit) s
21 r (then) s
22 r (ac) s
(kno) s
(wl) s
-1 r (edges) s
20 r (these) s
224 2345 p (parameters,) s
13 r (and) s
16 r (address) s
15 r 99 c
(hec) s
(king) s
13 r 98 c
1 r (egins.) s
295 2402 p (PP) s
20 r (curren) s
(tly) s
18 r (recognises) s
20 r 116 c
119 c
-1 r 111 c
18 r (forms) s
18 r (of) s
20 r (address:) s
29 r (RF) s
67 c
19 r (822) s
19 r (address,) s
21 r (and) s
224 2458 p (X.400) s
15 r (O/R) s
16 r (Address.) s
23 r (Both) s
15 r (of) s
16 r (these) s
16 r (are) s
16 r (handled) s
16 r (using) s
16 r 97 c
15 r (text) s
16 r (enco) s
1 r (ding.) s
22 r 70 c
-3 r (or) s
224 2514 p (X.400,) s
16 r (this) s
16 r (uses) s
17 r (the) s
17 r (RF) s
67 c
16 r (987) s
16 r ([9]) s
16 r (domain) s
15 r (orien) s
(ted) s
15 r (enco) s
1 r (ding.) s
26 r 65 c
17 r (submitting) s
224 2571 p (pro) s
1 r (cess) s
13 r (will) s
12 r (select) s
12 r (the) s
13 r (appropriate) s
12 r (form,) s
11 r (and) s
13 r (then) s
13 r (o\013er) s
12 r (eac) s
104 c
12 r (address) s
13 r (in) s
13 r (turn.) s
224 2627 p (There) s
16 r (are) s
14 r 116 c
119 c
-1 r 111 c
13 r (mo) s
1 r (des) s
15 r (of) s
14 r (in) s
(teraction:) s
224 2658 p 598 2 ru
cmr6.300 @sf
276 2686 p 53 c
cmr9.300 @sf
293 2701 p (On) s
13 r (BSD) s
14 r (UNIX,) s
12 r (this) s
13 r (is) s
14 r (So) s
1 r 99 c
107 c
(ets.) s
cmr10.329 @sf
960 2916 p 53 c
@eop
4 @bop0
cmr8.300 @sf
[<1FE07FF8F07CF83EF81EF81E701E001E003C007807E007F000F80078303C783C7C3C783C78783FF80FE0> 15 21 -1 0 18] 51 @dc
[<03FE03FE0070007000700070FFFEFFFEE0707070307018701C700E700670077003F001F000F000F00070> 15 21 -1 0 18] 52 @dc
cmr10.329 @sf
[<FFFFFFFFFFF0FFFFFFFFFFF0> 44 2 0 -11 45] 124 @dc
cmr6.300 @sf
[<3F00FFC0E1C0E0E0E0E000E000E001C00F80038001C021C071C073C07F801F00> 11 16 -1 0 15] 51 @dc
cmr9.300 @sf
[<03FFF00003FFF000001E0000001E0000001E0000001E0000001E0000001E0000001E0000001E0000001E0000001E0000001E
  0000001E0000001E0000001E0000001E0000C01E0180C01E0180C01E0180E01E0380601E0300601E0300781E0F007FFFFF00
  7FFFFF00> 25 26 -1 0 28] 84 @dc
[<3F1E007FFF00F8F980F07980F03980F03800F838007C38003F380007F80000380030380078780078F8007FF0003FC000> 17 16 -1 0 19] 97 @dc
[<19F8001BFE001F1F001E07801C07801C03C01C03C01C03C01C03C01C03C01C03C01C07801E07801F0F001FFE001CFC001C00
  001C00001C00001C00001C00001C00001C00001C0000FC0000FC0000> 18 26 -1 0 21] 98 @dc
[<FFC0FFC01C001C001C001C001C001C001C001C001C001C601EF01FF0FFF0FFE0> 12 16 -1 0 15] 114 @dc
[<FF8000FF80001C00001C00001C00001C00001C00001DF8001FFE001F1F001E0F801C07801C07C01C03C01C03C01C03C01C03
  C01C03C01C07801E0F801F1F00FFFE00FCFC00> 18 23 -1 7 21] 112 @dc
[<0FCFC01FFFC03C3E00781E00780E00F00E00F00E00F00E00F00E00F00E00F00E00780E00781E003E3E001FFE000FEE00000E
  00000E00000E00000E00000E00000E00000E00000E00007E00007E00> 18 26 -1 0 21] 100 @dc
[<FF1FE0FF1FE01E0E00061C00071C0003B80001F00000F00000E00001E00003B00003B800071C000F1E007F1FC07F1FC0> 19 16 0 0 20] 120 @dc
[<7FE07FE00E000E000E000E000E000E000E000E000E000E000E000E00FFC0FFC00E000E000E000E000E000E3C0F3C07BC03FC
  00F8> 14 26 0 0 12] 102 @dc
[<07CFC00FEFC01E3E001C1E001C0E001C0E001C0E001C0E001C0E001C0E001C0E001C0E001C0E001C0E00FC7E00FC7E00> 18 16 -1 0 21] 117 @dc
[<07FF07FF00700070007000700070FFFFFFFFE07060703070387018700C700E7006700770037001F001F000F000700070> 16 24 -1 0 19] 52 @dc
[<07E01FF83C3C381C700E700E700EF00FF00FF00FF00FF00FF00FF00FF00FF00FF00F700E700E700E381C3C3C1FF807E0> 16 24 -1 0 19] 48 @dc
[<01C00380070007000E001C001C00380038007000700070006000E000E000E000E000E000E000E000E000E000E000E000E000
  6000700070007000380038001C001C000E0007000700038001C0> 10 38 -2 10 15] 40 @dc
[<0FC03FF07C787038F01CE01CE01CE03CF03C70FC79F83FF81FF03FC03FF07E707838701860186018703838701FE00FC0> 14 24 -2 0 19] 56 @dc
[<E0007000380038001C000E000E0007000700038003800380018001C001C001C001C001C001C001C001C001C001C001C001C0
  0180038003800380070007000E000E001C00380038007000E000> 10 38 -2 10 15] 41 @dc
cmr6.300 @sf
[<1FE01FE0038003800380FFE0FFE0C380638033801B800B800F80078003800180> 11 16 -1 0 15] 52 @dc
cmr9.300 @sf
[<FF9FE7F8FF9FE7F81C0701C01C0701C01C0701C01C0701C01C0701C01C0701C01C0701C01C0701C01C0701C01C0701C01E07
  81C01F0FC3C0FDFE7F80FCFC3F00> 29 16 -1 0 32] 109 @dc
/cmti9.300 @newfont
cmti9.300 @sf
[<3E3C007FFE00F3F700F0F300E07300F07B00F07800F03800F03800F83C00783C00781C003C3C001E7E000FFE0003EE00000E
  00000F00000F00000700000700000780000780000380001F80001F80> 17 26 -4 0 20] 100 @dc
[<3E003F0073007B80398039803C001C001C001E00CE00CE00EF0067007E003E00000000000000000000000380078007800380> 9 25 -3 0 12] 105 @dc
[<700078007800380038003C003C001C001C001E00DE38CE3CCF3CEF9C7FFC3CF8> 14 16 -3 0 16] 114 @dc
[<1F803FE078F0F038F018F000F000F000FF80FFF0787878383C181E380FF803F0> 13 16 -4 0 18] 101 @dc
[<1F807FE078F0F038E018F000F000F000F000F800780078783C781F780FF803F0> 13 16 -4 0 18] 99 @dc
[<7C00FE00E700E380E180F18070007000700078003800380038003C00FFC0FFC01C001E000E000E000E000E000600> 10 23 -4 0 13] 116 @dc
[<1F807FE079F0F078E03CF03CF03EF01EF01EF81E781E780E3C1E1F3C0FFC03F0> 15 16 -4 0 20] 111 @dc
[<7F0000FF8000F3E000F0F000F070002078000038000FF8001FF8001C7C003C3C00381C003C1C001C1E001C0E001C0E001E0E
  00CE0F00CE0700EF07006707007E07803E0380> 17 23 -3 7 19] 121 @dc
cmr9.300 @sf
[<780000FE0000C70000F3000061800001800001800001C00001C00001C00003E00003E00007F0000730000730000E38000E18
  000E18001C0C001C0C003C0E00FF3F80FF3F80> 17 23 -1 7 20] 121 @dc
cmti9.300 @sf
[<FFFE0000FFFF80001F03E0000F00F0000F0078000F803C000F801C0007800E0007800F0007C00F0007C0070003C0078003C0
  078003E0078003E003C001E003C001E003C001F003C001F003C000F0038000F0038000F8038000F8070000781F0003FFFE00
  03FFF800> 26 26 -3 0 29] 68 @dc
cmr9.300 @sf
[<0380E0000380E0000380E00007C1F00007C1F00007C1B0000763B0000E6398000E6318000E3718001C370C001C360C001C3E
  0C003C1E0E00FF3F9F80FF3F9F80> 25 16 -1 0 28] 119 @dc
[<1F803FE070F0E078F03CF03CF03C703C003C003C003C707878F87FF06FC0600060006000600060007FE07FF07FF87038> 14 24 -2 0 19] 53 @dc
4 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
295 195 p (The) s
14 r (recipien) s
116 c
13 r (parameters) s
13 r (whic) s
104 c
13 r 99 c
(hange) s
14 r (whilst) s
13 r (the) s
15 r (message) s
13 r (is) s
14 r 98 c
1 r (eing) s
15 r (pro-) s
224 252 p (cessed) s
14 r (are) s
12 r (enco) s
1 r (ded) s
14 r (in) s
13 r 97 c
12 r (\014xed) s
13 r (format,) s
11 r (to) s
12 r (maxim) s
-1 r (ise) s
10 r (robustness) s
13 r (when) s
13 r 97 c
13 r (con) s
(trol) s
224 308 p (\014le) s
16 r (is) s
15 r (altered.) s
22 r (There) s
16 r (are) s
16 r (also) s
14 r 97 c
16 r (large) s
15 r 110 c
(um) s
-1 r 98 c
1 r (er) s
14 r (of) s
16 r (parameters) s
14 r (whic) s
104 c
14 r (apply) s
16 r (to) s
224 364 p (the) s
16 r (whole) s
16 r (message.) s
21 r (These) s
17 r (include) s
16 r (parameters) s
14 r (to) s
16 r (represen) s
116 c
15 r (all) s
15 r (of) s
15 r (the) s
17 r (X.400) s
224 421 p (Message) s
15 r 84 c
-3 r (ransfer) s
13 r (Service:) s
cmsy10.329 @sf
292 521 p 15 c
cmr10.329 @sf
23 r (The) s
21 r (message) s
19 r (originato) s
-1 r (r,) s
20 r (using) s
20 r (the) s
21 r (same) s
19 r (enco) s
1 r (ding) s
21 r (as) s
20 r 97 c
20 r (recipien) s
(t,) s
20 r (to) s
338 578 p (facilitate) s
13 r (error) s
14 r (returns.) s
cmsy10.329 @sf
292 670 p 15 c
cmr10.329 @sf
23 r (The) s
15 r (Message) s
15 r (ID.) s
cmsy10.329 @sf
292 762 p 15 c
cmr10.329 @sf
23 r 65 c
18 r (list) s
16 r (of) s
18 r (enco) s
1 r (ded) s
19 r (informati) s
-1 r (on) s
16 r 116 c
(yp) s
1 r (es,) s
17 r (whic) s
104 c
17 r (is) s
18 r (not) s
17 r (restricted) s
17 r (to) s
18 r (those) s
338 818 p (de\014ned) s
16 r (in) s
15 r (P2.) s
cmsy10.329 @sf
292 910 p 15 c
cmr10.329 @sf
23 r (Deferred) s
15 r (deliv) s
(ery) s
14 r (date.) s
cmsy10.329 @sf
292 1003 p 15 c
cmr10.329 @sf
23 r 84 c
-3 r (race) s
14 r (informat) s
-1 r (ion) s
295 1103 p (In) s
16 r (addition,) s
13 r (there) s
16 r (are) s
15 r 118 c
-2 r (arious) s
13 r (services) s
15 r (not) s
15 r (supp) s
1 r (orted) s
16 r 98 c
121 c
14 r (X.400:) s
cmsy10.329 @sf
292 1203 p 15 c
cmr10.329 @sf
23 r (Con) s
(trol) s
12 r (of) s
15 r (logging) s
14 r (on) s
15 r 97 c
15 r 112 c
1 r (er) s
15 r (message) s
14 r (basis.) s
cmsy10.329 @sf
292 1295 p 15 c
cmr10.329 @sf
23 r (Pro) s
(visi) s
-1 r (on) s
18 r (of) s
20 r (message) s
18 r (dela) s
121 c
(ed) s
18 r (deliv) s
(ery) s
18 r 119 c
(arnings) s
18 r (at) s
20 r 97 c
20 r (user) s
20 r (con) s
(trol) s
-1 r (led) s
338 1352 p (in) s
(terv) s
-2 r (a) s
-1 r (l.) s
cmsy10.329 @sf
292 1444 p 15 c
cmr10.329 @sf
23 r (Sp) s
1 r (eci\014cation) s
15 r (of) s
15 r (non-deliv) s
(ery) s
14 r (timeout) s
cmr8.300 @sf
1071 1429 p 51 c
cmr10.329 @sf
1089 1444 p 46 c
295 1544 p (The) s
17 r (message) s
15 r (itself) s
16 r (is) s
16 r (stored) s
16 r (within) s
16 r 97 c
16 r (directory) s
cmr8.300 @sf
1224 1529 p 52 c
cmr10.329 @sf
1243 1544 p 44 c
17 r (whic) s
104 c
16 r (con) s
(tai) s
-1 r (ns) s
15 r (sev) s
(eral) s
224 1601 p (things:) s
cmsy10.329 @sf
292 1701 p 15 c
cmr10.329 @sf
23 r 65 c
15 r (directory) s
14 r (called) s
15 r (\\base",) s
15 r (whic) s
104 c
14 r (con) s
(tains) s
13 r (the) s
15 r (message) s
14 r (as) s
15 r (submitted.) s
cmsy10.329 @sf
292 1793 p 15 c
cmr10.329 @sf
23 r (Zero) s
17 r (or) s
17 r (more) s
16 r (additional) s
16 r (directories,) s
17 r (whic) s
104 c
16 r (con) s
(tain) s
16 r (mo) s
1 r (di\014ed) s
17 r (forms) s
16 r (of) s
338 1849 p (the) s
18 r (original) s
15 r (message.) s
26 r (These) s
18 r (directories) s
17 r (are) s
17 r (lab) s
1 r (elled) s
17 r (according) s
17 r (to) s
17 r (the) s
338 1906 p (sequence) s
16 r (of) s
15 r (mo) s
1 r (di\014cations) s
13 r (\(c) s
(hannels\)) s
14 r (whic) s
104 c
14 r (ha) s
118 c
-1 r 101 c
14 r 98 c
1 r (een) s
16 r (applied.) s
cmsy10.329 @sf
292 1998 p 15 c
cmr10.329 @sf
23 r (Zero) s
12 r (or) s
12 r (more) s
11 r (deliv) s
(ery) s
11 r (noti\014cations,) s
12 r (whic) s
104 c
11 r (are) s
13 r (referenced) s
13 r (from) s
11 r (the) s
13 r (con-) s
338 2054 p (trol) s
18 r (\014le.) s
31 r (Sev) s
(eral) s
18 r (recipien) s
(ts) s
18 r (can) s
19 r (share) s
19 r (one) s
19 r (deliv) s
(ery) s
18 r (noti\014cation.) s
30 r (The) s
338 2111 p (adv) s
-2 r (an) s
(ta) s
-1 r (ges) s
15 r (of) s
17 r (storing) s
16 r (the) s
18 r (deliv) s
(ery) s
15 r (noti\014cation) s
16 r (with) s
17 r (the) s
17 r (original) s
15 r (mes-) s
338 2167 p (sage) s
15 r (are) s
14 r (discussed) s
16 r (later.) s
295 2268 p 65 c
17 r (PP) s
18 r (message) s
16 r (con) s
(ten) s
-1 r 116 c
15 r (is) s
17 r (stored) s
17 r (as) s
17 r (directory) s
-3 r 46 c
26 r (The) s
17 r (message) s
16 r (header) s
18 r (\(P2) s
224 2324 p (or) s
15 r (equiv) s
-2 r (alen) s
(t\),) s
13 r (and) s
16 r (eac) s
104 c
14 r 98 c
1 r 111 c
1 r (dy) s
17 r (part) s
15 r (are) s
15 r (stored) s
15 r (as) s
15 r (\014les.) s
21 r 70 c
-3 r (orw) s
-1 r (arded) s
14 r (Messages) s
224 2381 p (are) s
14 r (stored) s
13 r (as) s
13 r (sub) s
1 r (directories) s
14 r 124 c
14 r (the) s
14 r (format) s
11 r (de\014nition) s
14 r (is) s
13 r (recursiv) s
(e.) s
18 r (Eac) s
104 c
13 r (com-) s
224 2437 p 112 c
1 r (onen) s
116 c
18 r (is) s
19 r (lab) s
1 r (elled) s
18 r (according) s
19 r (to) s
18 r (its) s
18 r 116 c
(yp) s
1 r (e.) s
30 r (Bo) s
1 r (dy) s
19 r (parts) s
19 r (also) s
17 r (ha) s
118 c
101 c
17 r 97 c
19 r 110 c
(um) s
-2 r 98 c
1 r (er,) s
224 2493 p (so) s
21 r (that) s
20 r (sequencing) s
21 r (can) s
21 r 98 c
1 r 101 c
21 r (preserv) s
(ed.) s
36 r 70 c
-3 r (or) s
20 r (example:) s
30 r 65 c
20 r (\014le) s
21 r (called) s
21 r (\\3.ia5") s
224 2550 p 119 c
(ould) s
17 r 98 c
1 r 101 c
20 r (the) s
19 r (third) s
18 r 98 c
1 r 111 c
1 r (dy) s
20 r (part) s
18 r (con) s
(taining) s
17 r (IA5) s
19 r (text;) s
20 r 65 c
19 r (\014le) s
19 r (called) s
18 r (\\hdr.P2") s
224 2606 p 119 c
(ould) s
19 r (con) s
(tai) s
-1 r 110 c
19 r 97 c
19 r (heading) s
20 r (enco) s
1 r (ded) s
22 r (according) s
19 r (to) s
19 r (P2.) s
34 r (This) s
20 r (structure) s
19 r (is) s
20 r (not) s
224 2644 p 598 2 ru
cmr6.300 @sf
276 2672 p 51 c
cmr9.300 @sf
293 2687 p (This) s
14 r (can) s
13 r 98 c
1 r 101 c
14 r (represen) s
(ted) s
13 r (in) s
14 r (an) s
13 r (extension) s
15 r (feature) s
13 r (of) s
13 r (X.400\(88\)) s
cmr6.300 @sf
276 2718 p 52 c
cmr9.300 @sf
293 2733 p (The) s
18 r (term) s
cmti9.300 @sf
17 r (dir) s
-1 r 101 c
-2 r (cto) s
-1 r (ry) s
cmr9.300 @sf
16 r (is) s
18 r (tak) s
(en) s
17 r (to) s
17 r (mean) s
18 r 97 c
18 r (UNIX) s
17 r (directory) s
-2 r 44 c
18 r (as) s
18 r (distinct) s
19 r (from) s
18 r (the) s
17 r (term) s
cmti9.300 @sf
224 2778 p (Dir) s
-1 r 101 c
-1 r (c) s
-1 r (tory) s
cmr9.300 @sf
10 r (whic) s
104 c
13 r (refers) s
13 r (to) s
13 r (an) s
13 r (X.500) s
13 r (Directory) s
-2 r 46 c
cmr10.329 @sf
960 2916 p 52 c
@eop
3 @bop0
cmti10.329 @sf
[<0001F0000001FC000003FC000003FE000003FF000003C70000038300000181800001808000FF008007FF80000FC3E0001EC1
  F8003EE33C003C7F1E007C3E0F0078000F80780007C0F80003C0F80003E0F80001E0F80001F0F80001F07C0000F87C0000F8
  7C0000F83C0000F83E0000F83E0000781F0000781F0000780F800078078000F803C000F001E000F000F001E0007803E0001F
  07C0000FFF000001FC00> 29 40 -6 9 35] 81 @dc
[<FFE183FFE000FFE1C3FFE0001F01C03E00000E01E01E00000601F01E00000701F01F00000301D81F00000303980F00000303
  8C0F000003838C0F80000183860F800001838307800001838307800001C38187C00000C38187C00000C380C3C00000C380C3
  C00000E38063E00000638033E00000638031E00000638019E00000770019F0000037000DF0000037000CF00000370006F000
  003F0003F800001F0003F800001F0001F800001F0001FC0001FF0000FFC001FF0000FFC0> 42 31 -3 0 41] 77 @dc
[<00FF040007FFCC000FC0FE001F003E003E001E003C001F007C001F0078000F0078000F00F8000F80F8000F80F800FFF0F800
  FFF0FC0000007C0000007C0000007C0000007E0000003E0000003E0000601F0000601F0000600F80007007C0007007C00070
  03F000F001F800F8007C01F8003F87B8000FFF180001FC0C> 30 31 -6 0 35] 71 @dc
[<FFF80FC0FFF83FF00F803E7807807E3807807C1807C07C0003C07C0003C03C0003C03C0003E03C0001E03C0001E01C0001E0
  1C0001F03C0000F0780000FFF80000FFFE0000F81F00007807C0007803E0007801F0007C01F0003C00F8003C00F8003C00F8
  003E00F8001E00F8001E00F0001E03E001FFFFC001FFFF00> 29 31 -3 0 33] 82 @dc
cmbx10.432 @sf
[<00FFC00007FFF8001FFFFC003F83FF007F01FF807F80FF80FFC0FFC0FFC07FC0FFC07FE0FFC07FE0FFC07FE07F807FE07F80
  7FE01E007FE000007FE000007FC00000FFC00000FF800001FF000003FE0000FFF80000FFC00000FFF0000007F8000003FC00
  0001FE000F81FF001FC0FF003FE0FF803FE0FF803FE0FF803FE0FF803FE0FF803FC0FF801F81FF000FC3FE0007FFFC0003FF
  F80000FFC000> 27 39 -3 0 34] 51 @dc
3 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
295 195 p (The) s
11 r (second) s
11 r 107 c
(ey) s
9 r (pro) s
1 r (cess) s
11 r (is) s
10 r (the) s
cmti10.329 @sf
11 r (QMGR) s
cmr10.329 @sf
44 c
10 r (or) s
10 r (queue) s
12 r (manager,) s
9 r (whic) s
104 c
10 r (sc) s
(hedules) s
224 252 p (all) s
11 r (op) s
1 r (erations) s
12 r (within) s
11 r (PP) s
-3 r 46 c
11 r (The) s
12 r (QMGR) s
13 r (holds) s
12 r (the) s
12 r (full) s
12 r (queue) s
13 r (status) s
11 r (in) s
12 r (memory) s
-4 r 44 c
224 308 p (and) s
21 r (con) s
(trols) s
18 r (the) s
21 r (pro) s
1 r (cesses) s
21 r (whic) s
104 c
19 r 112 c
1 r (erform) s
20 r (the) s
20 r 119 c
(ork.) s
34 r (By) s
21 r (cen) s
(tralising) s
18 r (this) s
224 364 p (informati) s
-1 r (on) s
9 r (in) s
11 r 97 c
11 r (single) s
10 r (pro) s
1 r (cess,) s
12 r 97 c
11 r (range) s
10 r (of) s
11 r (useful) s
11 r (sc) s
(heduling) s
10 r (and) s
11 r (managemen) s
-1 r 116 c
224 421 p (options) s
17 r (are) s
17 r (made) s
16 r 97 c
118 c
-2 r (ai) s
-1 r (labl) s
-1 r (e,) s
16 r (and) s
18 r 112 c
1 r (erformance) s
16 r (is) s
17 r (also) s
16 r (impro) s
-1 r 118 c
-1 r (ed.) s
24 r (The) s
18 r (ma) s
3 r (jor) s
224 477 p (comp) s
1 r (onen) s
(ts) s
17 r (con) s
(troll) s
-1 r (ed) s
18 r 98 c
121 c
17 r (the) s
19 r (QMGR) s
19 r (are) s
18 r 99 c
(hannels,) s
19 r (whic) s
104 c
17 r 112 c
1 r (erform) s
18 r (one) s
19 r (of) s
224 534 p 116 c
119 c
-1 r 111 c
13 r (functions:) s
280 640 p (1.) s
22 r (Deliv) s
(ery) s
15 r (of) s
16 r 97 c
17 r (message,) s
15 r (either) s
17 r (lo) s
1 r (cally) s
15 r (or) s
17 r 98 c
121 c
15 r (use) s
18 r (of) s
16 r 97 c
17 r (Message) s
16 r 84 c
-3 r (ransfer) s
338 696 p (Proto) s
1 r (col) s
14 r (\(e.g.) s
19 r (the) s
15 r (X.400) s
14 r (P1\).) s
280 790 p (2.) s
22 r (Manipulation) s
18 r (of) s
20 r (the) s
20 r (message) s
18 r (within) s
19 r (the) s
20 r (queue) s
21 r (\(e.g.) s
33 r (to) s
20 r (carry) s
19 r (out) s
20 r 97 c
338 846 p (format) s
13 r (con) s
118 c
-1 r (ersion\)) s
-1 r 46 c
295 952 p 70 c
-3 r (or) s
10 r 97 c
12 r (giv) s
(en) s
10 r (MT) s
-3 r (A,) s
11 r (there) s
12 r (is) s
11 r 97 c
12 r (single) s
11 r (queue) s
13 r (and) s
12 r 97 c
12 r (single) s
11 r (QMGR.) s
12 r (There) s
12 r (ma) s
121 c
224 1008 p 98 c
1 r 101 c
13 r 109 c
(ul) s
-1 r (tiple) s
10 r (instances) s
12 r (of) s
12 r (the) s
12 r (Submit) s
11 r (pro) s
1 r (cess) s
12 r (acting) s
12 r (at) s
11 r (an) s
121 c
11 r (one) s
12 r (time.) s
17 r (Channel) s
224 1065 p (pro) s
1 r (cesses) s
16 r (and) s
16 r (UA) s
16 r (pro) s
1 r (cesses) s
17 r (are) s
15 r (generic) s
16 r (\(i.e.) s
21 r (there) s
15 r (are) s
16 r (di\013eren) s
116 c
14 r 116 c
(yp) s
1 r (es\),) s
14 r (and) s
224 1121 p (there) s
15 r (ma) s
121 c
13 r 98 c
1 r 101 c
16 r 109 c
-1 r (ultipl) s
-1 r 101 c
14 r (instances) s
15 r (of) s
15 r (an) s
121 c
13 r (sp) s
1 r (eci\014c) s
16 r 99 c
(hannel) s
15 r (or) s
15 r (UA.) s
cmbx10.432 @sf
224 1265 p 51 c
69 r (Queue) s
22 r (Structure) s
cmr10.329 @sf
224 1366 p 70 c
-3 r (or) s
14 r (eac) s
104 c
14 r (message) s
14 r (in) s
15 r (the) s
15 r (queue,) s
16 r (there) s
15 r (are) s
15 r 116 c
119 c
-1 r 111 c
13 r (comp) s
1 r (onen) s
(ts:) s
280 1472 p (1.) s
22 r 65 c
17 r (\014le) s
18 r (con) s
(taining) s
15 r (the) s
18 r (con) s
(trol) s
15 r (informat) s
-1 r (ion) s
15 r (for) s
17 r (pro) s
1 r (cessing) s
18 r (the) s
17 r (message.) s
338 1528 p (This) s
15 r (roughly) s
14 r (corresp) s
1 r (onds) s
16 r (to) s
14 r (the) s
15 r (en) s
118 c
(elop) s
1 r 101 c
14 r (informat) s
-1 r (ion) s
13 r (of) s
15 r (X.400.) s
280 1622 p (2.) s
22 r 65 c
21 r (directory) s
21 r (con) s
(taining) s
19 r (the) s
22 r (message) s
20 r (as) s
21 r (it) s
21 r (arriv) s
(ed) s
20 r (and) s
21 r (zero) s
22 r (or) s
21 r (more) s
338 1678 p (pro) s
1 r (cessed) s
15 r (forms) s
13 r (of) s
14 r (the) s
14 r (message,) s
13 r (together) s
14 r (with) s
13 r (an) s
121 c
13 r (asso) s
1 r (ciated) s
14 r (deliv) s
(ery) s
338 1735 p (noti\014cations.) s
295 1841 p (The) s
13 r (con) s
(trol) s
11 r (\014le) s
13 r (is) s
13 r (text) s
13 r (enco) s
1 r (ded,) s
15 r (as) s
12 r (there) s
14 r (is) s
13 r 97 c
13 r (big) s
12 r (adv) s
-2 r (an) s
(tage) s
11 r (in) s
13 r 98 c
1 r (eing) s
14 r (able) s
224 1897 p (to) s
17 r (view) s
17 r (and) s
17 r (manipulate) s
16 r 97 c
17 r (queued) s
18 r (message) s
16 r 98 c
121 c
17 r (use) s
17 r (of) s
17 r 97 c
17 r (text) s
17 r (editor.) s
26 r (Whilst) s
224 1954 p (this) s
18 r (should) s
18 r (de\014nitely) s
18 r (not) s
17 r 98 c
1 r 101 c
19 r 97 c
18 r (normal) s
16 r (ev) s
(en) s
(t) s
-1 r 44 c
17 r (it) s
17 r (is) s
18 r (hard) s
18 r (to) s
17 r (design) s
18 r (manage-) s
224 2010 p (men) s
116 c
18 r (to) s
1 r (ols) s
18 r (for) s
19 r (ev) s
(ery) s
19 r (ev) s
(en) s
(tuali) s
-1 r 116 c
-1 r 121 c
-4 r 46 c
31 r (The) s
20 r (queue) s
21 r (enco) s
1 r (ding) s
20 r (format) s
18 r (is) s
19 r (designed) s
224 2066 p (to) s
17 r 98 c
1 r 101 c
18 r (generic,) s
18 r (and) s
17 r (not) s
18 r (tied) s
17 r (to) s
17 r 97 c
17 r (sp) s
1 r (eci\014c) s
18 r (proto) s
1 r (col.) s
26 r (This) s
17 r (is) s
17 r (imp) s
1 r (ortan) s
-1 r 116 c
15 r (when) s
224 2123 p (attempting) s
13 r (to) s
14 r (supp) s
1 r (ort) s
15 r 97 c
15 r (range) s
14 r (of) s
15 r (proto) s
1 r (cols.) s
19 r (The) s
15 r 107 c
(ey) s
14 r (elemen) s
116 c
12 r (of) s
15 r (the) s
15 r (con) s
(trol) s
224 2179 p (\014le) s
19 r (is) s
18 r 97 c
18 r (single) s
17 r (line) s
18 r (of) s
18 r (informatio) s
-1 r 110 c
17 r (for) s
18 r (eac) s
104 c
17 r (recipien) s
(t.) s
28 r (The) s
19 r (more) s
17 r (imp) s
1 r (ortan) s
-1 r 116 c
224 2236 p (\(p) s
1 r (er) s
16 r (recipien) s
(t\)) s
13 r (parameters) s
13 r (are:) s
cmsy10.329 @sf
292 2342 p 15 c
cmr10.329 @sf
23 r 65 c
15 r (unique) s
16 r (\(in) s
(teger\)) s
12 r 107 c
(ey) s
-3 r 46 c
cmsy10.329 @sf
292 2435 p 15 c
cmr10.329 @sf
23 r (The) s
15 r (address) s
15 r (of) s
15 r (the) s
16 r (recipien) s
116 c
13 r (in) s
15 r (one) s
16 r (or) s
14 r (more) s
14 r (forms.) s
cmsy10.329 @sf
292 2529 p 15 c
cmr10.329 @sf
23 r (The) s
11 r (sequence) s
13 r (of) s
11 r 99 c
(hannels) s
10 r (whic) s
104 c
10 r (the) s
12 r (message) s
9 r (should) s
12 r 98 c
1 r 101 c
12 r (passed) s
11 r (through.) s
cmsy10.329 @sf
292 2623 p 15 c
cmr10.329 @sf
23 r (The) s
20 r (status) s
20 r (of) s
20 r (pro) s
1 r (cessing) s
20 r (for) s
20 r (the) s
21 r (recipien) s
(t,) s
20 r (whic) s
104 c
19 r 119 c
(ould) s
19 r (include) s
20 r (the) s
338 2679 p (format) s
-1 r (ting) s
18 r 99 c
(hannels) s
20 r (whic) s
104 c
20 r (ha) s
118 c
-1 r 101 c
19 r 98 c
1 r (een) s
22 r (applied,) s
21 r (whether) s
21 r (the) s
21 r (message) s
338 2736 p (has) s
21 r 98 c
1 r (een) s
23 r (deliv) s
(ered) s
21 r (to) s
21 r (that) s
21 r (recipien) s
(t,) s
21 r (and) s
22 r (an) s
121 c
21 r (deliv) s
(ery) s
20 r (noti\014cations) s
338 2792 p (needed.) s
960 2916 p 51 c
@eop
2 @bop0
cmbx10.432 @sf
[<FFFFF80000FFFFF80000FFFFF8000003FE00000003FE00000003FE00000003FE00000003FE00000003FE00000003FE000000
  03FE00000003FE00000003FE00000003FE00000003FE00000003FE00000003FE00000003FE00000003FFFFF80003FFFFFF00
  03FFFFFFC003FE007FE003FE001FF003FE000FF803FE0007FC03FE0007FC03FE0007FE03FE0007FE03FE0007FE03FE0007FE
  03FE0007FE03FE0007FE03FE0007FE03FE0007FC03FE0007FC03FE000FF803FE001FF003FE007FE0FFFFFFFFC0FFFFFFFF00
  FFFFFFF800> 39 41 -3 0 47] 80 @dc
/cmr10.300 @newfont
cmr10.300 @sf
[<003FC00001FFF00003F03C0007C00E000F8007001F0003003E0003807C0001807C0001807C000180F8000000F8000000F800
  0000F8000000F8000000F8000000F8000000F80001807C0001807C0003807C0003803E0003801F0007800F800F8007C01F80
  03F07B8001FFF380003FC180> 25 28 -2 0 30] 67 @dc
[<FF9FF0FF9FF01C03801C03801C03801C03801C03801C03801C03801C03801C03801C03801C03801E03801F07801F8F801DFF
  001C7E001C00001C00001C00001C00001C00001C00001C00001C00001C0000FC0000FC0000> 20 29 -1 0 23] 104 @dc
[<1F87807FEFC07C7E60F81E60F00E60F00E60F80E00780E007E0E001F8E0003FE00000E00000E00180E003C1E003C3C003FF8
  001FE000> 19 18 -1 0 21] 97 @dc
[<FF9FF0FF9FF01C03801C03801C03801C03801C03801C03801C03801C03801C03801C03801C03801E03801F07801F8F80FDFF
  00FC7E00> 20 18 -1 0 23] 110 @dc
[<07F00FF83F1C3C0E78067800F000F000F000F000FFFEFFFEF01E781E781C3E7C1FF807E0> 15 18 -1 0 18] 101 @dc
[<FF80FF801C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C00
  1C001C00FC00FC00> 9 29 -1 0 12] 108 @dc
[<003F800001FFF00003E0F80007803C000F001E001E000F003E000F803C0007807C0007C07C0007C0F80003E0F80003E0F800
  03E0F80003E0F80003E0F80003E0F80003E0F80003E0780003C07C0007C03C0007803C0007801E000F000F001E0007803C00
  03E0F80001FFF000003F8000> 27 28 -2 0 32] 79 @dc
[<07E3F00FFBF01E1F801C0F801C07801C03801C03801C03801C03801C03801C03801C03801C03801C03801C03801C0380FC1F
  80FC1F80> 20 18 -1 0 23] 117 @dc
[<07C00FE01E701C301C301C301C301C301C001C001C001C001C001C001C001C00FFE0FFE03C001C001C000C000C000C000C00
  0C00> 12 26 -1 0 16] 116 @dc
[<18FE001BFF801F87C01E03C01E01E01C01E01C00F01C00F01C00F01C00F01C00F01C00F01C00F01C01E01E03E01F87C01FFF
  801CFE001C00001C00001C00001C00001C00001C00001C00001C00001C0000FC0000FC0000> 20 29 -1 0 23] 98 @dc
[<03F0000FFC001E1E00380700780780700380F003C0F003C0F003C0F003C0F003C0F003C07003807807803807001E1E000FFC
  0003F000> 18 18 -1 0 21] 111 @dc
[<07E3F01FFBF03E1F807C0F80780780F00380F00380F00380F00380F00380F00380F00380F803807803807C07803E1F801FFF
  8007F380000380000380000380000380000380000380000380000380000380001F80001F80> 20 29 -1 0 23] 100 @dc
[<00000F0000001FC000003FC000003FC000003FE0000078E00000706000007020003FE02001FFF00003F8F80007B0FC000F39
  9E001E3F8F003C1F07803C0007807C0007C0780003C0F80003E0F80003E0F80003E0F80003E0F80003E0F80003E0F80003E0
  F80003E0780003C07C0007C03C0007803E000F801E000F000F001E0007803C0003E0F80001FFF000003F8000> 27 36 -2 8 32] 81 @dc
[<FFC387FF80FFC387FF801E038078000C07C078000C07C078000C07C078000C0F6078000C0F6078000C0F6078000C1E307800
  0C1E3078000C1E3078000C3C1878000C3C1878000C3C1878000C780C78000C780C78000C780C78000CF00678000CF0067800
  0CF00678000DE00378000DE00378000DE00378000FC001F8000FC001F800FFC001FF80FF8000FF80> 33 28 -2 0 38] 77 @dc
[<003FE18001FFFB8003F83F8007C00F800F8007801F0007803E0007807C0007807C0007807C000780F800FFF0F800FFF0F800
  0000F8000000F8000000F8000000F8000000F80001807C0001807C0003807C0003803E0003801F0007800F800F8007C01F80
  03F07B8001FFF380003FC180> 28 28 -2 0 33] 71 @dc
[<FFF01FC0FFF07FE00F00FE700F00FC300F00F8000F00F8000F00F8000F00F0000F00F0000F00F0000F01E0000F01E0000F03
  C0000FFF80000FFFE0000F01F0000F0078000F003C000F003E000F003E000F003E000F003E000F003E000F003C000F007800
  0F01F000FFFFE000FFFF0000> 28 28 -2 0 31] 82 @dc
[<C7F800FFFE00FE0F00F00780E00380E001C0C001C0C001C00001C00003C00007C0000F8000FF800FFF001FFE003FFC007FE0
  00FE0000F80000F00000E00180E00180E003807003807007803C1F801FFF8007F180> 18 28 -2 0 23] 83 @dc
[<FF8FF8FF80FF8FF8FF801C01C01C001C01C01C001C01C01C001C01C01C001C01C01C001C01C01C001C01C01C001C01C01C00
  1C01C01C001C01C01C001C01C01C001E01E01C001F03F03C001F87F87C00FDFF9FF800FC7F07F000> 33 18 -1 0 36] 109 @dc
[<FF80FF801C001C001C001C001C001C001C001C001C001C001C001C001C001C00FC00FC000000000000000000000000003C00
  7C007C007C003C00> 9 29 -1 0 12] 105 @dc
[<003F800000FFC00001F0F00003C0700007803800078018000F001C000F000C000F000C000F000C000F000C000F000C000F00
  0C000F000C000F000C000F000C000F000C000F000C000F000C000F000C000F000C000F000C000F000C000F000C000F000C00
  0F001E00FFF0FFC0FFF0FFC0> 26 28 -2 0 31] 85 @dc
[<FFC0FFF0FFC0FFF01F001F0006001E0006001E0007003E0003003C0003003C0003FFFC0001FFF8000180780001C0F80000C0
  F00000C0F00000E1F0000061E0000061E0000073E0000033C0000033C000003FC000001F8000001F8000001F8000000F0000
  000F0000000F000000060000> 28 28 -1 0 31] 65 @dc
[<FFF0FFF00F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F00
  0F00FFF0FFF0> 12 28 -1 0 15] 73 @dc
cmti10.329 @sf
[<3FC000FFF000F8F800787C00703E00001E00001F00001F00000F0007EF000FFF801E3F801C1F803C0F803C07C03C07C03C03
  C03C03C03E03E03E03E01E01E01E01E00F01F00F01F00781F003C1F001E3F800FFF8003E70> 21 29 -2 9 21] 103 @dc
[<C3FC0000EFFF0000FF0780007C01E0007800E000700070007000780030003800300038000000380000003800000078000000
  F8000003F800001FF80000FFF00001FFE00003FF800003FC000003E0000007C0000007800000038003000380030003800300
  01C0070001E0078000F00F80007C3F80003FFD800007F0C0> 26 31 -3 0 26] 83 @dc
[<1F80003FE00078F000707800F03C00F01E00F01E00F00F00F00F00F80F80780F807807807807807C07807C07803E07803F07
  003F8F003FFE001EFC001E00001F00001F00000F00000F00000F80000F80000780000780003FC0003FC00007C000> 17 32 -4 0 21] 98 @dc
[<1C03803E003E07C07F001E07C0F3801E03C0F1C01E03C0F8C01F03E078C00F03E078E00F01E07C000F01E03C000F81F03C00
  0781F03E000780F01E000780F01E00E7C0F81E0063C0F81F0063E07C0F0073F07E0F0033F8F71E003F9FF3FE001F0FC1F800> 35 20 -3 0 37] 109 @dc
2 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
cmbx10.432 @sf
224 195 p 50 c
69 r (Pro) s
2 r (cess) s
23 r (Structure) s
cmr10.329 @sf
224 297 p (The) s
19 r (ideas) s
19 r (for) s
19 r (the) s
19 r (basic) s
19 r (structure) s
19 r (dra) s
119 c
17 r (hea) s
(vily) s
17 r (on) s
19 r (the) s
19 r (MMDF) s
18 r (system) s
18 r ([5].) s
224 353 p (Extensiv) s
101 c
17 r (use) s
19 r (is) s
19 r (made) s
17 r (of) s
19 r (UNIX) s
19 r (pro) s
1 r (cesses) s
19 r (to) s
18 r (pro) s
(vide) s
17 r (mo) s
1 r (dularit) s
-1 r 121 c
-4 r 44 c
17 r (whic) s
104 c
18 r (is) s
224 409 p (fundamen) s
(tal) s
10 r (to) s
12 r (PP) s
-3 r 46 c
11 r (The) s
13 r (aim) s
10 r (has) s
12 r 98 c
1 r (een) s
14 r (to) s
12 r (mak) s
-1 r 101 c
11 r (eac) s
104 c
11 r (pro) s
1 r (cess) s
13 r 112 c
1 r (erform) s
11 r 97 c
12 r (single) s
224 466 p (function.) s
30 r (This) s
19 r (allo) s
-1 r (ws) s
17 r (for) s
18 r (\015exible) s
18 r (construction) s
18 r (of) s
19 r (complex) s
17 r (pro) s
1 r (cessing,) s
19 r (and) s
224 522 p (minim) s
-1 r (ises) s
15 r (side) s
16 r (e\013ects.) s
24 r (The) s
17 r 111 c
118 c
(era) s
-1 r (ll) s
14 r (pro) s
1 r (cess) s
17 r (structure) s
17 r (is) s
16 r (illustrated) s
15 r (in) s
16 r (\014gure) s
224 579 p (1.) s
224 630 p 1495 2 ru
2 setlinewidth
np 1524 1159 275 50 130 arc st
np 1533 1372 304 233 304 arc st
np 1528 916 275 50 130 arc st
np 1153 753 176 93 0 360 ellipse st
np 542 764 176 93 0 360 ellipse st
np 868 1034 176 93 0 360 ellipse st
np 864 1604 176 93 0 360 ellipse st
np 410 1285 176 93 0 360 ellipse st
np 731 945 p 759 959 l st
np 741 934 p 759 959 l
628 846 l st
np 656 860 p 628 846 l st
np 646 872 p 628 846 l st
np 999 930 p 980 955 l st
np 1008 942 p 980 955 l
1115 846 l st
np 1097 871 p 1115 846 l st
np 1087 859 p 1115 846 l st
np 1063 1077 p 1033 1071 l st
np 1057 1091 p 1033 1071 l
1344 1214 l st
np 1314 1208 p 1344 1214 l st
np 1320 1195 p 1344 1214 l st
np 685 1090 p 707 1067 l st
np 677 1077 p 707 1067 l
500 1199 l st
np 521 1177 p 500 1199 l st
np 530 1189 p 500 1199 l st
np 1036 1525 p 1014 1547 l st
np 1044 1538 p 1014 1547 l
1344 1349 l st
np 1322 1371 p 1344 1349 l st
np 1315 1358 p 1344 1349 l st
np 551 1369 p 523 1356 l st
np 542 1381 p 523 1356 l
751 1533 l st
np 723 1520 p 751 1533 l st
np 732 1508 p 751 1533 l st
np 620 1277 p 590 1285 l st
np 620 1292 p 590 1285 l st
[15] 0 setdash
np 590 1285 p 1344 1285 l st
[] 0 setdash
np 1314 1292 p 1344 1285 l st
np 1314 1277 p 1344 1285 l st
np 1708 1124 p 1708 1364 l st
cmr10.300 @sf
789 1653 p (Channel) s
778 1593 p (Outb) s
1 r (ound) s
1412 1271 p (Queue) s
336 1312 p (QMGR) s
785 1064 p (Submit) s
1082 779 p (UA) s
459 824 p (Channel) s
444 764 p (In) s
98 c
1 r (ound) s
np 1348 1128 p 1348 1367 l st
cmr10.329 @sf
658 1802 p (Figure) s
14 r (1:) s
20 r (PP) s
15 r (Pro) s
1 r (cess) s
15 r (Structure) s
224 1821 p 1495 2 ru
295 1927 p (PP) s
15 r (has) s
15 r 97 c
cmti10.329 @sf
15 r (single) s
cmr10.329 @sf
13 r (queue.) s
21 r (This) s
15 r (is) s
15 r (imp) s
1 r (ortan) s
-2 r 116 c
13 r (for) s
15 r (the) s
15 r (follo) s
(w) s
-1 r (ing) s
13 r (reasons:) s
280 2033 p (1.) s
22 r (It) s
15 r (increases) s
15 r (robustness.) s
280 2127 p (2.) s
22 r (It) s
15 r (facilitates) s
13 r (uniform) s
14 r (handling) s
15 r (of) s
14 r (errors.) s
280 2221 p (3.) s
22 r (It) s
15 r (simpli\014es) s
13 r (managemen) s
-1 r (t.) s
295 2327 p (There) s
18 r (are) s
17 r 116 c
119 c
-1 r 111 c
16 r (critical) s
16 r (pro) s
1 r (cesses) s
18 r (with) s
17 r (complex) s
17 r (functionalit) s
-1 r 121 c
-4 r 46 c
26 r (All) s
17 r (of) s
17 r (the) s
224 2383 p (others) s
18 r (are) s
19 r (\\dum) s
(b".) s
28 r (The) s
19 r (\014rst) s
19 r 107 c
(ey) s
17 r (pro) s
1 r (cess) s
19 r (in) s
19 r (PP) s
19 r (is) s
cmti10.329 @sf
18 r (Submit) s
cmr10.329 @sf
46 c
31 r (Submit) s
17 r (tak) s
(es) s
224 2440 p (an) s
18 r (incoming) s
16 r (message,) s
17 r (and) s
18 r (places) s
17 r (it) s
17 r (in) s
18 r (the) s
18 r (queue.) s
29 r (Submit) s
16 r (will) s
17 r 112 c
1 r (erform) s
17 r (all) s
224 2496 p (calculations) s
16 r (of) s
17 r (the) s
17 r (pro) s
1 r (cessing) s
17 r (required) s
17 r (for) s
17 r 97 c
17 r (giv) s
(en) s
15 r (message,) s
16 r (and) s
17 r (determine) s
224 2553 p (ho) s
119 c
14 r (it) s
15 r (will) s
14 r 98 c
1 r 101 c
16 r (deliv) s
(ered.) s
20 r (Early) s
15 r 99 c
(hec) s
(king) s
13 r (ensures) s
16 r (that) s
15 r (errors) s
15 r (are) s
15 r (detected) s
16 r (at) s
224 2609 p (the) s
18 r (earliest) s
17 r 112 c
1 r (oin) s
(t,) s
17 r (and) s
18 r (that) s
17 r (later) s
17 r (pro) s
1 r (cesses) s
19 r (can) s
18 r 98 c
1 r 101 c
19 r (implem) s
-1 r (en) s
-1 r (ted) s
16 r (in) s
18 r 97 c
18 r (more) s
224 2666 p (straigh) s
(t) s
-1 r (forw) s
-2 r (ard) s
15 r (fashion.) s
24 r (The) s
17 r (initial) s
15 r 99 c
(hec) s
(king) s
14 r (includes) s
18 r (all) s
15 r (managemen) s
-1 r 116 c
14 r (and) s
224 2722 p (address) s
19 r 118 c
(eri\014catio) s
-1 r (n.) s
28 r (Messages) s
18 r (ma) s
121 c
16 r 98 c
1 r 101 c
19 r (passed) s
19 r (to) s
18 r (Submit) s
18 r (either) s
18 r 98 c
121 c
17 r 97 c
18 r (User) s
224 2779 p (Agen) s
(t,) s
14 r (or) s
14 r 98 c
121 c
14 r 97 c
15 r (proto) s
1 r (col) s
14 r (serv) s
(er) s
14 r 124 c
16 r (kno) s
(wn) s
14 r (as) s
15 r (an) s
15 r (in) s
98 c
1 r (ound) s
15 r 99 c
(hannel.) s
960 2916 p 50 c
@eop
1 @bop0
cmr8.300 @sf
[<7FF07FF0070007000700070007000700070007000700070007000700070007000700F700FF000F000300> 12 21 -2 0 18] 49 @dc
/cmss10.329 @newfont
cmss10.329 @sf
[<00FE0007FF800FFFC01F83E03E00F07C00F87C007878003CF8003CF8003CF8003CF8003CF8003CF8003CF8003CF8003CF800
  3CF8003CF8003CF8003CF8003CF8003CF8003CF8003CF8003CF8003CF8003CF8003CF8003CF8003CF8003CF8003C> 22 32 -4 0 31] 85 @dc
[<F000FEF001FEF001FEF003FEF003DEF003DEF0079EF0079EF00F9EF00F1EF01F1EF01F1EF01E1EF03E1EF03C1EF07C1EF07C
  1EF0781EF0F81EF0F01EF1F01EF1F01EF1E01EF3E01EF3C01EF3C01EF7801EF7801EFF801EFF001EFF001EFE001E> 23 32 -4 0 32] 78 @dc
[<F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8F8> 5 32 -4 0 13] 73 @dc
[<F80007E0780007C07C000FC03E001F801E001F001F003E000F803E0007807C0007C0F80003E0F80001E1F00001F1E00000F3
  C000007FC000007F8000003F0000001F0000001F0000003F0000007F800000FFC00000FBC00001F3E00003E1F00003E0F000
  07C0F8000F807C001F803C001F003E003F001F007E000F007C000F80> 27 32 -1 0 30] 88 @dc
cmr8.300 @sf
[<FFF0FFF0FFF0703838181C180E00078003C001E001F000F000F800787078F878F878F8F8F1F07FE01F80> 13 21 -2 0 18] 50 @dc
cmr6.300 @sf
[<FF80FF801C001C001C001C001C001C001C001C001C001C009C00FC007C000C00> 9 16 -2 0 15] 49 @dc
cmr9.300 @sf
[<0FF03FFC7C3EF00FE007E007E00F701F3FFE3FFC3FF8300030003FC03FE03CF0787878787878787878783CF71FFF0FCF> 16 24 -1 8 19] 103 @dc
[<FF80FF801C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C001C00FC00
  FC00> 9 26 0 0 10] 108 @dc
[<01C00001C00001C00003E00003E00007F0000730000730000E38000E18000E18001C0C001C0C003C0E00FF3F80FF3F80> 17 16 -1 0 20] 118 @dc
[<FFF000FFF0000F00000F00000F00000F00000F00000F00000F00000F00000F00000FFF800FFFE00F00F00F00780F00380F00
  3C0F003C0F003C0F003C0F003C0F00380F00780F00F0FFFFE0FFFF80> 22 26 -1 0 26] 80 @dc
[<70F8F8F87000000000707070707070707070F8F8F8F8F8F8F870> 5 26 -3 0 11] 33 @dc
cmr6.300 @sf
[<FFC0FFC07FC0306018600E000700038001C001E000E0E0E0E0E0E3E07FC03F00> 11 16 -1 0 15] 50 @dc
/cmss9.300 @newfont
cmss9.300 @sf
[<03F8000FFE001FFF003E0F807C07C07803C0F001E0F001E0F001E0F001E0F001E0F001E0F001E0F001E0F001E0F001E0F001
  E0F001E0F001E0F001E0F001E0F001E0F001E0F001E0F001E0F001E0> 19 26 -3 0 26] 85 @dc
[<F003F0F007F0F007F0F00EF0F00EF0F01EF0F01CF0F01CF0F03CF0F038F0F078F0F070F0F0F0F0F0F0F0F0E0F0F1E0F0F1C0
  F0F3C0F0F380F0F380F0F780F0F700F0F700F0FE00F0FE00F0FC00F0> 20 26 -3 0 27] 78 @dc
[<F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0> 4 26 -3 0 11] 73 @dc
[<F0003E78003C7C007C3C00F81E00F01F01E00F03E00783C007C78003CF0001FF0000FE0000FC00007800007C0000FC0001FE
  0003FF0003CF800787800F83C01F03E01E01E03E00F07C00F8F8007C> 23 26 -1 0 26] 88 @dc
cmr9.300 @sf
[<FF81FFC0FF81FFC01E003C000E003C0006007800060078000600780003FFF00003FFF0000300F0000181E0000181E0000181
  E00000C3C00000C3C00000C3C000006780000067800000778000003F0000003F0000003F0000001E0000001E0000001E0000
  000C0000> 26 26 -1 0 29] 65 @dc
[<0FC0FC003FF3FF007C7FC300F81F0180F83F0180F07F0000F07B8000F0F1C000F1E0C00079E0E0003FC070001F8030000F80
  380007803C0007C0FF800FE0FF800EF000000E7000000E3800000E1800000E1800000E1800000E1800000738000003F00000
  01E00000> 25 26 -2 0 30] 38 @dc
[<FFFFE0FFFFE00F03E00F00E00F00600F00700F00700F00300F00300F00300F00000F00000F00000F00000F00000F00000F00
  000F00000F00000F00000F00000F00000F00000F0000FFF800FFF800> 20 26 -1 0 24] 76 @dc
1 @bop1
cmr10.329 @sf
1213 50 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
cmbx10.432 @sf
224 195 p 49 c
69 r (In) s
-1 r (tro) s
2 r (duction) s
cmr10.329 @sf
224 297 p (This) s
20 r (pap) s
1 r (er) s
21 r (describ) s
1 r (es) s
21 r (the) s
21 r (design) s
21 r (and) s
20 r (implemen) s
-1 r (ta) s
-1 r (tio) s
-1 r 110 c
19 r (of) s
20 r (PP) s
cmr8.300 @sf
1457 282 p 49 c
cmr10.329 @sf
1476 297 p 44 c
20 r (whic) s
104 c
20 r (is) s
20 r 97 c
224 353 p (Message) s
18 r 84 c
-3 r (ransfer) s
17 r (Agen) s
116 c
17 r (orien) s
(ted) s
17 r (to) s
119 c
-1 r (ards) s
16 r (supp) s
1 r (ort) s
19 r (of) s
18 r (the) s
19 r (X.400) s
17 r (series) s
18 r (rec-) s
224 409 p (ommendati) s
-1 r (ons) s
10 r ([2],) s
11 r ([3].) s
18 r (PP) s
12 r (is) s
11 r (implemen) s
-1 r (ted) s
10 r (to) s
11 r (run) s
13 r (under) s
12 r (the) s
cmss10.329 @sf
12 r (UNIX) s
cmr8.300 @sf
1498 394 p 50 c
cmr10.329 @sf
1530 409 p (op) s
1 r (erating) s
224 466 p (system.) s
22 r (Although) s
16 r (the) s
17 r (implem) s
-1 r (en) s
-1 r (tati) s
-1 r (on) s
15 r (of) s
16 r (an) s
16 r (X.400) s
15 r (MT) s
-3 r 65 c
15 r (is) s
16 r (no) s
119 c
15 r 97 c
16 r (far) s
16 r (from) s
224 522 p (un) s
(usual) s
14 r (ev) s
(en) s
(t,) s
13 r 97 c
15 r (pap) s
1 r (er) s
15 r (is) s
15 r (justi\014ed) s
15 r 98 c
1 r (ecause) s
17 r (of) s
14 r (the) s
16 r (far) s
14 r (reac) s
(hing) s
14 r (goals) s
14 r (for) s
14 r (PP) s
-3 r 46 c
295 579 p (There) s
11 r (has) s
11 r 98 c
1 r (een) s
12 r (extensiv) s
101 c
10 r (exp) s
1 r (erience) s
12 r (with) s
11 r 97 c
11 r 110 c
(um) s
-2 r 98 c
1 r (er) s
10 r (of) s
11 r (sophisticated) s
10 r (text-) s
224 635 p (based) s
17 r (Message) s
15 r 84 c
-3 r (ransfer) s
15 r (Systems,) s
15 r (usually) s
16 r (based) s
17 r (on) s
16 r (the) s
17 r (RF) s
67 c
14 r (822) s
16 r (standards) s
224 692 p ([4],) s
15 r ([5],) s
16 r ([6],) s
15 r ([7].) s
22 r (Man) s
121 c
15 r (emerging) s
15 r (X.400) s
15 r (systems,) s
15 r (whilst) s
15 r (o\013ering) s
15 r (the) s
17 r (sup) s
1 r (erior) s
224 748 p (X.400) s
14 r (proto) s
1 r (cols,) s
14 r (in) s
15 r (man) s
-1 r 121 c
14 r (resp) s
1 r (ects) s
15 r (o\013er) s
14 r 97 c
15 r 112 c
1 r 111 c
1 r (orer) s
15 r (service) s
16 r (than) s
15 r (the) s
15 r (older) s
15 r (sys-) s
224 805 p (tems.) s
23 r (PP) s
16 r 119 c
(as) s
14 r (ev) s
(olv) s
-1 r (ed) s
15 r (when) s
17 r 97 c
16 r (group) s
17 r (of) s
16 r (us) s
16 r (aimed) s
15 r (to) s
16 r (dra) s
119 c
15 r (on) s
16 r (the) s
17 r (results) s
16 r (of) s
224 861 p (earlier) s
15 r 119 c
(ork) s
13 r (and) s
16 r (create) s
15 r 97 c
15 r (Message) s
15 r 84 c
-3 r (ransfer) s
14 r (System) s
14 r (with) s
15 r (high) s
15 r (functionalit) s
121 c
-4 r 46 c
224 918 p (The) s
16 r (ma) s
3 r (jo) s
-1 r 114 c
13 r (goals) s
14 r (of) s
15 r (PP) s
15 r (are:) s
cmsy10.329 @sf
292 1024 p 15 c
cmr10.329 @sf
23 r (Pro) s
(visi) s
-1 r (on) s
14 r (of) s
15 r 97 c
16 r (clean) s
16 r (in) s
(terface) s
14 r (for) s
15 r (message) s
14 r (submission) s
15 r (and) s
16 r (deliv) s
(ery) s
-4 r 44 c
14 r (so) s
338 1080 p (that) s
14 r 97 c
15 r (wide) s
15 r (range) s
15 r (of) s
15 r (User) s
15 r (Agen) s
(ts) s
14 r (ma) s
-1 r 121 c
13 r 98 c
1 r 101 c
16 r (supp) s
1 r (orted.) s
cmsy10.329 @sf
292 1174 p 15 c
cmr10.329 @sf
23 r (Supp) s
1 r (ort) s
15 r (for) s
15 r (all) s
14 r (of) s
15 r (the) s
16 r (Message) s
14 r 84 c
-3 r (ransfer) s
14 r (Service) s
15 r (Elemen) s
(ts) s
13 r (sp) s
1 r (eci\014ed) s
17 r 98 c
121 c
338 1231 p (the) s
15 r (1984) s
14 r (and) s
16 r (1988) s
14 r 118 c
(ersions) s
13 r (of) s
15 r (X.400.) s
cmsy10.329 @sf
292 1324 p 15 c
cmr10.329 @sf
23 r (Supp) s
1 r (ort) s
15 r (for) s
15 r (some) s
14 r (services) s
15 r (not) s
15 r (pro) s
(vided) s
14 r 98 c
121 c
14 r (X.400.) s
cmsy10.329 @sf
292 1418 p 15 c
cmr10.329 @sf
23 r (Sp) s
1 r (eci\014c) s
16 r (supp) s
1 r (ort) s
15 r (to) s
15 r (facilitate) s
13 r (the) s
15 r (handling) s
15 r (of) s
15 r 109 c
(ult) s
-1 r (im) s
-1 r (edia) s
13 r (messages.) s
cmsy10.329 @sf
292 1512 p 15 c
cmr10.329 @sf
23 r (Robustness) s
16 r (and) s
15 r (e\016ciency) s
16 r (required) s
15 r (to) s
14 r (supp) s
1 r (ort) s
16 r (high) s
15 r (lev) s
(els) s
13 r (of) s
15 r (tra\016c.) s
cmsy10.329 @sf
292 1606 p 15 c
cmr10.329 @sf
23 r (Sc) s
(heduling) s
13 r (of) s
14 r (message) s
13 r (deliv) s
(ery) s
13 r (in) s
14 r 97 c
14 r (sophisticated) s
13 r (manner,) s
13 r (to) s
14 r (optimi) s
-1 r (se) s
338 1662 p (lo) s
1 r (cal) s
14 r (and) s
15 r (comm) s
-1 r (unicati) s
-1 r (on) s
13 r (resources.) s
cmsy10.329 @sf
292 1756 p 15 c
cmr10.329 @sf
23 r (Pro) s
(visi) s
-1 r (on) s
20 r (of) s
21 r 97 c
21 r (range) s
21 r (of) s
21 r (managemen) s
-1 r (t) s
-1 r 44 c
21 r (authorisation) s
20 r (and) s
21 r (monitoring) s
338 1813 p (facilities.) s
cmsy10.329 @sf
292 1906 p 15 c
cmr10.329 @sf
23 r (Reformatti) s
-1 r (ng) s
14 r 98 c
1 r (et) s
119 c
-1 r (een) s
14 r 98 c
1 r 111 c
1 r (dy) s
16 r (part) s
15 r 116 c
(yp) s
1 r (es) s
14 r (in) s
15 r 97 c
15 r (general) s
14 r (manner.) s
cmsy10.329 @sf
292 2000 p 15 c
cmr10.329 @sf
23 r (Supp) s
1 r (ort) s
15 r (for) s
15 r 109 c
(ul) s
-1 r (tiple) s
13 r (address) s
15 r (formats) s
13 r (\(initiall) s
-1 r 121 c
14 r 116 c
119 c
-1 r (o) s
-1 r (\).) s
cmsy10.329 @sf
292 2094 p 15 c
cmr10.329 @sf
23 r (Use) s
15 r (of) s
15 r (standardised) s
15 r (OSI) s
16 r (Directory) s
14 r (Services) s
15 r ([8].) s
cmsy10.329 @sf
292 2188 p 15 c
cmr10.329 @sf
23 r (Supp) s
1 r (ort) s
15 r (for) s
15 r (message) s
14 r (proto) s
1 r (col) s
14 r (con) s
118 c
(ers) s
-1 r (ion) s
13 r (in) s
15 r (an) s
15 r (in) s
(tegrated) s
13 r (fashion.) s
cmsy10.329 @sf
292 2282 p 15 c
cmr10.329 @sf
23 r 70 c
-3 r (ull) s
11 r (access) s
13 r (to) s
12 r (the) s
13 r (Message) s
12 r 84 c
-3 r (ransfer) s
11 r (Service) s
13 r (for) s
12 r (applications) s
11 r (other) s
13 r (than) s
338 2338 p (In) s
(ter-P) s
(erso) s
-1 r (nal) s
13 r (Messaging.) s
295 2444 p (The) s
16 r (rest) s
16 r (of) s
15 r (this) s
16 r (pap) s
1 r (er) s
16 r (then) s
17 r (discusses) s
16 r (the) s
16 r (features) s
16 r (of) s
15 r (PP) s
16 r (in) s
16 r (more) s
15 r (detail,) s
224 2501 p (and) s
16 r (explains) s
14 r (ho) s
119 c
14 r (they) s
15 r (are) s
15 r (used) s
16 r (to) s
14 r (ac) s
(hiev) s
101 c
13 r (the) s
16 r (listed) s
14 r (goals.) s
224 2540 p 598 2 ru
cmr6.300 @sf
276 2568 p 49 c
cmr9.300 @sf
293 2583 p (In) s
13 r (this) s
14 r (age) s
13 r (of) s
13 r (acron) s
(yms) s
13 r (it) s
13 r (will) s
15 r 98 c
1 r 101 c
13 r 97 c
13 r (surprise) s
15 r (to) s
13 r (disco) s
118 c
(er) s
12 r (that) s
13 r (PP) s
13 r (is) s
14 r (not) s
13 r (an) s
14 r (acron) s
(ym!) s
cmr6.300 @sf
276 2614 p 50 c
cmss9.300 @sf
293 2629 p (UNIX) s
cmr9.300 @sf
13 r (is) s
14 r 97 c
13 r (trademark) s
13 r (of) s
13 r 65 c
-2 r (T&T) s
11 r (Bell) s
14 r (Lab) s
1 r (oratories.) s
cmr10.329 @sf
960 2916 p 49 c
@eop
0 @bop0
/cmr10.432 @newfont
cmr10.432 @sf
[<FFFFC00000FFFFC0000007F800000003F000000003F000000003F000000003F000000003F000000003F000000003F0000000
  03F000000003F000000003F000000003F000000003F000000003F000000003F000000003F000000003F000000003FFFF8000
  03FFFFF00003F001FC0003F0007E0003F0003F0003F0001F8003F0000FC003F0000FC003F0000FE003F0000FE003F0000FE0
  03F0000FE003F0000FE003F0000FE003F0000FC003F0000FC003F0001F8003F0003F0003F0007E0007F001FC00FFFFFFF000
  FFFFFF8000> 35 41 -2 0 41] 80 @dc
[<FFFFFFFFFFFFFFE0FFFFFFFFFFFFFFE0> 59 2 0 -15 60] 124 @dc
[<FFF801FFFFFFF801FFFF1FC0001FE00780000FC00380000FC00380001FC00180001F800180001F8001C0003F8000C0003F00
  00C0003F000060007E000060007E00007FFFFE00003FFFFC00003000FC00003000FC00001801F800001801F800001801F800
  000C03F000000C03F000000C03F000000607E000000607E000000607E00000030FC00000030FC00000030FC00000019F8000
  00019F80000001BF80000000FF00000000FF00000000FF000000007E000000007E000000007E000000003C000000003C0000
  00003C00000000180000> 40 42 -2 0 45] 65 @dc
[<FFFC0300FFFFC0FFFC0780FFFFC00FC0078007F8000780078003F00003000FC003F00003000FC003F00003000FC003F00003
  001F6003F00003001F6003F00003001F6003F00003003E3003F00003003E3003F00003007C1803F00003007C1803F0000300
  7C1803F0000300F80C03F0000300F80C03F0000300F80C03F0000301F00603F0000301F00603F0000301F00603F0000303E0
  0303F0000303E00303F0000307C00183F0000307C00183F0000307C00183F000030F8000C3F000030F8000C3F000030F8000
  C3F000031F000063F000031F000063F000033E000033F000033E000033F000033E000033F000037C00001BF000037C00001B
  F000037C00001BF00003F800000FF00007F800000FF800FFF800000FFFC0FFF0000007FFC0> 50 41 -2 0 55] 77 @dc
[<007F0003FFE007F0F00FC0381F00183F001C7E000C7E00007C0000FC0000FC0000FC0000FC0000FC0000FFFFFCFFFFFCFC00
  7C7C007C7C007C7E00F83E00F81F01F81F81F007C7E003FFC000FF00> 22 26 -2 0 27] 101 @dc
[<C3FC00EFFF00FE0F80F803C0F003C0F001E0E001E0E001E0C003E0C007E0001FE003FFC00FFFC03FFF807FFF007FFC00FFC0
  00FC0000F800C0F000C0F000C0F001C07803C03E0FC01FFFC007F8C0> 19 26 -2 0 24] 115 @dc
[<07F81F001FFE7F803F8F7FC07E03F860FE01F860FC01F860FC00F860FC00F860FC00F860FE00F8007F00F8003F80F8001FE0
  F8000FFCF80001FFF800001FF8000000F8000000F8000000F8001E00F8003F01F8003F01F0003F03F0003F07E0001FFF8000
  07FE0000> 27 26 -2 0 30] 97 @dc
[<00FF800007FFF0001FC1FC003E003E007C001F00F8000F80F0000780F0000780F0000780F0000780F8000F8078001F803E00
  7F001FFFFF000FFFFE001FFFF8001FFFE0003E00000038000000380000003800000039FE00001FFF80001F87C0001F03E000
  3F03F0003E01F0007E01F8007E01F8007E01F8007E01F8007E01F8007E01F8003E01F0003F03F1801F03E3C00F87F3C007FF
  FBC001FE3FC000000F80> 26 40 -2 13 30] 103 @dc
[<01FFFFFE0001FFFFFE000001FE00000000FC00000000FC00000000FC00000000FC00000000FC00000000FC00000000FC0000
  0000FC00000000FC00000000FC00000000FC00000000FC00000000FC00000000FC00000000FC00000000FC00000000FC0000
  0000FC00000000FC00000000FC00000000FC00000000FC00000000FC00000000FC0000C000FC000CC000FC000CC000FC000C
  C000FC000CC000FC000CE000FC001CE000FC001C6000FC00186000FC00187000FC00387800FC00787E00FC01F87FFFFFFFF8
  7FFFFFFFF8> 38 41 -2 0 43] 84 @dc
[<FFFF00FFFF0007E00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C0
  0007C00007C00007C1E007E3F007E3F00FF3F0FFFBF0FFFFE007CF80> 20 26 -1 0 23] 114 @dc
[<FFFE7FFCFFFE7FFC07C00F8007C00F8007C00F8007C00F8007C00F8007C00F8007C00F8007C00F8007C00F8007C00F8007C0
  0F8007C00F8007C00F8007C00F8007C00F8007C00F8007C00F8007E00F8007E00F8007F00F800FF81F00FFDE3F00FFCFFE00
  07C3F800> 30 26 -1 0 33] 110 @dc
[<FFFF00FFFF0007E00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C00007C0
  0007C00007C00007C00007C00007C00007C00007C000FFFE00FFFE0007C00007C00007C00007C00007C00007C00007C00007
  C00007C0C007C3F003E3F003E3F001F3F000F9F0007FE0000FC0> 20 42 -1 0 18] 102 @dc
[<007E0001FF0003F38003E18007C1C007C0C007C0C007C0C007C0C007C0C007C0C007C00007C00007C00007C00007C00007C0
  0007C00007C00007C00007C00007C00007C00007C000FFFF80FFFF801FC0000FC00007C00003C00001C00001C00001C00000
  C00000C00000C00000C000> 18 37 -1 0 23] 116 @dc
/cmr10.360 @newfont
cmr10.360 @sf
[<C1FE00EFFF80FF03C0FC01E0F000F0E000F0E00078C00078C00078C000780000780000F80001F80003F8000FF001FFF007FF
  E01FFFC03FFF807FFE007FE000FE0000FC0000F80000F80030F00030F00070F000707800F07800F03C03F01E0FF00FFF7003
  F830> 21 34 -3 0 28] 83 @dc
[<78FCFCFCFC78> 6 6 -4 0 14] 46 @dc
[<FFFFFFF0FFFFFFF007C003F007C000F807C0007807C0003807C0001807C0001807C0001807C0001807C0000C07C0300C07C0
  300C07C0300007C0300007C0700007C0F00007FFF00007FFF00007C0F00007C0700007C0300007C0303007C0303007C03030
  07C0003007C0007007C0007007C0006007C000E007C001E007C007E0FFFFFFE0FFFFFFE0> 30 34 -2 0 34] 69 @dc
[<FFFE07FFC0FFFE07FFC007C001FE0007C001F80007C001F80007C003F00007C003F00007C007E00007C00FC00007C00FC000
  07C01F800007C03F000007C03F000007E07E000007F07C000007F8FC000007DDF8000007CFF8000007C7F0000007C3E00000
  07C1E0000007C0E0000007C070000007C038000007C03C000007C01E000007C00F000007C007800007C003C00007C001E000
  07C000F00007C001FC00FFFE07FF80FFFE07FF80> 34 34 -2 0 39] 75 @dc
[<FFE0FFE00F000F000F000F000F000F000F000F000F000F000F000F000F000F000F001F00FF00FF000F000000000000000000
  0000000000001E003F003F003F003F001E00> 11 34 0 0 13] 105 @dc
[<FFF0FFF00F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F000F00
  0F000F000F000F000F000F001F00FF00FF000F00> 12 35 0 0 13] 108 @dc
[<01FC0007FF000FC3C03F01C03E00E07C00607C0000F80000F80000F80000F80000F80000FFFFE0FFFFE07801E07C03E03C03
  C03E07C01F0F8007FF0001FC00> 19 21 -1 0 22] 101 @dc
/cmbx10.329 @newfont
cmbx10.329 @sf
[<FFF01FFFE0FFF01FFFE0FFF01FFFE0070001FC00070001FC00038003F800038003F80003C007F80001FFFFF00001FFFFF000
  00FFFFE00000E00FE00000F01FE00000701FC00000703FC00000383F800000383F8000003C7F8000001C7F0000001CFF0000
  000EFE0000000EFE0000000FFE00000007FC00000007FC00000003F800000003F800000003F800000001F000000001F00000
  0000E00000> 35 31 -2 0 40] 65 @dc
[<1C1FE0001E7FF8001FF0FC001FC07E001F803F001F801F001F801F801F801F801F801F801F801F801F801F801F801F801F80
  1F801F801F801F801F001F803F001FC03E001FF0FC001FFFF8001F9FE0001F8000001F8000001F8000001F8000001F800000
  1F8000001F8000001F8000001F800000FF800000FF800000FF800000> 25 32 -1 0 29] 98 @dc
[<EFF0FFFCFC3EF80FF00FE00FE01F00FF0FFF3FFE7FFEFFFCFFF0FE00F80EF00EF01E783E3FFE0FFE> 16 20 -2 0 21] 115 @dc
[<03F00FFC0FDC1F8E1F8E1F8E1F8E1F8E1F801F801F801F801F801F801F801F801F80FFFCFFFC3FFC1F800F80078007800780
  0380038003800380> 15 29 -1 0 20] 116 @dc
[<FFF800FFF800FFF8001F80001F80001F80001F80001F80001F80001F80001F80001F80001F80001F83C01FC7E01FC7E01FE7
  E0FF77E0FF7FC0FF1F80> 19 20 -1 0 22] 114 @dc
[<0FF0FE3FFDFE7E1FFEFC07F0F803F0F803F0F803F0FC03F07E03F03FC3F00FFFF001FFF00003F01E03F03F03F03F03F03F07
  E03F0FE01FFF8007FE00> 23 20 -1 0 25] 97 @dc
[<03FE000FFF801FC3C03F01E07E00E07E0000FC0000FC0000FC0000FC0000FC0000FC0000FC0000FC07807C0FC07E0FC03F0F
  C01F8FC00FFF8003FE00> 19 20 -2 0 23] 99 @dc
cmr10.360 @sf
[<FFFFF800FFFFFF0007C01F8007C007C007C001E007C000F007C000F807C0007C07C0007C07C0003E07C0003E07C0003E07C0
  003F07C0003F07C0003F07C0003F07C0003F07C0003F07C0003F07C0003F07C0003F07C0003E07C0003E07C0003E07C0007C
  07C0007C07C0007807C000F807C001F007C001E007C007C007C01F80FFFFFE00FFFFF800> 32 34 -2 0 38] 68 @dc
[<FFF000FFF0000F00000F00000F00000F00000F00000F00000F00000F00000F1FC00F7FF00FE1F80FC0FC0F807C0F003E0F00
  3E0F001F0F001F0F001F0F001F0F001F0F001F0F001F0F003E0F003E0F007E0FC0FCFFE1F8FF7FF00F1FC0> 24 31 -1 10 28] 112 @dc
[<0FC1E03FF3F87E3FDCFC1F8CF80F8CF8078CF8078CF807807C07807E07803F87800FFF8003FF80000F800007803807807C0F
  807C0F007C3F007FFE001FF000> 22 21 -2 0 25] 97 @dc
[<FFF800FFF8000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00000F87000F8F
  801FCF80FFEF80FFFF800F3E00> 17 21 -1 0 20] 114 @dc
[<01F007F807980F1C0F0C0F0C0F0C0F0C0F0C0F000F000F000F000F000F000F000F000F000F00FFF8FFF83F001F000F000700
  070007000300030003000300> 14 31 -1 0 19] 116 @dc
[<FFF3FFCFFFFFF3FFCFFF0F003C00F00F003C00F00F003C00F00F003C00F00F003C00F00F003C00F00F003C00F00F003C00F0
  0F003C00F00F003C00F00F003C00F00F003C00F00F003C00F00F803E00F00F803E00F01FC07F01F0FFF0FBC3E0FF7FF9FFE0
  0F1FE07F80> 40 21 -1 0 43] 109 @dc
[<FFF3FF80FFF3FF800F0078000F0078000F0078000F0078000F0078000F0078000F0078000F0078000F0078000F0078000F00
  78000F0078000F0078000F8078000F8078001FC0F800FFF1F000FF7FF0000F1FC000> 25 21 -1 0 28] 110 @dc
[<00FC0007FF800F03C01E01E03C00F07C00F8780078F8007CF8007CF8007CF8007CF8007CF8007CF8007C7800787800783C00
  F01E01E00F03C007FF8000FC00> 22 21 -1 0 25] 111 @dc
[<7FFC007FFC000780000780000780000780000780000780000780000780000780000780000780000780000780000780000780
  00078000078000FFF800FFF800078000078000078000078000078000078000078000078000078F8003CF8003EF8001FF8000
  FF00003F00> 17 35 0 0 15] 102 @dc
[<000FF800007FFE0000FE0F0003F003C007E001C00FC000E01F8000701F0000303F0000303E0000387E0000187E000018FC00
  0018FC000000FC000000FC000000FC000000FC000000FC000000FC000000FC000018FC0000187E0000187E0000383E000038
  3F0000381F0000781F8000780FC000F807E001F803F003F800FE0F38007FFE38000FF018> 29 34 -3 0 36] 67 @dc
[<01FC7F8007FF7F800783FC000F01F8000F00F8000F00F8000F0078000F0078000F0078000F0078000F0078000F0078000F00
  78000F0078000F0078000F0078000F0078001F00F800FF07F800FF07F8000F007800> 25 21 -1 0 28] 117 @dc
[<03FC000FFF001F87803F01803E01C07C00C07C0000F80000F80000F80000F80000F80000F80000F800007800007C07003C0F
  803E0F801F0F800FFF8003FE00> 18 21 -2 0 22] 99 @dc
[<0007F000001FFC00007E1E0000F8070001F0038003E001C003E000C003C000E007C0006007C0006007C0006007C0006007C0
  006007C0006007C0006007C0006007C0006007C0006007C0006007C0006007C0006007C0006007C0006007C0006007C00060
  07C0006007C0006007C0006007C0006007C0006007C0006007C001F8FFFE0FFFFFFE0FFF> 32 34 -2 0 37] 85 @dc
[<003800003800007C00007C00007C0000FE0000F60000F60001E30001E30001E30003C18003C18007C1C00780C00780C00F00
  E00F00E01F00F8FFE3FEFFE3FE> 23 21 -1 0 26] 118 @dc
[<CFE0FFF8FC3CF01EE00EE00EC00EC01E007E0FFC3FFC7FF8FFE0FE00F000E00CE00CF01C783C3FFC0FEC> 15 21 -2 0 20] 115 @dc
[<3E00007F0000FB8000F8C000F8C000F86000006000007000003000003000003800003800007C00007C00007C0000FE0000F6
  0000F60001E30001E30001E30003C18003C18007C1C00780C00780C00F00E00F00E01F00F8FFE3FEFFE3FE> 23 31 -1 10 26] 121 @dc
[<03FF000FFFC03F03F07C00F8700038F0003CE0001CE0001CE0003CF0003C7801F83FFFF81FFFF03FFFE03FFF803C00003800
  003000003BF8003FFE001F1F003E0F807C07C07C07C07C07C07C07C07C07C07C07C03E0F9C1F1FBC0FFFFC03F8F8> 22 32 -1 11 25] 103 @dc
[<FFFFFF80FFFFFF8007C01F8007C0078007C0038007C0038007C0018007C001C007C001C007C000C007C000C007C000C007C0
  00C007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C0000007C00000
  07C0000007C0000007C0000007C0000007C0000007C0000007C00000FFFF0000FFFF0000> 26 34 -2 0 31] 76 @dc
[<03F8FF0FFEFF1F0FF83E03F07C01F07C00F07800F0F800F0F800F0F800F0F800F0F800F0F800F0F800F07800F07C00F07C00
  F03E01F01F87F00FFFF003F8F00000F00000F00000F00000F00000F00000F00000F00000F00000F00000F00001F0000FF000
  0FF00000F0> 24 35 -2 0 28] 100 @dc
0 @bop1
cmr10.329 @sf
224 252 p (Researc) s
104 c
15 r (Note) s
15 r (RN/88/31) s
811 r (Researc) s
104 c
224 308 p (August) s
15 r (1988) s
1150 r (Note) s
cmr10.432 @sf
535 1269 p (PP) s
19 r 124 c
20 r 65 c
20 r (Message) s
17 r 84 c
-4 r (ransfer) s
20 r (Agen) s
-1 r 116 c
cmr10.360 @sf
867 1463 p (S.E.) s
16 r (Kille) s
cmbx10.329 @sf
871 1628 p (Abstract) s
cmr10.329 @sf
295 1711 p (This) s
11 r (pap) s
1 r (er) s
12 r (is) s
11 r (to) s
11 r 98 c
1 r 101 c
12 r (presen) s
(ted) s
11 r (at) s
11 r (the) s
12 r (IFIP) s
12 r 87 c
71 c
10 r (6.5) s
10 r (Conference) s
13 r (on) s
11 r (Message) s
224 1768 p (Handling) s
15 r (Systems) s
14 r (and) s
15 r (Distributed) s
14 r (Applications) s
15 r 124 c
15 r (Octob) s
1 r (er) s
16 r (1988.) s
295 1824 p (This) s
15 r (pap) s
1 r (er) s
16 r (describ) s
1 r (es) s
17 r (the) s
16 r (design) s
16 r (and) s
16 r (implem) s
-1 r (en) s
-1 r (tat) s
-1 r (ion) s
14 r (of) s
15 r (PP) s
-3 r 44 c
15 r (whic) s
104 c
14 r (is) s
16 r (an) s
224 1880 p (adv) s
-2 r (anced) s
16 r (system) s
15 r (for) s
15 r (pro) s
(vision) s
14 r (of) s
16 r (Message) s
15 r 84 c
-3 r (ransfer) s
15 r (Services.) s
22 r (The) s
17 r (follo) s
-1 r (wing) s
224 1937 p (features) s
15 r (are) s
15 r (of) s
15 r (particular) s
13 r (note:) s
cmsy10.329 @sf
292 2020 p 15 c
cmr10.329 @sf
23 r (Supp) s
1 r (ort) s
17 r (for) s
17 r 97 c
16 r (wide) s
17 r (range) s
17 r (of) s
16 r (enco) s
1 r (ded) s
19 r (informa) s
-1 r (tio) s
-1 r 110 c
16 r 116 c
(yp) s
1 r (es,) s
16 r (and) s
17 r (for) s
16 r (con-) s
338 2076 p 118 c
(ersion) s
13 r 98 c
1 r (et) s
119 c
(een) s
14 r (them.) s
cmsy10.329 @sf
292 2163 p 15 c
cmr10.329 @sf
23 r (Supp) s
1 r (ort) s
17 r (for) s
16 r (di\013eren) s
116 c
15 r (Message) s
16 r 84 c
-3 r (ransfer) s
15 r (Proto) s
1 r (cols,) s
15 r (and) s
17 r (con) s
118 c
(ers) s
-1 r (ion) s
15 r 98 c
1 r (e-) s
338 2220 p 116 c
119 c
-1 r (een) s
14 r (them.) s
cmsy10.329 @sf
292 2307 p 15 c
cmr10.329 @sf
23 r (Robustness) s
16 r (and) s
15 r (e\016ciency) s
16 r (for) s
14 r (use) s
16 r (in) s
15 r (large) s
14 r (scale) s
15 r (service) s
15 r (en) s
(vironm) s
-1 r (en) s
(t) s
-1 r (s.) s
295 2389 p (This) s
14 r (pap) s
1 r (er) s
15 r (describ) s
1 r (es) s
16 r (the) s
15 r (approac) s
(hes) s
13 r (used) s
16 r (to) s
14 r (ac) s
(hiev) s
-1 r 101 c
13 r (these) s
15 r (features,) s
14 r (and) s
224 2446 p (considers) s
13 r (ho) s
119 c
12 r (they) s
14 r (could) s
13 r 98 c
1 r 101 c
15 r (used) s
14 r (as) s
13 r 97 c
13 r (basis) s
13 r (for) s
13 r (future) s
13 r (services) s
13 r (and) s
14 r (researc) s
(h.) s
224 2502 p (PP) s
14 r (will) s
12 r 98 c
1 r 101 c
14 r (released) s
14 r (in) s
(to) s
11 r (the) s
14 r (public) s
14 r (domain,) s
12 r (as) s
13 r 97 c
13 r (part) s
13 r (of) s
13 r (the) s
14 r (ISODE) s
14 r 80 c
(ac) s
-1 r 107 c
-2 r (ag) s
-1 r 101 c
224 2559 p ([1].) s
cmr10.360 @sf
608 2700 p (Departmen) s
-2 r 116 c
15 r (of) s
17 r (Computer) s
14 r (Science) s
682 2756 p (Univ) s
(ersit) s
121 c
16 r (College) s
18 r (London) s
@eop
@end
