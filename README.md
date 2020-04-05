This is a version of TZX2TAP converted and built for the ZX Spectrum Next
(NextZXOS) by Chris Young 2020.

To install, copy TZX2TAP to c:/dot
In c:/nextzxos/browser.cfg, modify the TZX line to be:
    TZX<cls:.tzx2tap "|"
This will mean selecting a TZX file in the browser will convert the file.
The resulting TAP will then be available to open as usual.

Original readme follows...




                             ZXTape Utilities
                           --------------------
                           TZX to TAP Converter
                                   v0.13b

                               by Tomaz Kac

  This utility will let you convert the new format .TZX files to standard
.TAP files, which are used by most emulators today. Since .TZX files have more
information about speed and other important features these features will be
lost during conversion. If the .TZX file only had normal loading blocks then
this ofcourse will not matter. All Normal Loading, Custom Loading and Pure
Data blocks will be converted into .TAP file... however the blocks which are
over 64k long will be skipped. Ofcourse all other information and control
blocks will be skipped too.
  This might be usefull for those emulators that can use only .TAP files and
can figure out the speed themselves from the actual code (like Warajevo,
Radovan Garabik's emulator,...).

  Syntax is simple... :  TZX2TAP INPUT.TZX [OUTPUT.TAP]

  If OUTPUT.TAP is not given then the INPUT filename will be used, but its
extension will be changed to .TAP.

  If any of the non-standard blocks are encountered then the --Warning will
be displayed, so you know what has been converted and what has been skipped.


HISTORY: 0.11b  - A bug caused the converter to crash when the TZX file
                  contained Sequence of Pulses blocks.
                  ANSI stuff removed (bold text)
         0.12b  - ZX Tape format version 1.02 is now handled correctly
         0.13b  - Revision 1.03 is used now, Succesfully converted blocks
                  count shows only the count of resulting .TAP blocks now.


  If you are looking for sample .TZX files then the best place (for now) is
the homepage of this format:  http://www.uni-mb.si/~uel047r1a/ZXTape .
The latest revision of the ZXTape format and of all these utilties together
with others that use this format can be found at that page too.

  My email is   tomaz.kac@uni-mb.si   ... mail me if you have any problems
with this program.

                                                                            TC
