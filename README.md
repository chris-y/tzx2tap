**TZX2TAP v1.2** by Chris Young 2020

TZX to TAP file conversion for the ZX Spectrum Next (NextZXOS) as a dot command.

Based on ZXTape Utilities TZX to TAP Converter v0.13b by Tomaz Kac.

To install
- copy TZX2TAP to c:/dot
- copy tzxconv.bas to c:/nextzxos

In c:/nextzxos/browser.cfg, add a new TZX line:

    TZX<clear 65367:.tzx2tap "|":load "c:/nextzxos/tzxconv.bas":let f$="|":goto 9000

Put this either before or after the existing line.  If it is before, you can convert TZX files by pressing Enter from the browser.  If it is after, you will need to press Symbol Shift+Enter to do the conversion.  The original TZX loader will be on the other combination.  It is recommended to put it before on a non-accelerated Next, and after on an accelerated Next.

Selecting a TZX file in the browser will convert the file, and then ask if you want to load it (similar to how the disk mounter works).  If no warnings are displayed it is highly likely the file will open fine on a basic Next.

The resulting TAP (named the same but with a .tap extension) will remain available to open as usual afterwards.  If the file already exists it will print an error but will return OK as it assumes the file has already been converted.

This allows you to easily open TZX files on an unexpanded Spectrum Next, provided no custom loader is present in the file.

This program supports v1.20 of the TZX specification.  Normal and custom loader blocks below 64K are converted.  Most other blocks are skipped over (loops are supported as they were trivial to add).
