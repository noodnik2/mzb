# Midi Zuke Box

## Overview

These artifacts are from a legacy project used to listen to "playlists"
of [MIDI] files in Microsoft Windows (using its support for [MCI]),
stored as [ZIP] files having the file extension `.mzb`.

### Samples

See a set of sample playlists in the [samples](samples) folder. 

## Note: Old Source Files

The project files were copied from (what is believed to be) the most recent
backup CD, marked on the front with the date "July 20, 2004", from:

- `040720-restore/mzb1119.zip`

Only the source files were kept, and file names were normalized to lower-case.

### Building

The original top-level (Microsoft Visual Studio) build file is at:

- [src/mzb.mak](src/mzb.mak)

Because the toolset used by the project has been deprecated, it's not expected
the project can be built "as is;" reviving the project to work with an updated
toolset will take some work.

### Original Folder Structure

Given the warning above, the following list of original source files and folder structure
is provided below, as it might prove useful when updating the project to a newer toolset:

```
$ unzip 040720-restore/mzb1119.zip
$ find . -type f -exec ls -ld {} \; | cut -c 33-

40240 Aug 22  1996 ./MZB.EXE
766 Jul  4  1996 ./ICONMZB.ICO
2006 Jul  4  1996 ./MZB.BMP
0 Aug 22  1996 ./INFLATE.SBR
12048 Jul 10  1996 ./ZL/ZL.C
10622 Jul  8  1996 ./ZL/HOOPY.MID
39617 Jul  8  1996 ./ZL/INFLATE.C
13279 Jun 27  1996 ./ZL/T1.C
12023 Jul 10  1996 ./ZL/ZL.BAK
6975 Jul  8  1996 ./ZL/T2.DAT
2350 Jul  6  1996 ./ZL/MZB.PDB
102 Jul  8  1996 ./ZL/I/TI.H
39369 Jul  8  1996 ./ZL/I/INFLATE.C
6975 Jul  8  1996 ./ZL/I/T2.DAT
61722 Jul  8  1996 ./ZL/I/UNZIP.H
887 Jul  8  1996 ./ZL/I/INFLATE.H
188 Jul  8  1996 ./ZL/I/T.C
414 Jul  8  1996 ./ZL/ZLI.H
277 Jul  8  1996 ./ZL/ZLC.H
3389 Jun 27  1996 ./ZL/T2.C
2197 Jul 10  1996 ./ZL/ZL.H
674 May 26  1996 ./ZL/T1.H
61672 Jul  8  1996 ./ZL/UNZIP.H
3168 Jul  8  1996 ./ZL/INFLATE.H
908 Jul  8  1996 ./ZL/INFLATE0.H
37084 Jan 25  1993 ./ZL/APPNOTE.TXT
1187 Jul  8  1996 ./ZL/ZLI.C
681 Jul  8  1996 ./ZL/ZLC.C
2671 Jul  8  1996 ./ZL/PKW.H
769 Aug 22  1996 ./ZLI.OBJ
21 Aug 16  1996 ./TODO.TXT
766 Jul  4  1996 ./ICONMZB1.ICO
7296 Jul  6  1996 ./MZB.PDB
29858 Aug 22  1996 ./MZB.MAP
3132 Jul 10  1996 ./MZB.MAK
115 Aug  9  1996 ./TODO.BAK
52340 Aug 22  1996 ./MZB.C
2848 Jul  9  1996 ./OLD/MZB2.RC
40862 Jul 10  1996 ./OLD/MZB.C
40925 Jul 10  1996 ./OLD/MZB.BAK
1055 Jul  6  1996 ./OLD/RESOURCE.H
2692 Jul  6  1996 ./OLD/MZB.RC
3564 Aug 22  1996 ./ZL.OBJ
0 Aug 22  1996 ./MZB.SBR
109 Dec  4  1995 ./RESET.MID
16188 Aug 22  1996 ./MZB.APS
0 Aug 22  1996 ./ZLC.SBR
24 Jul  6  1996 ./POST.BAT
51320 Aug 10  1996 ./MZB.BAK
6111 Aug 22  1996 ./INFLATE.OBJ
0 Aug 22  1996 ./ZLI.SBR
1283 Aug 21  1996 ./RESOURCE.H
172943 Aug 22  1996 ./MZB.BSC
19794 Aug 22  1996 ./MZB.OBJ
3171 Aug 22  1996 ./MZB.RC
90 Aug 22  1996 ./MZB.VCW
1273 Aug 22  1996 ./MZB.RES
529 Aug 22  1996 ./ZLC.OBJ
254 Aug 22  1996 ./MZB.WSP
0 Aug 22  1996 ./ZL.SBR
43639 Jul 10  1996 ./DELETED/MZBNEW.C
251 Jul 10  1996 ./MZB.DEF
```

[MIDI]: https://en.wikipedia.org/wiki/MIDI
[ZIP]: https://en.wikipedia.org/wiki/ZIP_(file_format)
[MCI]: https://en.wikipedia.org/wiki/Media_Control_Interface
