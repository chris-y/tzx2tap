**TZX2TAP v1.2** by Chris Young 2020

TZX to TAP file conversion for the ZX Spectrum Next (NextZXOS) as a dot command.

Based on ZXTape Utilities TZX to TAP Converter v0.13b by Tomaz Kac.

To install
- copy TZX2TAP to c:/dot
- copy tzxconv.bas to c:/nextzxos

In c:/nextzxos/browser.cfg, modify the TZX line to be:

    TZX<clear 65367:.tzx2tap "|":load "c:/nextzxos/tzxconv.bas":let f$="|":goto 9000

This will mean selecting a TZX file in the browser will convert the file, and then ask if you want to load it (similar to how the disk mounter works).  If no warnings are displayed it is highly likely the file will open fine on a basic Next.

The resulting TAP (named the same but with a .tap extension) will remain available to open as usual afterwards.  If the file already exists it will print an error but will return OK as it assumes the file has already been converted.

This allows you to easily open TZX files on an unexpanded Spectrum Next, provided no custom loader is present in the file.

This program supports v1.20 of the TZX specification.  Normal and custom loader blocks below 64K are converted.  Most other blocks are skipped over (loops are supported as they were trivial to add).
