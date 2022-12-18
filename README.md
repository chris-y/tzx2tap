![.github/workflows/z88dk.yml](https://github.com/chris-y/tzx2tap/workflows/.github/workflows/z88dk.yml/badge.svg)


**TZX2TAP v1.3.1** by Chris Young 2020

TZX to TAP file conversion for the ZX Spectrum Next (NextZXOS) as a dot command.

Based on ZXTape Utilities TZX to TAP Converter v0.13b by Tomaz Kac.

To install
- copy TZX2TAP to c:/dot
- copy tzxconv.bas to c:/nextzxos

In c:/nextzxos/browser.cfg, add a new TZX line:

    TZX<.tzx2tap -b "|":load "c:/nextzxos/tzxconv.bas":let f$="|":goto 9000

Put this either before or after the existing line.  If it is before, you can convert TZX files by pressing Enter from the browser.  If it is after, you will need to press Symbol Shift+Enter to do the conversion.  The original TZX loader will be on the other combination.  It is recommended to put it before on a non-accelerated Next, and after on an accelerated Next.

In c:/nextzxos/enBrowsext.cfg, add the following:

    tZX2TAP:f.tzx:<cls:.tzx2tap -v "|":pause 0

Selecting a TZX file in the browser will convert the file, and then ask if you want to load it (similar to how the disk mounter works in release 1.3 of NextZXOS).  This allows you to easily open TZX files on an unexpanded Spectrum Next, provided no custom loader is present in the file.  If no warnings are displayed it is highly likely the file will open fine on a basic Next.

The resulting TAP (named the same but with a .tap extension) will remain available to open as usual afterwards.  If the file already exists it will print an error but will return OK as it assumes the file has already been converted.

Pressing EXT MODE over a TZX file in the browser will give an option on "Z" to convert the file.  This will not ask to automatically open it, but will display a more detailed view of the contents.

This program supports v1.20 of the TZX specification.  Normal and custom loader blocks below 64K are converted.  Most other blocks are skipped over (loops are supported as they were trivial to add).

**Full usage**

`.TZX2TAP [-l|-v|-b] IN.TZX [OUT.TAP]`

`.TZX2TAP` with no options will convert IN.TZX to OUT.TAP.  Without the output filename, the resulting file will be named IN.TAP.

`-v` Verbose mode

This prints more information whilst the file is being converted.

| ID | * | SIZE | DESC |
|----|---|------|------|
|ID of the block from the TZX specification    |indicates whether the block was converted   |size of the block (including headers)      | description of the block or some info extracted from the header      |
|    |` `: not converted   |      |      |
|    |`>`: not converted because longer than 64K (resulting TAP unlikely to work)   |      |      |
|    |`*`: converted but will probably still need a Pi to load on a Next   |      |      |
|    |`+`: converted ok    |      |      |


`-l` List

Same as -v but only lists the blocks does not do any conversion.  The flags are shown so it is possible to tell whether the file would be converted as expected.

`-b` Browser mode

Only for use in browser.cfg.  Shows the conversion on the ZX bar, with the number of blocks converted and number of blocks processed.  A * in front of this number indicates that the file probably won't load on the Next without a Pi and/or blocks longer than 64K were encountered.
