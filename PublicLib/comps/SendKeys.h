#ifndef __SENDKEYS_04192004__INC__
#define __SENDKEYS_04192004__INC__

#include <windows.h>
#include <tchar.h>
// Please see SendKeys.cpp for copyright and usage issues.

class CSendKeys
{
private:
	CSendKeys();

	static bool m_bWait, m_bUsingParens, m_bShiftDown, m_bAltDown, m_bControlDown, m_bWinDown;
	static DWORD  m_nDelayAlways, m_nDelayNow;

	static void CarryDelay();

	typedef BYTE KEYBOARDSTATE_t[256];

	static struct key_desc_t
	{
		LPCSTR keyName;
		BYTE VKey;
		bool normalkey; // a normal character or a VKEY ?
	};

	static enum
	{
		MaxSendKeysRecs  = 71,
		MaxExtendedVKeys = 10
	};

	/*
	Reference: VkKeyScan() / MSDN
	Bit Meaning 
	--- --------
	1   Either SHIFT key is pressed. 
	2   Either CTRL key is pressed. 
	4   Either ALT key is pressed. 
	8   The Hankaku key is pressed 
	16  Reserved (defined by the keyboard layout driver). 
	32  Reserved (defined by the keyboard layout driver). 
	*/
	static const WORD VKKEYSCANSHIFTON;
	static const WORD VKKEYSCANCTRLON;
	static const WORD VKKEYSCANALTON;
	static const WORD INVALIDKEY;

	static key_desc_t KeyNames[MaxSendKeysRecs]; 
	static const BYTE ExtendedVKeys[MaxExtendedVKeys];

	static bool BitSet(BYTE BitTable, UINT BitMask);

	static void PopUpShiftKeys();

	static bool IsVkExtended(BYTE VKey);
	static void SendKeyUp(BYTE VKey);
	static void SendKeyDown(BYTE VKey, WORD NumTimes, bool GenUpMsg, bool bDelay = false);
	static WORD StringToVKey(LPCSTR KeyString, int &idx);
	static void KeyboardEvent(BYTE VKey, BYTE ScanCode, LONG Flags);
public:

	static void SendKey(WORD MKey, WORD NumTimes, bool GenDownMsg);
	static bool SendKeys(LPCSTR KeysString, bool Wait = false);
	//static bool AppActivate(HWND wnd);
	//static bool AppActivate(LPCSTR WindowTitle, LPSTR WindowClass = 0);
	static void SetDelay(const DWORD delay) { m_nDelayAlways = delay; }
};

#endif