# Microsoft Visual C++ generated build script - Do not modify

PROJ = MZB
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\MARTY\PRV\DEVEL\WIN\MZB\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = MZB.C       
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W4 /Zi /AL /Od /D "_DEBUG" /D "__Windows__" /I "zl" /GA /Fd"MZB.PDB"
CFLAGS_R_WEXE = /nologo /W4 /AL /O1 /D "NDEBUG" /I "zl" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO /MAP  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /MAP  
LIBS_D_WEXE = libw llibcew oldnames commdlg.lib mmsystem.lib 
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib mmsystem.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo 
RUNFLAGS = 
DEFFILE = MZB.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = MZB.SBR \
		ZL.SBR \
		ZLC.SBR \
		ZLI.SBR \
		INFLATE.SBR


MZB_RCDEP = d:\marty\prv\devel\win\mzb\iconmzb.ico


ZL_DEP = d:\marty\prv\devel\win\mzb\zl\zl.h


ZL_DEP = d:\marty\prv\devel\win\mzb\zl\pkw.h \
	d:\marty\prv\devel\win\mzb\zl\zlc.h \
	d:\marty\prv\devel\win\mzb\zl\zli.h \
	d:\marty\prv\devel\win\mzb\zl\zl.h


ZLC_DEP = d:\marty\prv\devel\win\mzb\zl\zl.h \
	d:\marty\prv\devel\win\mzb\zl\pkw.h \
	d:\marty\prv\devel\win\mzb\zl\zlc.h


ZLI_DEP = d:\marty\prv\devel\win\mzb\zl\zli.h \
	d:\marty\prv\devel\win\mzb\zl\zl.h \
	d:\marty\prv\devel\win\mzb\zl\inflate.h


INFLATE_DEP = d:\marty\prv\devel\win\mzb\zl\zli.h \
	d:\marty\prv\devel\win\mzb\zl\zl.h \
	d:\marty\prv\devel\win\mzb\zl\inflate.h


all:	$(PROJ).EXE $(PROJ).BSC

MZB.OBJ:	MZB.C $(MZB_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c MZB.C

MZB.RES:	MZB.RC $(MZB_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r MZB.RC

ZL.OBJ:	ZL\ZL.C $(ZL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ZL\ZL.C

ZLC.OBJ:	ZL\ZLC.C $(ZLC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ZL\ZLC.C

ZLI.OBJ:	ZL\ZLI.C $(ZLI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ZL\ZLI.C

INFLATE.OBJ:	ZL\INFLATE.C $(INFLATE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ZL\INFLATE.C


$(PROJ).EXE::	MZB.RES

$(PROJ).EXE::	MZB.OBJ ZL.OBJ ZLC.OBJ ZLI.OBJ INFLATE.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
MZB.OBJ +
ZL.OBJ +
ZLC.OBJ +
ZLI.OBJ +
INFLATE.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
C:\MSVC\LIB\+
C:\MSVC\MFC\LIB\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) MZB.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	MZB.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) MZB.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
