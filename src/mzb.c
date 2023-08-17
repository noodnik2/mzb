#define	MSRC_Revision		119
#define	MSRC_Version		1
#define	MSRC_BetaLevel		0

#define	MZBDATE_RELEASE		"August 22, 1996"

#define	STATIC static

/*
	#includes
*/

#include	<windows.h>
#include	<mmsystem.h>
#include	<commdlg.h>
#include	<stdlib.h>
#include	<memory.h>
#include	"zl.h"
#include	"resource.h"


/*
	local #defines
*/

#define	MZB_WM			UINT
#define	MZBWM_OPENSET		(WM_USER+1)
#define	MZBWM_SELDLG 		(WM_USER+2)
#define	MZBWM_STOPSEL		(WM_USER+3)
#define	MZBWM_PAUSESEL		(WM_USER+4)
#define	MZBWM_RESUMESEL		(WM_USER+5)
#define	MZBWM_FFSEL		(WM_USER+6)
#define	MZBWM_REWSEL		(WM_USER+7)
#define	MZBWM_SEEKSEL		(WM_USER+8)
#define	MZBWM_NEWSEL		(WM_USER+10)
#define	MZBWM_NEXTSEL		(WM_USER+11)
#define	MZBWM_PREVSEL		(WM_USER+12)
#define	MZBWM_NEWSELDELAYED	(WM_USER+13)
#define	MZBWM_PLAYSEL		(WM_USER+14)

#define	MZB_TIMER		int
#define	MZBTIMER_DELAY		0
#define	MZBTIMER_STATUS		1

#define	MZB_SSPOS		int
#define	MZBSSPOS_MAX		100

#define	MZB_DM			UINT
#define	MZBDM_REINIT		(WM_USER+1)

#define	MZB_BOOL		int
#define	MZBBOOL_FALSE		0
#define	MZBBOOL_TRUE		(!MZBBOOL_FALSE)

#define	MZBMAX_ERRORMSGLEN	255
#define	MZBMAX_ZIPCOMMENTLEN	128
#define	MZBMAX_FULLFILENAMELEN	_MAX_PATH
#define	MZBMAX_MCICMDLEN	(MZBMAX_FULLFILENAMELEN + 100)
#define	MZBMAX_MCIRETLEN	255
#define	MZBMAX_MCIERRLEN	255
#define	MZBMAX_MCIERREXLEN	(MZBMAX_MCIERRLEN + 128)
#define	MZBMAX_MCIPORTNAMELEN	64
#define	MZBMAX_ENTRYFILENAMELEN	32
#define	MZBMAX_ENTRYCOMMENTLEN	224
#define	MZBMAX_TITLETEXTLEN	128
#define	MZBMAX_SONGPOINTERLEN	64

#define	MZBMSGPFX_ZIPFILEERROR	"ZIPfile access error: "

#define	MZB_ENDOF(s)		(s + lstrlen(s))

#define	MZB_U2			unsigned short
#define	MZB_U4			unsigned long

#define	MZBIS_PAUSED()		mzbp.bIsPaused
#define	MZBUNSET_PAUSED()	mzbp.bIsPaused= MZBBOOL_FALSE
#define	MZBSET_PAUSED()		mzbp.bIsPaused= MZBBOOL_TRUE


/*
	local structs
*/

typedef struct wincrparms {
	int nCmdShow ;
	LPSTR lpszCmdLine ;
} WINCRP, *PWINCRP, FAR *LPWINCRP ;

typedef struct mzb_inivals {
	MZB_U2 uDelayMsec ;
	MZB_U2 uStatusMsec ;
	MZB_U2 uDebugLevel ;
	MZB_U2 uSongFFRewIncrement ;
	MZB_U2 uSongScrollIncrement ;
	MZB_BOOL bAutoDialog ;
	char szLogFileName[MZBMAX_FULLFILENAMELEN+1] ;
	char szResetFileName[MZBMAX_FULLFILENAMELEN+1] ;
	char szTempDirName[MZBMAX_FULLFILENAMELEN+1] ;
	char szMCIPortName[MZBMAX_MCIPORTNAMELEN+1] ;
} MZBI, *PMZBI, FAR *LPMZBI ;

typedef struct mzb_entry {
	unsigned long ulLclHdrPos ;
	char szEntryName[MZBMAX_ENTRYFILENAMELEN+1] ;
	char szEntryComment[MZBMAX_ENTRYCOMMENTLEN+1] ;
} *PMZBE, FAR *LPMZBE ;

typedef struct mzb_set {
	struct mzb_entry *pSetEntry ;
	int iNumEntries ;
	int iEntryIdx ;
	char szZipFileName[MZBMAX_FULLFILENAMELEN+1] ;
	char szZipFileComment[MZBMAX_ZIPCOMMENTLEN+1] ;
} MZBS, *PMZBS, FAR *LPMZBS ;

typedef struct mzb_parms {
	MZB_U4 ulSongPosition ;
	MZB_U4 ulMaxSongPosition ;
	MZB_U4 ulSelSongPositionPending ;
	int iSelIndexPending ;
	int iSelIndex ;
	DWORD dwSongStartTick ;
	MZB_BOOL bUserPlayed ;
	MZB_BOOL bIsPlaying ;
	MZB_BOOL bIsPaused ;
	MZB_BOOL bIsSetHandLoaded ;
	struct mzb_set set ;
	char szTempFileName[MZBMAX_FULLFILENAMELEN+1] ;
} MZBP, *PMZBP, FAR *LPMZBP ;

typedef struct mzb_seldlg {
	int iSelIndex ;
	MZB_BOOL bAutoDialog ;
} MZBSELDLG, *PMZBSELDLG, FAR *LPMZBSELDLG ;


/*
	internal data
*/

#if	MSRC_BetaLevel
STATIC const char cszAppName[]= "MZB Beta Test" ;
#else
STATIC const char cszAppName[]= "MIDI Zuke Box" ;
#endif

STATIC const char cszAppClass[]= "ClsMidiZukeBox" ;
STATIC const char cszTempMIDIFileName[]= "mzbtemp.mid" ;

STATIC char szAppTitle[sizeof(cszAppName) + 25] ;
STATIC char *pszEnvTempDir ;
STATIC FILE *fpLog ;

STATIC HINSTANCE hInst ;
STATIC HWND hWndTop ;
STATIC HWND hDlgSel ;
STATIC MZBP mzbp ;
STATIC MZBI mzbi ;


/*
	internal function decls
*/

LRESULT __export CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM) ;
BOOL __export CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM) ;

STATIC MZB_BOOL lclEventEndSession() ;
STATIC MZB_BOOL lclEventDestroyWindow(HWND hWnd) ;
STATIC MZB_BOOL lclEventCreateWindow(HWND hWnd, LPARAM lParam) ;
STATIC MZB_BOOL lclInitApplication(HINSTANCE) ;
STATIC MZB_BOOL lclFiniInstance() ;
STATIC MZB_BOOL lclInitInstance(LPSTR, int) ;
STATIC MZB_BOOL lclReadProfile() ;
STATIC MZB_BOOL lclWriteProfile() ;
STATIC MZB_BOOL lclErrorMsg(HWND hWndParent, char *pszErrorMsg, long lrc) ;
STATIC MZB_BOOL lclGetMessageProc(WPARAM *pexitcode) ;
STATIC MZB_BOOL lclAboutBox(HWND hWndParent) ;
STATIC MZB_BOOL lclInvokeSelDialog(HWND hWndParent) ;
STATIC MZB_BOOL lclInitSelDialog(LPMZBSELDLG lpseldlg) ;
STATIC void lclUpdateDlgSelSongText() ;
STATIC void lclUpdateDlgSelScrollSong() ;
STATIC void lclUpdateTopWindowState() ;
STATIC void lclUpdateSelDialogState() ;
STATIC void lclSetSelDialogEntry(int iSelIndex) ;
STATIC void lclLoadSelList() ;
STATIC MZB_BOOL lclReadSelDialog(HWND hDlg, LPMZBSELDLG) ;
STATIC MZB_BOOL lclStartSelection(MZB_U4 ulStartSongPosition) ;
STATIC MZB_BOOL lclResumeSelection(MZB_U4 ulResumeSongPosition) ;
STATIC MZB_BOOL lclStopSelection() ;
STATIC MZB_BOOL lclPauseSelection() ;
STATIC MZB_BOOL lclPlayResetfile() ;
STATIC MZB_BOOL lclStartTempfile(MZB_U4 ulStartSongPosition) ;
STATIC MZB_BOOL lclResumeTempfile(MZB_U4 ulResumeSongPosition) ;
STATIC MZB_BOOL lclStopTempfile() ;
STATIC MZB_BOOL lclExtractSelectionIntoTempfile() ;
STATIC MZB_BOOL lclGetDlgSelIndex(LPINT lpIndex) ;
STATIC LPSTR lclGetEntryName(int iSelIndex) ;
STATIC LPSTR lclGetEntryComment(int iSelIndex) ;
STATIC void lclSetWindowTitle(HWND hWnd, LPSTR lpszExtra) ;
STATIC void lclEraseTempfile() ;
STATIC MZB_BOOL lclExtractEntry(int iSelIndex, char *pszSaveFileName) ;
STATIC MZB_BOOL lclCopySetInto(HWND hWndOwner, char *pszSaveFileName) ;
STATIC MZB_BOOL lclOpenMCI(LPCSTR lpcszMidiFileName) ;
STATIC MZB_BOOL lclGetSongPointerMCI() ;
static MZB_U4 lclGetAsciiNumber(const char *pszAscii) ;
STATIC MZB_BOOL sendMCIStringReturnAnswer(
	const char *pszMCICommand,
	char *pszMCIReturn,
	int iLenReturnMax
) ;
STATIC MZB_BOOL sendMCIString(const char *pszMCICommand) ;
STATIC MZB_BOOL sendMCIStringNoCheck(const char *pszMCICommand) ;
STATIC void lclMCINotify(HWND hWnd, WPARAM wParam) ;
STATIC void lclLoadSet() ;
STATIC void lclSaveSet() ;
STATIC void lclSaveFile(int iSelIndex) ;
STATIC MZB_BOOL lclGetOpenFileName(
	HWND hWndOwner,
	char *pszFileName,
	const char *pcszFilter
) ;
STATIC MZB_BOOL lclGetSaveFileName(
	HWND hWndOwner,
	char *pszFileName,
	const char *pcszFilter
) ;
STATIC MZB_BOOL openSet(struct mzb_set *pSet, LPCSTR lpszZipFileName) ;
STATIC MZB_BOOL extractEntry(
	struct mzb_set *pSet,
	int iSetIndex,
	const char *pszIntoFileName
) ;
STATIC MZB_BOOL isSetOpen(struct mzb_set *pSet) ;
STATIC void closeSet(struct mzb_set *pSet) ;
STATIC void lclSaveSelectedFile() ;
STATIC void lclPlaySelectedFile(MZB_U4 ulSongPointer) ;
STATIC void lclSetBusy(MZB_BOOL bBusy) ;

STATIC void lclSeekSelection(MZB_U4 ulNewSongPosition) ;
STATIC void lclFFSelection(MZB_U2 uIncr) ;
STATIC void lclRewSelection(MZB_U2 uDecr) ;
STATIC void lclDoScrollSong(WPARAM wScrollCode, int iSongPos) ;
STATIC void lclCalcScrollSongPos(MZB_U4 *puSongPos, int nPos) ;
STATIC void lclSetScrollSongPos(MZB_U4 uSongPos) ;
STATIC void lclSetScrollSongRange() ;
STATIC void lclInitScrollSong(MZB_U4 u4InitialSongPosition, DWORD dwStartTick) ;
static void lclDebugPage(HWND hWndParent) ;


/*
	main
*/

int PASCAL WinMain(
	HINSTANCE hInstance,
	HINSTANCE hInstancePrev,
	LPSTR lpszCmdLine,
	int nCmdShow
) {
	WPARAM exitcode ;

	hInst= hInstance ;
	if (!lclInitApplication(hInstancePrev)) {
		return(FALSE) ;
	}
	if (!lclInitInstance(lpszCmdLine, nCmdShow)) {
		return(FALSE) ;
	}
	while(lclGetMessageProc(&exitcode)) ;
	lclFiniInstance() ;
	
	return(exitcode) ;
}


/*
	system-exported callback functions
*/

LRESULT __export CALLBACK WindowProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	switch(message) {

		case	WM_CREATE:
			if (!lclEventCreateWindow(hWnd, lParam)) {
				return(-1) ;
				break ;
			}
			return(0) ;
			break ;

		case	WM_DESTROY:
			lclEventDestroyWindow(hWnd) ;
			PostQuitMessage(0) ;
			return(0) ;
			break ;

		case	WM_ENDSESSION:
			if ((BOOL) wParam) {	/* if session ending */
				lclEventEndSession() ;
			}
			return(0) ;
			break ;

		case	WM_QUERYOPEN:
			PostMessage(hWnd, MZBWM_SELDLG, 0, 0L) ;
			return(0) ;
			break ;

		case	MZBWM_OPENSET:
			closeSet(&mzbp.set) ;
			mzbp.iSelIndex= 0;
			mzbp.iSelIndexPending= 0 ;
			mzbp.ulSelSongPositionPending= (MZB_U4) 0 ;
			mzbp.szTempFileName[0]= '\0' ;
			MZBUNSET_PAUSED() ;
			if (!openSet(&mzbp.set, (LPSTR) lParam)) {
				closeSet(&mzbp.set) ;
				lclErrorMsg(hWnd, "Can't open set", 0L) ;
				return((LRESULT) MZBBOOL_FALSE) ;
			}
			return((LRESULT) MZBBOOL_TRUE) ;
			break ;
			
		case	MZBWM_SELDLG:
			lclInvokeSelDialog(hWnd) ;
			return(0) ;
			break ;
 
		case	MZBWM_NEXTSEL:
			PostMessage(
				hWnd,
				MZBWM_PLAYSEL,
				(WPARAM) (mzbp.iSelIndex + 1),
				(LPARAM) 0L
			) ;
			return(0) ;
			break ;

		case	MZBWM_PREVSEL:
			PostMessage(
				hWnd,
				MZBWM_PLAYSEL,
				(WPARAM) (mzbp.iSelIndex - 1),
				(LPARAM) 0L
			) ;
			return(0) ;
			break ;

		case	MZBWM_STOPSEL:
			lclSetBusy(MZBBOOL_TRUE) ;
			lclStopSelection() ;
			lclUpdateSelDialogState() ;
			lclUpdateTopWindowState() ;
			lclSetBusy(MZBBOOL_FALSE) ;
			return(0) ;
			break ;

		case	MZBWM_PAUSESEL:
			lclSetBusy(MZBBOOL_TRUE) ;
			lclPauseSelection() ;
			lclUpdateSelDialogState() ;
			lclUpdateTopWindowState() ;
			lclSetBusy(MZBBOOL_FALSE) ;
			return(0) ;
			break ;

		case	MZBWM_SEEKSEL:
			lclSetBusy(MZBBOOL_TRUE) ;
			lclSeekSelection((MZB_U4) lParam) ;
			lclUpdateSelDialogState() ;
			lclSetBusy(MZBBOOL_FALSE) ;
			return(0) ;
			break ;

		case	MZBWM_FFSEL:
			lclSetBusy(MZBBOOL_TRUE) ;
			lclFFSelection((MZB_U2) wParam) ;
			lclUpdateSelDialogState() ;
			lclSetBusy(MZBBOOL_FALSE) ;
			return(0) ;
			break ;

		case	MZBWM_REWSEL:
			lclSetBusy(MZBBOOL_TRUE) ;
			lclRewSelection((MZB_U2) wParam) ;
			lclUpdateSelDialogState() ;
			lclSetBusy(MZBBOOL_FALSE) ;
			return(0) ;
			break ;

		case	MZBWM_RESUMESEL:
			lclSetBusy(MZBBOOL_TRUE) ;
			lclResumeSelection((MZB_U4) lParam) ;
			lclUpdateSelDialogState() ;
			lclUpdateTopWindowState() ;
			lclSetBusy(MZBBOOL_FALSE) ;
			return(0) ;
			break ;

		case	MZBWM_PLAYSEL:
			SendMessage(hWnd, MZBWM_STOPSEL, 0, 0L) ;
			if (!mzbp.bIsPlaying) {
				PostMessage(
					hWnd,
				     	MZBWM_NEWSEL,
					wParam,
					lParam
				) ;
				return(0) ;
			}
			PostMessage(
				hWnd,
				MZBWM_NEWSELDELAYED,
				wParam,
				lParam
			) ;
			return(0) ;
			break ;

		case	MZBWM_NEWSEL:
			mzbp.iSelIndex= (int) wParam ;
			if (mzbp.iSelIndex< 0) {
				mzbp.iSelIndex= 0 ;
			}
			if (mzbp.iSelIndex>= mzbp.set.iNumEntries) {
				mzbp.iSelIndex= mzbp.set.iNumEntries - 1 ;
			}
			lclSetBusy(MZBBOOL_TRUE) ;
			lclSetSelDialogEntry(mzbp.iSelIndex) ;
			lclStartSelection((MZB_U4) lParam) ;
			lclUpdateSelDialogState() ;
			lclUpdateTopWindowState() ;
			lclSetBusy(MZBBOOL_FALSE) ;
			return(0) ;
			break ;

		case	MZBWM_NEWSELDELAYED:
			mzbp.iSelIndexPending= (int) wParam ;
			mzbp.ulSelSongPositionPending= (MZB_U4) lParam ;
			if (mzbi.uDelayMsec< 10) {
				PostMessage(hWnd, WM_TIMER, 0, 0L) ;
				return(0) ;
			}
			SetTimer(
				hWnd,
				MZBTIMER_DELAY,
				(UINT) mzbi.uDelayMsec,
				NULL
			) ;
			return(0) ;
			break ;

		case	WM_TIMER:
			switch(wParam) {

				case	MZBTIMER_DELAY:
					KillTimer(hWnd, MZBTIMER_DELAY) ;
					PostMessage(
						hWnd,
						MZBWM_NEWSEL,
						(WPARAM) mzbp.iSelIndexPending,
						(LPARAM) (
						   mzbp.ulSelSongPositionPending
						)
					) ;
					return(0) ;
					break ;

				case	MZBTIMER_STATUS:
					lclUpdateDlgSelScrollSong() ;
					return(0) ;
					break ;
			}
			break ;

		case	WM_CLOSE:
			lclStopSelection() ;
			DestroyWindow(hWnd) ;
			return(0) ;
			break ;

		case	MM_MCINOTIFY:
			lclMCINotify(hWnd, wParam) ;
			return(0) ;
			break ;
	}

	return(DefWindowProc(hWnd, message, wParam, lParam)) ;
}

BOOL __export CALLBACK DialogProc(
	HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	static LPMZBSELDLG lpseldlg= (LPMZBSELDLG) 0 ;
	WORD wVal ;

	switch(message) {

		case	MZBDM_REINIT:
			lpseldlg->iSelIndex= 0 ;
			lclInitSelDialog(lpseldlg) ;
			break ;
			
		case	WM_DESTROY:
			lpseldlg= (LPMZBSELDLG) 0 ;
			break ;

		case	WM_INITDIALOG:
			hDlgSel= hDlg ;
			lpseldlg= (LPMZBSELDLG) lParam ;
			lclInitSelDialog(lpseldlg) ;
			SetWindowText(hDlg, szAppTitle) ;
			return(TRUE) ;		/* we didn't set focus	*/
			break ;

		case	WM_HSCROLL:
			lclDoScrollSong(wParam, (int) LOWORD(lParam)) ;
			break ;

		case	WM_COMMAND:

			switch(wParam) {
				case	IDOK:
				case	IDCANCEL:
				case	IDEXIT:
				case	IDABOUT:
					break ;

				default:
					mzbp.bUserPlayed= MZBBOOL_TRUE ;
					break ;
			}

			switch(wParam) {

				case	IDOK:
				case	IDEXIT:
					lclReadSelDialog(hDlg, lpseldlg) ;
					/*FALLTHRU*/
				case	IDCANCEL:
					hDlgSel= (HWND) NULL ;
					EndDialog(hDlg, (int) wParam) ;
					return(TRUE) ;
					break ;

				case	IDABOUT:
					if (
					    	(GetKeyState(VK_MENU)&0x8000)
					     &&	(GetKeyState(VK_SHIFT)&0x8000)
					     &&	(GetKeyState(VK_CONTROL)&0x8000)
					) {
						lclDebugPage(hDlg) ;
						break ;
					}
					lclAboutBox(hDlg) ;
					break ;

				case	IDFF:
					PostMessage(
						hWndTop,
						MZBWM_FFSEL,
						mzbi.uSongFFRewIncrement,
						0L
					) ;
					break ;

				case	IDREW:
					PostMessage(
						hWndTop,
						MZBWM_REWSEL,
						mzbi.uSongFFRewIncrement,
						0L
					) ;
					break ;

				case	IDPREV:
					PostMessage(
						hWndTop,
						MZBWM_PREVSEL,
						0,
						0L
					) ;
					break ;

				case	IDNEXT:
					PostMessage(
						hWndTop,
						MZBWM_NEXTSEL,
						0,
						0L
					) ;
					break ;

				case	IDPLAY:
					lclPlaySelectedFile(mzbp.ulSongPosition) ;
					break ;

				case	IDSTOP:
					PostMessage(
						hWndTop,
						MZBWM_STOPSEL,
						0,
						0L
					) ;
					break ;

				case	IDPAUSE:
					PostMessage(
						hWndTop,
						MZBWM_PAUSESEL,
						0,
						0L
					) ;
					break ;

				case	IDSAVE:
					lclSaveSelectedFile() ;
					break ;

				case	IDLOADSET:
					lclLoadSet() ;
					mzbp.bIsSetHandLoaded= MZBBOOL_TRUE ;
					lclUpdateTopWindowState() ;
					break ;

				case	IDSAVESET:
					lclSaveSet() ;
					break ;

				case	IDC_CBFLIST:
					wVal= HIWORD(lParam) ;
					if (wVal == CBN_SELCHANGE) {
						lclUpdateDlgSelSongText() ;
						return(TRUE) ;
					}
#if 0
					if (wVal == CBN_SELENDOK) {
						/* combo box closed up */
						return(TRUE) ;
					}
#endif
					break ;

			}
			break ;
	}

	return(FALSE) ;
}	


/*
	internal functions
*/

/* ******************************************************************* */

STATIC MZB_BOOL lclGetMessageProc(WPARAM *pexitcode)
{
	MSG msg ;
	static bExited= FALSE ;
	static WPARAM exitcode ;
	
	if (!bExited) {
		if (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg) ;
			DispatchMessage(&msg) ;
			return(MZBBOOL_TRUE) ;
		}
		exitcode= msg.wParam ;
		bExited= TRUE ;
	}
	if (pexitcode != (WPARAM *) NULL) {
		(*pexitcode)= msg.wParam ;
	}
	return(MZBBOOL_FALSE) ;			
}

/* ******************************************************************* */

STATIC MZB_BOOL lclInitApplication(HINSTANCE hInstancePrev)
{
	WNDCLASS wc ;

	if (hInstancePrev) return(MZBBOOL_TRUE) ;
	
	wc.style	= 0 ;
	wc.lpfnWndProc	= (WNDPROC) WindowProc ;
	wc.cbClsExtra	= 0 ;
	wc.cbWndExtra	= 0 ;
	wc.hInstance	= hInst ;
	wc.hIcon	= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONMZB)) ;
	wc.hCursor	= LoadCursor(NULL, IDC_ARROW) ;
	wc.hbrBackground= (HBRUSH) (COLOR_BACKGROUND + 1) ;
	wc.lpszMenuName	= NULL ;
	wc.lpszClassName= cszAppClass ;
	
	if (!RegisterClass(&wc)) {
		lclErrorMsg(NULL, "Can't register window class", 0L) ;
		return(MZBBOOL_FALSE) ;
	}
	
	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclInitInstance(LPSTR lpszCmdLine, int nCmdShow)
{
	HWND hWnd ;
	WINCRP wcrp ;

	fpLog= (FILE *) NULL ;
	wsprintf(
		szAppTitle,
		"%s v%d.%03d",
		cszAppName,
		MSRC_Version,
		MSRC_Revision
	) ;

	wcrp.nCmdShow= nCmdShow ;
	wcrp.lpszCmdLine= lpszCmdLine ;
	pszEnvTempDir= getenv("TEMP") ;

	if (!lclReadProfile()) {
		lclErrorMsg(NULL, "Can't read profile", 0L) ;
		return(MZBBOOL_FALSE) ;
	}

	if (mzbi.szLogFileName[0]) {
		fpLog= fopen(mzbi.szLogFileName, "w") ;
		if (!fpLog) lclErrorMsg(NULL, "fopen error\n", 0L) ;
	}

	hWnd= CreateWindow(
		cszAppClass,
		cszAppName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInst,
		(void FAR *) ((LPWINCRP) &wcrp)
	) ;
	if (hWnd == NULL) {
		lclErrorMsg(NULL, "Can't create window", 0L) ;
		return(MZBBOOL_FALSE) ;
	}

	ShowWindow(hWnd, SW_SHOWMINNOACTIVE) ;
	UpdateWindow(hWnd) ;

	if (mzbi.bAutoDialog) {
		PostMessage(hWnd, MZBWM_SELDLG, 0, 0L) ;
	}

	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclFiniInstance()
{
	if (!lclWriteProfile()) {
		lclErrorMsg(NULL, "Can't write profile", 0L) ;
		return(MZBBOOL_FALSE) ;
	}
	if (fpLog) fclose(fpLog) ;
	return(MZBBOOL_TRUE) ;
}

/* ******************************************************************* */

STATIC MZB_BOOL lclErrorMsg(HWND hWndParent, char *pszErrorMsg, long lrc)
{
	char szErrorMsg[MZBMAX_ERRORMSGLEN+1] ;

	szErrorMsg[0]= '\0' ;
	if (lrc != 0) wsprintf((LPSTR) MZB_ENDOF(szErrorMsg), "R(%ld); ", lrc) ;
	lstrcpy((LPSTR) MZB_ENDOF(szErrorMsg), pszErrorMsg) ;
	EnableWindow(hWndParent, FALSE) ;
	MessageBox(
		hWndParent,
		szErrorMsg,
		cszAppName,
		MB_OK | MB_ICONEXCLAMATION
	) ;
	EnableWindow(hWndParent, TRUE) ;
	return(MZBBOOL_FALSE) ;
}

/******************************************************************/
/******************************************************************/


static void lclDebugPage(HWND hWndParent)
{
	char szPageBuff[1024] ;
	unsigned long ulSec ;

	ulSec= (mzbp.ulMaxSongPosition + 500L) / 1000L ;

	wsprintf(
		szPageBuff,
		"Song Length: %lu:%02lu (%lu)",
		ulSec / 60L,
		ulSec % 60L,
		mzbp.ulMaxSongPosition
	) ;

	MessageBox(
		hWndParent,
		szPageBuff,
		cszAppName,
		MB_OK | MB_ICONINFORMATION
	) ;
}

/******************************************************************/
/******************************************************************/

STATIC const char cszProfileValueNone[]= "<empty>" ;
STATIC const char cszProfileSection[]= "MZB Midi General" ;
STATIC const char cszProfileFile[]= "mzb.ini" ;

STATIC BOOL lclGetProfileString(
	const char *pszVar,
	char *pszStr,
	char *pszStrDef,
	int maxsize
) {
	GetPrivateProfileString(
		cszProfileSection,
		pszVar,
		(LPCSTR) (pszStrDef? pszStrDef: ""),
		pszStr,
		maxsize,
		cszProfileFile
	) ;
	if (lstrcmp(pszStr, cszProfileValueNone) == 0) {
		pszStr[0]= '\0' ;
	}
	return(TRUE) ;
}

STATIC BOOL lclGetProfileU2(const char *pszVar, MZB_U2 *puValue, MZB_U2 uDef)
{
	(*puValue)= (MZB_U2) GetPrivateProfileInt(
		cszProfileSection,
		pszVar,
		(int) uDef,		/* Stupid this is an 'int' */
		cszProfileFile
	) ;
	return(TRUE) ;
}

STATIC MZB_BOOL lclGetProfileBool(
	const char *pszVar,
	MZB_BOOL *pbValue,
	MZB_BOOL bValueDef
) {
	MZB_BOOL bOk ;
	MZB_U2 uValue ;

	bOk= lclGetProfileU2(pszVar, &uValue, (MZB_U2) bValueDef) ;
	(*pbValue)= (MZB_BOOL) uValue ;
	return(bOk) ;
}

/******************************************************************/

STATIC BOOL lclWriteProfileString(const char *pszVar, const char *pszValue)
{
	LPCSTR lpcszValue ;

	/*
		I cast "pszValue" because the compiler doesn't make
		the proper conversion when it's NULL.
	*/
	lpcszValue= (pszValue == (char *) NULL)	?
		(LPCSTR) NULL 			:
		(LPCSTR) pszValue		;

	if (lpcszValue != (LPCSTR) NULL) {
		if (lpcszValue[0] == '\0') lpcszValue= (LPCSTR) NULL ;
	}

	/*
		"cszProfileValueNone" added so that all string
		variables would always appear in .INI file.
	*/
	if (lpcszValue == (LPCSTR) NULL) {
		lpcszValue= (LPCSTR) cszProfileValueNone ;
	}

	WritePrivateProfileString(
		cszProfileSection,
		pszVar,
		lpcszValue,
		cszProfileFile
	) ;

	return(TRUE) ;
}

STATIC BOOL lclWriteProfileInt(const char *pszVar, int iValue)
{
	char profileintstrbuf[16] ;

	wsprintf(profileintstrbuf, "%d", iValue) ;
	return(lclWriteProfileString(pszVar, profileintstrbuf)) ;
}

STATIC BOOL lclWriteProfileBool(const char *pszVar, BOOL bValue)
{
	return(lclWriteProfileInt(pszVar, (int) bValue)) ;
}

/******************************************************************/
                    
STATIC const char cszProfileVarLogName[]= "Log File" ;
STATIC const char cszProfileVarDebug[]= "Debug Level" ;
STATIC const char cszProfileVarPIncr[]= "FF_Rew Increment" ;
STATIC const char cszProfileVarSIncr[]= "Scroll Increment" ;
STATIC const char cszProfileVarAutoDlg[]= "Auto Dialog" ;
STATIC const char cszProfileVarTempdir[]= "Temp Directory" ;
STATIC const char cszProfileVarReset[]= "Reset MIDI File" ;
STATIC const char cszProfileVarDelay[]= "Intersong Delay" ;
STATIC const char cszProfileVarStatus[]= "Status update" ;
STATIC const char cszProfileVarPort[]= "MCI Port" ;

STATIC MZB_BOOL lclReadProfile()
{
	memset(&mzbi, '\0', sizeof(mzbi)) ;

	lclGetProfileU2(cszProfileVarDebug, &mzbi.uDebugLevel, 0) ;
	lclGetProfileU2(cszProfileVarPIncr, &mzbi.uSongFFRewIncrement, 5000) ;
	lclGetProfileU2(cszProfileVarSIncr, &mzbi.uSongScrollIncrement, 10000) ;
	lclGetProfileU2(cszProfileVarDelay, &mzbi.uDelayMsec, 100) ;
	lclGetProfileU2(cszProfileVarStatus, &mzbi.uStatusMsec, 5000) ;
	lclGetProfileBool(cszProfileVarAutoDlg, &mzbi.bAutoDialog, FALSE) ;

	lclGetProfileString(
		cszProfileVarLogName,
		mzbi.szLogFileName,
		(char *) NULL,
		sizeof(mzbi.szLogFileName)
	) ;
	lclGetProfileString(
		cszProfileVarTempdir,
		mzbi.szTempDirName,
		(char *) NULL,
		sizeof(mzbi.szTempDirName)
	) ;
	lclGetProfileString(
		cszProfileVarReset,
		mzbi.szResetFileName,
		(char *) NULL,
		sizeof(mzbi.szResetFileName)
	) ;
	lclGetProfileString(
		cszProfileVarPort,
		mzbi.szMCIPortName,
		"mapper",
		sizeof(mzbi.szMCIPortName)
	) ;

	return(MZBBOOL_TRUE) ;
}

/******************************************************************/

STATIC MZB_BOOL lclWriteProfile()
{
	lclWriteProfileInt(cszProfileVarDelay,  mzbi.uDelayMsec) ;
	lclWriteProfileInt(cszProfileVarStatus,  mzbi.uStatusMsec) ;
	lclWriteProfileInt(cszProfileVarPIncr,  mzbi.uSongFFRewIncrement) ;
	lclWriteProfileInt(cszProfileVarSIncr,  mzbi.uSongScrollIncrement) ;
	lclWriteProfileBool(cszProfileVarAutoDlg,  mzbi.bAutoDialog) ;
	lclWriteProfileString(cszProfileVarTempdir, mzbi.szTempDirName) ;
	lclWriteProfileString(cszProfileVarReset, mzbi.szResetFileName) ;
	lclWriteProfileString(cszProfileVarPort, mzbi.szMCIPortName) ;
	return(MZBBOOL_TRUE) ;
}

/******************************************************************/
/******************************************************************/

STATIC MZB_BOOL lclAboutBox(HWND hWndParent)
{
	char szAppTitleAbout[sizeof(cszAppName) + 25] ;
	static const char cszAboutText[]= 
		"Midi \"Zuke\" Box... \"a juke box with a ZIP!\"\n"
		"Plays sets collected from \"Marty's MIDI Catalog\"\n"
		"\n"
		"Copyright (C) 1996, by Marty Ross.  All rights reserved.\n"
		"No warranty af any kind granted or implied.\n"
		"\n"
		"This version released on " MZBDATE_RELEASE ".  Update now!\n"
		"(http://advant.pos.co.il/~marty/mzbdoc.htm)\n"
		"(e-mail: marty@netvision.net.il)\n"
		"\n"
		"NOTES:\n"
		"(1) Uses \"MCI MIDI Sequencer\" for MIDI device access.\n"
		"(2) See \"mzb.ini\" file for tunable settings.\n"
		"(3) Handles only limited subset of ZIPfile formats.\n"
	;

	wsprintf(szAppTitleAbout, "About %s", szAppTitle) ;
	MessageBox(
		hWndParent,
		cszAboutText,
		szAppTitleAbout,
		MB_OK | MB_ICONINFORMATION
	) ;
	return(MZBBOOL_TRUE) ;
}

/* ******************************************************************* */

STATIC HCURSOR hCurHourGlass, hCurSave ;
STATIC MZB_BOOL bIsBusyNow= MZBBOOL_FALSE ;
STATIC void lclSetBusy(MZB_BOOL bBusy)
{
	if (bBusy == bIsBusyNow) return ;
	if (bBusy) {
		if (!hDlgSel) return ;
		if (hDlgSel != GetActiveWindow()) return ;
	}

	if (!hCurHourGlass) {
		hCurHourGlass= LoadCursor(NULL, IDC_WAIT) ;
		if (!hCurHourGlass) return ;
	}
	hCurSave= SetCursor(bBusy? hCurHourGlass: hCurSave) ;
	bIsBusyNow= bBusy ;
}
	

/* ******************************************************************* */

STATIC MZB_BOOL lclInvokeSelDialog(HWND hWndParent)
{
	DLGPROC lpfnDialogProc ;
	MZBSELDLG seldlg ;
	int seldlgrc ;

	if (hDlgSel) return(MZBBOOL_FALSE) ;		/* already active! */

	memset(&seldlg, '\0', sizeof(seldlg)) ;
	seldlg.iSelIndex= mzbp.iSelIndex ;
	seldlg.bAutoDialog= mzbi.bAutoDialog ;

	lpfnDialogProc= (DLGPROC) MakeProcInstance(
		(FARPROC) DialogProc, hInst
	) ;
 	seldlgrc= DialogBoxParam(
		hInst,
		"MIDIBOX",
		hWndParent,
		lpfnDialogProc,
		(LPARAM) ((LPMZBSELDLG) &seldlg)
	) ;
	FreeProcInstance((FARPROC) lpfnDialogProc) ;
	
	if (seldlgrc< 0) {
		lclErrorMsg(
			hWndParent,
			"Can't create dialog",
			(long) seldlgrc
		) ;
		return(MZBBOOL_FALSE) ;
	}

	switch(seldlgrc) {

		case	IDOK:
			mzbi.bAutoDialog= seldlg.bAutoDialog ;
			if (seldlg.iSelIndex != mzbp.iSelIndex) {
				mzbp.bUserPlayed= MZBBOOL_TRUE ;
				PostMessage(
					hWndTop,
					MZBWM_PLAYSEL,
					(WPARAM) seldlg.iSelIndex,
					0L
				) ;
			}
			break ;

		case	IDEXIT:
			PostMessage(hWndTop, WM_CLOSE, 0, 0L) ;
			break ;
	}

	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclInitSelDialog(LPMZBSELDLG lpseldlg)
{
	if (!hDlgSel || !lpseldlg) return(MZBBOOL_FALSE) ;

	/*
		load the dialog's combo-box with the list of files
	*/
	lclLoadSelList() ;

	/*
		set the zipfile comment.
	*/
	SendDlgItemMessage(
		hDlgSel,
		IDC_SSETNAME,
		WM_SETTEXT,
		(WPARAM) 0,
		(LPARAM) ((LPSTR) mzbp.set.szZipFileComment)
	) ;

	/*
		set entry to that specified by dialog invocation
	*/
	lclSetSelDialogEntry(lpseldlg->iSelIndex) ;

	/*
		Set the check-box states
	*/
	SendDlgItemMessage(
		hDlgSel,
		IDC_CKAUTO,
		BM_SETCHECK,
		lpseldlg->bAutoDialog? 1: 0,
		0
	) ;

	/*
		Set permanent range for scroll bar
	*/
	lclSetScrollSongRange()	;

	/*
		update "state" fields
	*/
	lclUpdateSelDialogState() ;

	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclGetDlgSelIndex(LPINT lpIndex)
{
	DWORD dwIndex ;

	if (!hDlgSel) return(MZBBOOL_FALSE) ;
	dwIndex= SendDlgItemMessage(
		hDlgSel,
		IDC_CBFLIST,
		CB_GETCURSEL,
		0,
		0L
	) ;
	if (dwIndex == CB_ERR) {
		return(MZBBOOL_FALSE) ;
	}
	if (lpIndex != (LPINT) 0) {
		(*lpIndex)= (int) LOWORD(dwIndex) ;
	}
	return(MZBBOOL_TRUE) ;
}

STATIC void lclUpdateDlgSelScrollSong()
{
	if (!mzbp.bIsPlaying) return ;
	lclSetScrollSongPos((MZB_U4) (GetTickCount() - mzbp.dwSongStartTick)) ;
}

STATIC void lclUpdateDlgSelSongText()
{
	int iSelIndex ;
	unsigned long ulSec ;
	char szTextBuf[128] ;

	if (!lclGetDlgSelIndex(&iSelIndex)) return ;

	/*
		set the song comment
	*/
	SendDlgItemMessage(
		hDlgSel,
		IDC_SCOMMENT,
		WM_SETTEXT,
		(WPARAM) 0,
		(LPARAM) lclGetEntryComment(iSelIndex)
	) ;

	/*
		set the song time text
	*/
	ulSec= (mzbp.ulMaxSongPosition + 500L) / 1000L ;
	wsprintf(szTextBuf, "%lu:%02lu", ulSec / 60L, ulSec % 60L) ;
	SendDlgItemMessage(
		hDlgSel,
		IDC_MAXTIME,
		WM_SETTEXT,
		(WPARAM) 0,
		(LPARAM) ((LPSTR) szTextBuf)
	) ;
}

STATIC void lclUpdateTopWindowState()
{
	if (hWndTop == (HWND) NULL) return ;
	lclSetWindowTitle(
		hWndTop,
		(
			mzbp.bIsPlaying
		      ? lclGetEntryName(mzbp.iSelIndex)
		      : (LPSTR) NULL
		)
	) ;
}

STATIC void lclSetSelDialogEntry(int iSelIndex)
{
	if (!hDlgSel) return ;

	/*
		set the current item in the combo-box
	*/
	SendDlgItemMessage(
		hDlgSel,
		IDC_CBFLIST,
		CB_SETCURSEL,
		(WPARAM) iSelIndex,
		(LPARAM) 0L
	) ;
}

/*
	update selection dialog "state" fields
*/
STATIC void lclUpdateSelDialogState()
{
	MZB_BOOL bIsPlayable ;

	if (!hDlgSel) return ;

	/*
		enable/disable appropriate buttons
	*/

	bIsPlayable= lclGetDlgSelIndex((LPINT) 0) ;
	EnableWindow(
		GetDlgItem(hDlgSel, IDC_SBSCROLLSONG),
		bIsPlayable
	) ;
	EnableWindow(
		GetDlgItem(hDlgSel, IDPLAY),
		bIsPlayable
	) ;
	EnableWindow(
		GetDlgItem(hDlgSel, IDSTOP),
		(BOOL) (mzbp.bIsPlaying || MZBIS_PAUSED())
	) ;
	EnableWindow(
		GetDlgItem(hDlgSel, IDPAUSE),
		(BOOL) mzbp.bIsPlaying
	) ;
	EnableWindow(
		GetDlgItem(hDlgSel, IDREW),
		(BOOL) mzbp.bIsPlaying
	) ;
	EnableWindow(
		GetDlgItem(hDlgSel, IDFF),
		(BOOL) mzbp.bIsPlaying
	) ;
	EnableWindow(
		GetDlgItem(hDlgSel, IDPREV),
		(BOOL) (mzbp.iSelIndex> 0)
	) ;
	EnableWindow(
		GetDlgItem(hDlgSel, IDNEXT),
		(BOOL) (mzbp.iSelIndex< mzbp.set.iNumEntries)
	) ;

	/*
		"update" the comment text box
		to correspond to selected entry
	*/
	lclUpdateDlgSelSongText() ;
	lclUpdateDlgSelScrollSong() ;
}

STATIC MZB_BOOL lclReadSelDialog(HWND hDlg, LPMZBSELDLG lpseldlg)
{
	int iSelIndex ;

	if (!hDlg) return(MZBBOOL_FALSE) ;

	/*
		read the state of the "auto" check button
	*/
	lpseldlg->bAutoDialog= (
		SendDlgItemMessage(
			hDlg,
			IDC_CKAUTO,
			BM_GETCHECK,
			0,
			0L
		)
	      ? MZBBOOL_TRUE
	      : MZBBOOL_FALSE
	) ;

	/*
		read the selected entry index from the combo box
	*/
	if (!lclGetDlgSelIndex(&iSelIndex)) return(MZBBOOL_FALSE) ;
	lpseldlg->iSelIndex= iSelIndex ;

	return(MZBBOOL_TRUE) ;
}

/* ******************************************************************* */

STATIC void lclPlaySelectedFile(MZB_U4 ulSongPointer)
{
	int iSelIndex ;

	if (!lclGetDlgSelIndex(&iSelIndex)) {
		lclErrorMsg(hDlgSel, "No song to play", 0L) ;
		return ;
	}
	if (
		(MZBIS_PAUSED() || mzbp.bIsPlaying)
	     && (iSelIndex == mzbp.iSelIndex)
	) {
		PostMessage(
			hWndTop,
			MZBWM_RESUMESEL,
			0,
			(LPARAM) ulSongPointer
		) ;
		return ;
	}
	PostMessage(hWndTop, MZBWM_PLAYSEL, (WPARAM) iSelIndex, 0L) ;
}

STATIC void lclSaveSelectedFile()
{
	int iSelIndex ;

	if (!lclGetDlgSelIndex(&iSelIndex)) {
		lclErrorMsg(hDlgSel, "No song to save", 0L) ;
		return ;
	}
	lclSaveFile(iSelIndex) ;
}

/* ******************************************************************* */

STATIC const char cszDefaultSetName[]= "myset.mzb" ;
STATIC const char cszSetFilter[]=
	"MZB Sets\0*.mzb\0"
	"MZB/ZIP Sets\0*.mzb;*.zip\0"
	"Zip Files\0*.zip\0"
	"All Files\0*.*\0"
	"\0"
;

STATIC void lclLoadSet()
{
	char szLoadFileName[MZBMAX_FULLFILENAMELEN+1] ;

	if (!hDlgSel) return ;	/* never happens */

	if (mzbp.bIsPlaying) {
		lclErrorMsg(hDlgSel, "Stop playing song first", 0L) ;
		return ;
	}

	memset(szLoadFileName, '\0', sizeof(szLoadFileName)) ;
	if (!lclGetOpenFileName(hDlgSel, szLoadFileName, cszSetFilter)) {
		return ;
	}

	SendMessage(
		hWndTop,
		MZBWM_OPENSET,
		0,
		(LPARAM) ((LPSTR) szLoadFileName)
	) ;

	PostMessage(hDlgSel, MZBDM_REINIT, 0, 0L) ;
}

STATIC void lclSaveSet()
{
	char szSaveFileName[MZBMAX_FULLFILENAMELEN+1] ;

	if (!hDlgSel) return ;	/* never happens */

	if (!isSetOpen(&mzbp.set)) {
		lclErrorMsg(hDlgSel, "No set to save", 0L) ;
		return ;
	}

	memset(szSaveFileName, '\0', sizeof(szSaveFileName)) ;
	lstrcpy(szSaveFileName, cszDefaultSetName) ;
	if (!lclGetSaveFileName(hDlgSel, szSaveFileName, cszSetFilter)) {
		return ;
	}
	lclSetBusy(MZBBOOL_TRUE) ;
	lclCopySetInto(hDlgSel, szSaveFileName) ;
	lclSetBusy(MZBBOOL_FALSE) ;
}

STATIC void lclSaveFile(int iSelIndex)
{
	char szSaveFileName[MZBMAX_FULLFILENAMELEN+1] ;
	LPCSTR lpcsEntryName ;
	static const char cszFilter[]=
		"Midi Files (*.mid;*.smf)\0*.mid;*.smf\0"
		"Karaoke Files (*.kar)\0*.kar\0"
		"All Files (*.*)\0*.*\0"
		"\0"
	;

	memset(szSaveFileName, '\0', sizeof(szSaveFileName)) ;
	lpcsEntryName= lclGetEntryName(iSelIndex) ;
	if (lpcsEntryName) lstrcpy(szSaveFileName, lpcsEntryName) ;
	if (!lclGetSaveFileName(hDlgSel, szSaveFileName, cszFilter)) {
		return ;
	}
	lclSetBusy(MZBBOOL_TRUE) ;
	lclExtractEntry(iSelIndex, szSaveFileName) ;
	lclSetBusy(MZBBOOL_FALSE) ;
}

/* ******************************************************************* */

STATIC MZB_BOOL lclGetOpenFileName(
	HWND hWndOwner,
	char *pszFileName,
	const char *pcszFilter
) {
	struct openfiledata_tag {
		const char *pcszFilter ;
		char szFileName[MZBMAX_FULLFILENAMELEN+1] ;
		char szFileTitle[MZBMAX_FULLFILENAMELEN+1] ;
		char szFileDir[MZBMAX_FULLFILENAMELEN+1] ;
	} ofd ;
	OPENFILENAME ofn ;

	/*
		set open file data
	*/	
	memset(&ofd, '\0', sizeof(ofd)) ;
	ofd.pcszFilter= pcszFilter ;
	lstrcpy(ofd.szFileName, pszFileName) ;

	/*
		set OPENFILENAME structure
	*/
	memset(&ofn, '\0', sizeof(ofn)) ;
	ofn.lStructSize= sizeof(ofn) ;
	ofn.hwndOwner= hWndOwner ;
	ofn.lpstrFilter= ofd.pcszFilter ;
	ofn.lpstrFile= ofd.szFileName ;
	ofn.nMaxFile= sizeof(ofd.szFileName) ;
	ofn.lpstrFileTitle= ofd.szFileTitle ;
	ofn.nMaxFileTitle= sizeof(ofd.szFileTitle) ;
	ofn.lpstrInitialDir= ofd.szFileDir ;
	ofn.Flags= (
		0
	      | OFN_HIDEREADONLY
	      | OFN_FILEMUSTEXIST
	) ;

	if (!GetOpenFileName(&ofn)) return(MZBBOOL_FALSE) ;

	lstrcpy(pszFileName, ofn.lpstrFile) ;
	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclGetSaveFileName(
	HWND hWndOwner,
	char *pszFileName,
	const char *pcszFilter
) {
	struct savefiledata_tag {
		const char *pcszFilter ;
		char szFileName[MZBMAX_FULLFILENAMELEN+1] ;
		char szFileTitle[MZBMAX_FULLFILENAMELEN+1] ;
		char szFileDir[MZBMAX_FULLFILENAMELEN+1] ;
	} sfd ;
	OPENFILENAME ofn ;

	/*
		set save file data
	*/	
	memset(&sfd, '\0', sizeof(sfd)) ;
	sfd.pcszFilter= pcszFilter ;
	lstrcpy(sfd.szFileName, pszFileName) ;

	/*
		set OPENFILENAME structure
	*/
	memset(&ofn, '\0', sizeof(ofn)) ;
	ofn.lStructSize= sizeof(ofn) ;
	ofn.hwndOwner= hWndOwner ;
	ofn.lpstrFilter= sfd.pcszFilter ;
	ofn.lpstrFile= sfd.szFileName ;
	ofn.nMaxFile= sizeof(sfd.szFileName) ;
	ofn.lpstrFileTitle= sfd.szFileTitle ;
	ofn.nMaxFileTitle= sizeof(sfd.szFileTitle) ;
	ofn.lpstrInitialDir= sfd.szFileDir ;
	ofn.Flags= (
		OFN_NOREADONLYRETURN
	      | OFN_HIDEREADONLY
	      | OFN_OVERWRITEPROMPT
	      | OFN_PATHMUSTEXIST
	) ;

	if (!GetSaveFileName(&ofn)) return(MZBBOOL_FALSE) ;

	lstrcpy(pszFileName, ofn.lpstrFile) ;
	return(MZBBOOL_TRUE) ;
}

/* ******************************************************************* */

STATIC MZB_BOOL lclEventCreateWindow(HWND hWnd, LPARAM lParam)
{
	LPCREATESTRUCT lpcs ;
	LPWINCRP lpwincrp ;

	hWndTop= hWnd ;
	lpcs= (LPCREATESTRUCT) lParam ;
	lpwincrp= (LPWINCRP) lpcs->lpCreateParams ;
	memset(&mzbp, '\0', sizeof(MZBP)) ;

	if (lpwincrp->lpszCmdLine) {
		if (lpwincrp->lpszCmdLine[0]) {
			if (
				!(
					(MZB_BOOL) SendMessage(
						hWnd,
						MZBWM_OPENSET,
						0,
						(LPARAM) lpwincrp->lpszCmdLine
					)
				)
			) {
				return(MZBBOOL_FALSE) ;
			}
			PostMessage(hWnd, MZBWM_PLAYSEL, 0, 0L) ;
		}
	}

	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclEventDestroyWindow(HWND hWnd)
{
	/*
		is it necessary to kill these timers?
	*/
	KillTimer(hWnd, MZBTIMER_DELAY) ;
	KillTimer(hWnd, MZBTIMER_STATUS) ;
	lclStopSelection() ;
	closeSet(&mzbp.set) ;
	hWndTop= NULL ;
	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclEventEndSession()
{
	lclWriteProfile() ;
	return(MZBBOOL_TRUE) ;
}

STATIC void lclSetScrollSongPos(MZB_U4 ulSongPos)
{
	double dWork ;
	int iSongPos ;
	HWND hWndScrollSong ;

	if (!hDlgSel) return ;
	hWndScrollSong= GetDlgItem(hDlgSel, IDC_SBSCROLLSONG) ;
	if (!hWndScrollSong) return ;		/* NEVER HAPPENS, RIGHT!? */
	iSongPos= 0 ;
	if (mzbp.ulMaxSongPosition> 0) {
		dWork= (double) ulSongPos ;
		dWork/= (double) mzbp.ulMaxSongPosition ;
		dWork*= (double) MZBSSPOS_MAX ;
		dWork+= (double) 0.5e0 ;
		iSongPos= (int) dWork ;
	}
	SetScrollPos(hWndScrollSong, SB_CTL, iSongPos, TRUE) ;
}

STATIC void lclCalcScrollSongPos(MZB_U4 *puSongPos, int iSongPos)
{
	double dWork ;

	dWork= (double) iSongPos ;
	dWork/= (double) MZBSSPOS_MAX ;
	dWork*= (double) mzbp.ulMaxSongPosition ;
	dWork+= (double) 0.5e0 ;
	(*puSongPos)= (MZB_U4) dWork ;
}

STATIC void lclSetScrollSongRange()
{
	HWND hWndScrollSong ;

	if (!hDlgSel) return ;
	hWndScrollSong= GetDlgItem(hDlgSel, IDC_SBSCROLLSONG) ;
	if (!hWndScrollSong) return ;		/* NEVER HAPPENS, RIGHT!? */

	SetScrollRange(
		hWndScrollSong,
		SB_CTL,
		0,
		MZBSSPOS_MAX,
		FALSE
	) ;
}

STATIC void lclDoScrollSong(WPARAM wScrollCode, int iSongPos)
{
	MZB_U4 ulNewSongPosition ;

	switch(wScrollCode) {

		case	SB_THUMBPOSITION:
			lclCalcScrollSongPos(&ulNewSongPosition, iSongPos) ;
			if (mzbp.bIsPlaying) {
				PostMessage(
					hWndTop,
					MZBWM_SEEKSEL,
					0,
					ulNewSongPosition
				) ;
				break ;
			}
			if (mzbp.bIsPaused) {
				mzbp.ulSongPosition= ulNewSongPosition ;
				lclSetScrollSongPos(mzbp.ulSongPosition) ;
				break ;
			}
			break ;

		case	SB_PAGEDOWN:
			PostMessage(
				hWndTop,
				MZBWM_FFSEL,
				mzbi.uSongScrollIncrement,
				0L
			) ;
			break ;

		case	SB_PAGEUP:
			PostMessage(
				hWndTop,
				MZBWM_REWSEL,
				mzbi.uSongScrollIncrement,
				0L
			) ;
			break ;

		case	SB_LINEDOWN:
			PostMessage(
				hWndTop,
				MZBWM_FFSEL,
				mzbi.uSongFFRewIncrement,
				0L
			) ;
			break ;

		case	SB_LINEUP:
			PostMessage(
				hWndTop,
				MZBWM_REWSEL,
				mzbi.uSongFFRewIncrement,
				0L
			) ;
			break ;
	}
}

STATIC LPSTR lclGetEntryName(int iSelIndex)
{
	if ((iSelIndex< 0) || (iSelIndex>= mzbp.set.iNumEntries)) {
		return((LPSTR) NULL) ;
	}	
	return((LPSTR) mzbp.set.pSetEntry[iSelIndex].szEntryName) ;
}

STATIC LPSTR lclGetEntryComment(int iSelIndex)
{
	if ((iSelIndex< 0) || (iSelIndex>= mzbp.set.iNumEntries)) {
		return((LPSTR) NULL) ;
	}	
	return((LPSTR) mzbp.set.pSetEntry[iSelIndex].szEntryComment) ;
}

STATIC void lclLoadSelList()
{
	int iEntryIdx ;
	DWORD dwIndex ;
	HWND hWndFlist ;

	if (!hDlgSel) return ;

	hWndFlist= GetDlgItem(hDlgSel, IDC_CBFLIST) ;
	if (!hWndFlist) return ;		/* never happens */

	SendMessage(hWndFlist, CB_RESETCONTENT, 0, 0L) ;
	for (iEntryIdx= 0; iEntryIdx< mzbp.set.iNumEntries; iEntryIdx++) {
		dwIndex= SendMessage(
			hWndFlist,
			CB_ADDSTRING,
			(WPARAM) 0,
			(LPARAM) lclGetEntryName(iEntryIdx)
		) ;
		if ((dwIndex == CB_ERR) || (dwIndex == CB_ERRSPACE)) {
			return ;
		}
	}
}

STATIC MZB_BOOL lclStartSelection(MZB_U4 ulStartSongPosition)
{
	if (!lclExtractSelectionIntoTempfile()) return(MZBBOOL_FALSE) ;
	if (!lclPlayResetfile()) return(MZBBOOL_FALSE) ;
	return(lclStartTempfile(ulStartSongPosition)) ;
}

STATIC MZB_BOOL lclStopSelection()
{
	lclStopTempfile() ;
	lclSetScrollSongPos((MZB_U4) 0) ;
	lclEraseTempfile() ;
	MZBUNSET_PAUSED() ;
	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclResumeSelection(MZB_U4 ulResumeSongPosition)
{
	return(lclResumeTempfile(ulResumeSongPosition)) ;
}

STATIC MZB_BOOL lclPauseSelection()
{
	lclStopTempfile() ;
	MZBSET_PAUSED() ;
	return(MZBBOOL_TRUE) ;
}

STATIC void lclSetWindowTitle(HWND hWnd, LPSTR lpszExtra)
{
	char szTitle[MZBMAX_TITLETEXTLEN+1] ;

	lstrcpy(szTitle, cszAppName) ;
	if (mzbp.bIsSetHandLoaded && isSetOpen(&mzbp.set)) {
		lstrcat(szTitle, " [") ;
		lstrcat(szTitle, mzbp.set.szZipFileName) ;
		lstrcat(szTitle, "]") ;
	}
	if (lpszExtra) {
		lstrcat(szTitle, " - ") ;
		lstrcat(szTitle, lpszExtra) ;
	}
	SetWindowText(hWnd, szTitle) ;
}

STATIC void lclMCINotify(HWND hWnd, WPARAM wParam)
{
	switch(wParam) {

		case	MCI_NOTIFY_ABORTED:
			break ;

		case	MCI_NOTIFY_SUCCESSFUL:
			if (mzbp.iSelIndex == (mzbp.set.iNumEntries - 1)) {
				if (!mzbp.bUserPlayed) {
					PostMessage(hWndTop, WM_CLOSE, 0, 0L) ;
					break ;
				}
				PostMessage(hWnd, MZBWM_STOPSEL, 0, 0L) ;
				break ;
			}
			PostMessage(hWnd, MZBWM_NEXTSEL, 0, 0L) ;
			break ;

		default:
			SendMessage(hWnd, MZBWM_STOPSEL, 0, 0L) ;
			if (mzbi.uDebugLevel> 0) {
				lclErrorMsg(
					hWnd,
					"MCI_NOTIFY Error",
					(long) wParam
				) ;
			}
			break ;
	}
}

STATIC MZB_BOOL sendMCIStringNoCheck(const char *pszMCICommand)
{
        mciSendString(pszMCICommand, NULL, 0, hWndTop) ;
	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL sendMCIString(const char *pszMCICommand)
{
	return(sendMCIStringReturnAnswer(pszMCICommand, (char *) NULL, 0)) ;
}

STATIC MZB_BOOL sendMCIStringReturnAnswer(
	const char *pszMCICommand,
	char *pszMCIReturn,
	int iLenReturnMax
) {
	DWORD dwErr ;
	char szMCIReturn[MZBMAX_MCIRETLEN+1] ;
	char szMCIError[MZBMAX_MCIERRLEN+1] ;
	char szMCIErrorExtended[MZBMAX_MCIERREXLEN+1] ;
	int iLen ;
	
	dwErr= mciSendString(
		pszMCICommand,
		szMCIReturn,
		MZBMAX_MCIRETLEN,
		hWndTop
	) ;

	if (dwErr != 0) {
	        mciGetErrorString(dwErr, szMCIError, MZBMAX_MCIERRLEN) ;
		wsprintf(
			szMCIErrorExtended,
			(
				"The MCI Command:\n"
				"   %s\n"
				"\n"
				"Generated the following error:\n"
				"   %s"
			),
			pszMCICommand,
			szMCIError
		) ;
		lclErrorMsg(hWndTop, szMCIErrorExtended, (long) dwErr) ;
		return(MZBBOOL_FALSE) ;
	}

	if (pszMCIReturn) {
		iLen= lstrlen(szMCIReturn) ;
		if (iLen> (iLenReturnMax-1)) {
			iLen= iLenReturnMax - 1 ;
		}
		memcpy(pszMCIReturn, szMCIReturn, iLen) ;
		pszMCIReturn[iLen]= '\0' ;
	}

	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclExtractEntry(
 	int iSelIndex,
	char *pszSaveFileName
) {
	ZL_RC zlrc ;

	zlrc= zlExtractEntry(
		mzbp.set.szZipFileName,
		pszSaveFileName,
		mzbp.set.pSetEntry[iSelIndex].ulLclHdrPos
	) ;
	if (zlrc != ZLRC_SUCCESS) {
		lclEraseTempfile() ;
		return(lclErrorMsg(
			hWndTop,
			MZBMSGPFX_ZIPFILEERROR "zlExtractEntry",
			(long) zlrc
		)) ;
	}
	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclStopTempfile()
{
	if (!mzbp.bIsPlaying) return(MZBBOOL_FALSE) ;
	lclGetSongPointerMCI() ;
	sendMCIStringNoCheck("stop mzb wait") ;
	sendMCIStringNoCheck("close mzb wait") ;
	mzbp.bIsPlaying= MZBBOOL_FALSE ;
	KillTimer(hWndTop, MZBTIMER_STATUS) ;
	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclOpenMCI(LPCSTR lpcszMidiFileName)
{
	char szMCICmd[MZBMAX_MCICMDLEN+1] ;

	wsprintf(
		szMCICmd,
		"open %s type sequencer alias mzb wait",
		lpcszMidiFileName
	) ;
	if (!sendMCIString(szMCICmd)) {
		return(MZBBOOL_FALSE) ;
	}
	if (mzbi.szMCIPortName[0]) {
		wsprintf(
			szMCICmd,
			"set mzb port %s wait",
			mzbi.szMCIPortName
		) ;
		if (!sendMCIString(szMCICmd)) {
			return(MZBBOOL_FALSE) ;
		}
	}
	sendMCIStringNoCheck("set mzb time format ms wait") ;
	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclGetSongPointerMCI()
{
	char szMCISongPointer[MZBMAX_SONGPOINTERLEN+1] ;
	
	if (
		!sendMCIStringReturnAnswer(
			"status mzb position wait",
			szMCISongPointer,
			sizeof(szMCISongPointer)
		)
	) {
		return(MZBBOOL_FALSE) ;
	}
	mzbp.ulSongPosition= lclGetAsciiNumber(szMCISongPointer) ;
	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclPlayResetfile()
{
	if (!mzbi.szResetFileName[0]) return(MZBBOOL_TRUE) ;
	if (!lclOpenMCI(mzbi.szResetFileName)) return(MZBBOOL_FALSE) ;
	sendMCIStringNoCheck("play mzb wait") ;
	sendMCIStringNoCheck("close mzb wait") ;
	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclOpenTempfile()
{
	if (mzbp.bIsPlaying) return(MZBBOOL_TRUE) ;
	if (!lclOpenMCI(mzbp.szTempFileName)) return(MZBBOOL_FALSE) ;
	mzbp.bIsPlaying= MZBBOOL_TRUE ;
	return(MZBBOOL_TRUE) ;
}

STATIC void lclInitScrollSong(
	MZB_U4 u4InitialSongPosition,
	DWORD dwStartTick
) {
	mzbp.ulSongPosition= u4InitialSongPosition ;
	mzbp.dwSongStartTick= dwStartTick - mzbp.ulSongPosition ;

	lclSetScrollSongPos(mzbp.ulSongPosition) ;
	if (mzbi.uStatusMsec> 0) {
		SetTimer(
			hWndTop,
			MZBTIMER_STATUS,
			(UINT) mzbi.uStatusMsec,
			NULL
		) ;
	}
}

static char cszMCIPlayFormat[]= "play mzb from %ld notify" ;

STATIC MZB_BOOL lclStartTempfile(MZB_U4 ulStartSongPosition)
{
	char szMCISongLength[MZBMAX_SONGPOINTERLEN+1] ;
	char szMCICmd[MZBMAX_MCICMDLEN+1] ;
	DWORD dwStartTick ;

	if (!lclOpenTempfile()) return(MZBBOOL_FALSE) ;
	if (
		!sendMCIStringReturnAnswer(
			"status mzb length wait",
			szMCISongLength,
			sizeof(szMCISongLength)
		)
	) {
		return(MZBBOOL_FALSE) ;
	}

	wsprintf(szMCICmd, cszMCIPlayFormat, ulStartSongPosition) ;
	if (!sendMCIString(szMCICmd)) return(MZBBOOL_FALSE) ;

	dwStartTick= GetTickCount() ;
	mzbp.ulMaxSongPosition= lclGetAsciiNumber(szMCISongLength) ;

#if 0
	THIS MAY BE REMOVED - was moved to dialog init code
	lclSetScrollSongRange() ;
#endif

	lclInitScrollSong(ulStartSongPosition, dwStartTick) ;

	MZBUNSET_PAUSED() ;

	return(MZBBOOL_TRUE) ;
}

STATIC MZB_BOOL lclResumeTempfile(MZB_U4 ulStartSongPosition)
{
	char szMCICmd[MZBMAX_MCICMDLEN+1] ;

	if (!lclOpenTempfile()) return(MZBBOOL_FALSE) ;

	wsprintf(szMCICmd, cszMCIPlayFormat, ulStartSongPosition) ;
	if (!sendMCIString(szMCICmd)) return(MZBBOOL_FALSE) ;

	lclInitScrollSong(ulStartSongPosition, GetTickCount()) ;

	MZBUNSET_PAUSED() ;

	return(MZBBOOL_TRUE) ;
}

STATIC void lclEraseTempfile()
{
	if (mzbp.szTempFileName[0]) {
		unlink(mzbp.szTempFileName) ;
		mzbp.szTempFileName[0]= '\0' ;
	}
}

STATIC MZB_BOOL lclExtractSelectionIntoTempfile()
{
	const char *pcszEnvTempDir ;

	if (mzbp.szTempFileName[0]) {
		return(MZBBOOL_FALSE) ;
	}

	pcszEnvTempDir= (
		mzbi.szTempDirName[0]
	      ? mzbi.szTempDirName
	      : pszEnvTempDir
	) ;

	mzbp.szTempFileName[0]= '\0' ;		/* pedantic */
	if (pcszEnvTempDir) {
		lstrcat(mzbp.szTempFileName, pcszEnvTempDir) ;
		if (
			'\\'
		     != mzbp.szTempFileName[lstrlen(mzbp.szTempFileName)-1]
		) {
			lstrcat(mzbp.szTempFileName, "\\") ;
		}
	}

	lstrcat(mzbp.szTempFileName, cszTempMIDIFileName) ;
	return(lclExtractEntry(mzbp.iSelIndex, mzbp.szTempFileName)) ;
}

/*********************************************************************/

STATIC void lclSeekSelection(MZB_U4 ulNewSongPosition)
{
	if (ulNewSongPosition> mzbp.ulMaxSongPosition) {
		ulNewSongPosition= mzbp.ulMaxSongPosition ;
	}
	lclPlaySelectedFile(ulNewSongPosition) ;
}

STATIC void lclFFSelection(MZB_U2 uSongFFIncrement)
{
	if (mzbp.bIsPlaying) lclGetSongPointerMCI() ;
	lclSeekSelection(mzbp.ulSongPosition + (MZB_U4) uSongFFIncrement) ;
}

STATIC void lclRewSelection(MZB_U2 uSongRewIncrement)
{
	MZB_U4 uNewSongPosition ;

	if (mzbp.bIsPlaying) lclGetSongPointerMCI() ;
	uNewSongPosition= (
		(mzbp.ulSongPosition> (MZB_U4) uSongRewIncrement)
		?	(mzbp.ulSongPosition - (MZB_U4) uSongRewIncrement)
		:	0
	) ;
	lclSeekSelection(uNewSongPosition) ;
}

STATIC MZB_U4 lclGetAsciiNumber(const char *pcszAscii)
{
	return(pcszAscii[0]? (MZB_U4) atol(pcszAscii): 0L) ;
}

/*********************************************************************/

STATIC MZB_BOOL lclCopySetInto(HWND hWndOwner, char *pszSaveFileName)
{
	FILE *fpIn ;
	FILE *fpOut ;
	int c ;

	fpIn= fopen(mzbp.set.szZipFileName, "rb") ;
	if (!fpIn) {
		return(lclErrorMsg(hWndOwner, "Can't open Set", 0L)) ;
	}

	fpOut= fopen(pszSaveFileName, "wb") ;
	if (!fpOut) {
		fclose(fpIn) ;
		return(lclErrorMsg(hWndOwner, "Can't open output Set", 0L)) ;
	}

	while((c= getc(fpIn)) != EOF) {
		putc(c, fpOut) ;
	}

	fclose(fpOut) ;
	fclose(fpIn) ;
	return(MZBBOOL_TRUE) ;
}

/*
	this stuff will get split into its own module...
*/


/*
	internal function decls
*/

STATIC ZL_BOOL myProcessEntry(
	const struct zl_file *pzlf,
	const struct zl_entry *pzle,
	void *pUser
) ;

STATIC ZL_BOOL myProcessCDir(
	const struct zl_file *pzlf,
	const struct zl_entry *pzle,
	void *pUser
) ;

STATIC void fillInEntry(
	struct mzb_entry *pSetEntry,
	const struct zl_entry *pzle
) ;


/*
	entry points
*/

STATIC MZB_BOOL openSet(struct mzb_set *pSet, LPCSTR lpcszZipFileName)
{
	ZL_RC zrc ;
	struct zl_parms zlp ;
	int iLenZipFileName ;

	iLenZipFileName= lstrlen(lpcszZipFileName) ;
	if (iLenZipFileName> sizeof(pSet->szZipFileName)) {
		lclErrorMsg(NULL, "File name too long", 0L) ;
		return(MZBBOOL_FALSE) ;
	}

	memset(pSet, '\0', sizeof(struct mzb_set)) ;
	memcpy(pSet->szZipFileName, lpcszZipFileName, (size_t) iLenZipFileName) ;
	pSet->szZipFileName[iLenZipFileName]= '\0' ;

	memset(&zlp, '\0', sizeof(zlp)) ;
	zlp.fnProcessEntry= myProcessEntry ;
	zlp.fnProcessCDir= myProcessCDir ;
	zlp.pszZipFileComment= pSet->szZipFileComment ;
	zlp.lMaxLenZipFileComment= MZBMAX_ZIPCOMMENTLEN ;
	zlp.pUser= pSet ;
	
	zrc= zlReadZip(pSet->szZipFileName, &zlp) ;
	if (zrc != ZLRC_SUCCESS) {		       
		return(lclErrorMsg(
			hWndTop,
			MZBMSGPFX_ZIPFILEERROR "zlReadZip",
			(long) zrc
		)) ;
	}
	return(MZBBOOL_TRUE) ;
}

STATIC void closeSet(struct mzb_set *pSet)
{
	if (pSet->pSetEntry) free((void *) pSet->pSetEntry) ;
	memset(pSet, '\0', sizeof(struct mzb_set)) ;
}

STATIC MZB_BOOL isSetOpen(struct mzb_set *pSet)
{
	return((MZB_BOOL) (pSet->pSetEntry != (struct mzb_entry *) 0)) ;
}


/*
	internal functions
*/

STATIC ZL_BOOL myProcessEntry(
	const struct zl_file *pzlf,
	const struct zl_entry *pzle,
	void *pUser
) {
	struct mzb_set *pSet= (struct mzb_set *) pUser ;
	pSet->iNumEntries++ ;
	return(ZLBOOL_TRUE) ;
}

STATIC ZL_BOOL myProcessCDir(
	const struct zl_file *pzlf,
	const struct zl_entry *pzle,
	void *pUser
) {
	struct mzb_set *pSet= (struct mzb_set *) pUser ;
	struct mzb_entry *pSetEntry= pSet->pSetEntry ;
	size_t size ;
 
	if (!pSetEntry) {
		size= (size_t) (pSet->iNumEntries * sizeof(struct mzb_entry)) ;
		pSetEntry= (struct mzb_entry *) malloc(size) ;
		if (!pSetEntry) return(ZLBOOL_FALSE) ;
		memset(pSetEntry, '\0', size) ;
		pSet->pSetEntry= pSetEntry ;
		pSet->iEntryIdx= 0 ;
	}

	if ((pSet->iEntryIdx)>= (pSet->iNumEntries)) {
		return(ZLBOOL_FALSE) ;
	}

	fillInEntry(pSetEntry + pSet->iEntryIdx++, pzle) ;
	return(ZLBOOL_TRUE) ;
}

STATIC void getLimitedString(
	const char *pcszFullString,
	char *pszLimitedString,
	int iLenMax,
	MZB_BOOL bRightJustify
) {
	int iLenFullString, iLenLimitedString ;
	int iDotOffset, iStringOffset, iSourceOffset ;

	iLenFullString= lstrlen(pcszFullString) ;

	if (iLenFullString<= iLenMax) {
		memcpy(
			pszLimitedString,
			pcszFullString,
			(size_t) iLenFullString
		) ;
		pszLimitedString[iLenFullString]= '\0' ;
		return ;
	}

	iLenLimitedString= iLenMax - 3 ;

	if (bRightJustify) {
		iDotOffset= 0 ;
		iStringOffset= 3 ;
		iSourceOffset= iLenFullString - iLenLimitedString ;
	}
	else {
		iDotOffset= iLenMax - 3 ;
		iStringOffset= 0 ;
		iSourceOffset= 0 ;
	}

	memcpy(pszLimitedString + iDotOffset, "...", 3) ;
	memcpy(
		pszLimitedString + iStringOffset,
		pcszFullString + iSourceOffset,
		(size_t) iLenLimitedString
	) ;

	pszLimitedString[iLenMax++]= '\0' ;
}	

STATIC void fillInEntry(
	struct mzb_entry *pSetEntry,
	const struct zl_entry *pzle
) {
	pSetEntry->ulLclHdrPos= (unsigned long) pzle->lhdrLoc ;
	getLimitedString(
		pzle->pszFileName,
		pSetEntry->szEntryName,
		MZBMAX_ENTRYFILENAMELEN,
		MZBBOOL_TRUE			/* ...ylongfilename */
	) ;
	getLimitedString(
		pzle->pszFileComment,
		pSetEntry->szEntryComment,
		MZBMAX_ENTRYCOMMENTLEN,
		MZBBOOL_FALSE			/* verylongfileco... */
	) ;
}

/* end of mzb.c */
