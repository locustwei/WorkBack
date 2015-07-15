#include "sendkeys.h"

/* 
* ----------------------------------------------------------------------------- 
* Copyright (c) 2004 lallous <lallousx86@yahoo.com>
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
* ----------------------------------------------------------------------------- 


The Original SendKeys copyright info
---------------------------------------
SendKeys (sndkeys32.pas) routine for 32-bit Delphi.
Written by Ken Henderson
Copyright (c) 1995 Ken Henderson <khen@compuserve.com>

History
----------
04/19/2004
  * Initial version development
04/21/2004
  * Added number of times specifier to special keys
  * Added {BEEP X Y}
  * Added {APPACTIVATE WindowTitle}
  * Added CarryDelay() and now delay works properly with all keys
  * Added SetDelay() method
  * Fixed code in AppActivate that allowed to pass both NULL windowTitle/windowClass

05/21/2004
  * Fixed a bug in StringToVKey() that caused the search for RIGHTPAREN to be matched as RIGHT
  * Adjusted code so it compiles w/ VC6
05/24/2004
  * Added unicode support

Todo
-------
* perhaps add mousecontrol: mousemove+mouse clicks
* allow sending of normal keys multiple times as: {a 10}

*/

const WORD CSendKeys::VKKEYSCANSHIFTON = 0x01;
const WORD CSendKeys::VKKEYSCANCTRLON  = 0x02;
const WORD CSendKeys::VKKEYSCANALTON   = 0x04;
const WORD CSendKeys::INVALIDKEY       = 0xFFFF;
bool CSendKeys::m_bWait = false; 
bool CSendKeys::m_bUsingParens = false;
bool CSendKeys::m_bShiftDown = false; 
bool CSendKeys::m_bAltDown = false;
bool CSendKeys::m_bControlDown = false;
bool CSendKeys::m_bWinDown = false;
DWORD CSendKeys::m_nDelayAlways = 0;
DWORD CSendKeys::m_nDelayNow = 0;

const BYTE CSendKeys::ExtendedVKeys[MaxExtendedVKeys] =
{
    VK_UP, 
    VK_DOWN,
    VK_LEFT,
    VK_RIGHT,
    VK_HOME,
    VK_END,
    VK_PRIOR, // PgUp
    VK_NEXT,  //  PgDn
    VK_INSERT,
    VK_DELETE
};

CSendKeys::CSendKeys()
{
  m_nDelayNow = m_nDelayAlways = 0;
}

// Delphi port regexps:
// ---------------------
// search: .+Name:'([^']+)'.+vkey:([^\)]+)\)
// replace: {"\1", \2}
//
// **If you add to this list, you must be sure to keep it sorted alphabetically
// by Name because a binary search routine is used to scan it.**
//
CSendKeys::key_desc_t CSendKeys::KeyNames[CSendKeys::MaxSendKeysRecs] = 
{
  {"ADD", VK_ADD}, 
  {"APPS", VK_APPS},
  {"AT", '@', true},
  {"BACKSPACE", VK_BACK},
  {"BKSP", VK_BACK},
  {"BREAK", VK_CANCEL},
  {"BS", VK_BACK},
  {"CAPSLOCK", VK_CAPITAL},
  {"CARET", '^', true},
  {"CLEAR", VK_CLEAR},
  {"DECIMAL", VK_DECIMAL}, 
  {"DEL", VK_DELETE},
  {"DELETE", VK_DELETE},
  {"DIVIDE", VK_DIVIDE}, 
  {"DOWN", VK_DOWN},
  {"END", VK_END},
  {"ENTER", VK_RETURN},
  {"ESC", VK_ESCAPE},
  {"ESCAPE", VK_ESCAPE},
  {"F1", VK_F1},
  {"F10", VK_F10},
  {"F11", VK_F11},
  {"F12", VK_F12},
  {"F13", VK_F13},
  {"F14", VK_F14},
  {"F15", VK_F15},
  {"F16", VK_F16},
  {"F2", VK_F2},
  {"F3", VK_F3},
  {"F4", VK_F4},
  {"F5", VK_F5},
  {"F6", VK_F6},
  {"F7", VK_F7},
  {"F8", VK_F8},
  {"F9", VK_F9},
  {"HELP", VK_HELP},
  {"HOME", VK_HOME},
  {"INS", VK_INSERT},
  {"LEFT", VK_LEFT},
  {"LEFTBRACE", '{', true},
  {"LEFTPAREN", '(', true},
  {"LWIN", VK_LWIN},
  {"MULTIPLY", VK_MULTIPLY}, 
  {"NUMLOCK", VK_NUMLOCK},
  {"NUMPAD0", VK_NUMPAD0}, 
  {"NUMPAD1", VK_NUMPAD1}, 
  {"NUMPAD2", VK_NUMPAD2}, 
  {"NUMPAD3", VK_NUMPAD3}, 
  {"NUMPAD4", VK_NUMPAD4}, 
  {"NUMPAD5", VK_NUMPAD5}, 
  {"NUMPAD6", VK_NUMPAD6}, 
  {"NUMPAD7", VK_NUMPAD7}, 
  {"NUMPAD8", VK_NUMPAD8}, 
  {"NUMPAD9", VK_NUMPAD9}, 
  {"PERCENT", '%', true},
  {"PGDN", VK_NEXT},
  {"PGUP", VK_PRIOR},
  {"PLUS", '+', true},
  {"PRTSC", VK_PRINT},
  {"RIGHT", VK_RIGHT},
  {"RIGHTBRACE", '}', true},
  {"RIGHTPAREN", ')', true},
  {"RWIN", VK_RWIN},
  {"SCROLL", VK_SCROLL},
  {"SEPARATOR", VK_SEPARATOR}, 
  {"SNAPSHOT", VK_SNAPSHOT},
  {"SUBTRACT", VK_SUBTRACT}, 
  {"TAB", VK_TAB},
  {"TILDE", '~', true}, 
  {"UP", VK_UP},
  {"WIN", VK_LWIN}
};


// calls keybd_event() and waits, if needed, till the sent input is processed
void CSendKeys::KeyboardEvent(BYTE VKey, BYTE ScanCode, LONG Flags)
{
  MSG KeyboardMsg;

  keybd_event(VKey, ScanCode, Flags, 0);

  if (m_bWait)
  {
    while (::PeekMessage(&KeyboardMsg, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
    {
      ::TranslateMessage(&KeyboardMsg);
      ::DispatchMessage(&KeyboardMsg);
    }
  }
}

// Checks whether the specified VKey is an extended key or not
bool CSendKeys::IsVkExtended(BYTE VKey)
{
  for (int i=0;i<MaxExtendedVKeys;i++)
  {
    if (ExtendedVKeys[i] == VKey)
      return true;
  }
  return false;
}

// Generates KEYUP
void CSendKeys::SendKeyUp(BYTE VKey)
{
  BYTE ScanCode = LOBYTE(::MapVirtualKey(VKey, 0));

  KeyboardEvent(VKey, 
                ScanCode, 
                KEYEVENTF_KEYUP | (IsVkExtended(VKey) ? KEYEVENTF_EXTENDEDKEY : 0));
}

void CSendKeys::SendKeyDown(BYTE VKey, WORD NumTimes, bool GenUpMsg, bool bDelay)
{
  WORD Cnt = 0;
  BYTE ScanCode = 0;
  bool NumState = false;

  if (VKey == VK_NUMLOCK)
  {
    DWORD dwVersion = ::GetVersion();

    for (Cnt=1; Cnt<=NumTimes; Cnt++)
    {
      if (bDelay)
        CarryDelay();
      // snippet based on:
      // http://www.codeproject.com/cpp/togglekeys.asp
      if (dwVersion < 0x80000000)
      {
        ::keybd_event(VKey, 0x45, KEYEVENTF_EXTENDEDKEY, 0);
        ::keybd_event(VKey, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
      }
      else
      {
        // Win98 and later
        if ( ((DWORD)(HIBYTE(LOWORD(dwVersion))) >= 10) )
        {
          // Define _WIN32_WINNT > 0x0400
          // to compile
          INPUT input[2] = {0};
          input[0].type = input[1].type = INPUT_KEYBOARD;
          input[0].ki.wVk = input[1].ki.wVk = VKey;
          input[1].ki.dwFlags = KEYEVENTF_KEYUP;
          ::SendInput(sizeof(input) / sizeof(INPUT), input, sizeof(INPUT));
        }
        // Win95
        else
        {
          KEYBOARDSTATE_t KeyboardState;
          NumState = GetKeyState(VK_NUMLOCK) & 1 ? true : false;
          GetKeyboardState(&KeyboardState[0]);
          if (NumState)
            KeyboardState[VK_NUMLOCK] &= ~1;
          else
            KeyboardState[VK_NUMLOCK] |= 1;

          SetKeyboardState(&KeyboardState[0]);
        }
      }
    }
    return;
  }

  // Get scancode
  ScanCode = LOBYTE(::MapVirtualKey(VKey, 0));

  // Send keys
  for (Cnt=1; Cnt<=NumTimes; Cnt++)
  {
    // Carry needed delay ?
    if (bDelay)
      CarryDelay();

    KeyboardEvent(VKey, ScanCode, IsVkExtended(VKey) ? KEYEVENTF_EXTENDEDKEY : 0);
    if (GenUpMsg)
      KeyboardEvent(VKey, ScanCode, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP);
  }
}

// Checks whether a bit is set
bool CSendKeys::BitSet(BYTE BitTable, UINT BitMask)
{
  return BitTable & BitMask ? true : false;
}

// Sends a single key
void CSendKeys::SendKey(WORD MKey, WORD NumTimes, bool GenDownMsg)
{
  // Send appropriate shift keys associated with the given VKey
  if (BitSet(HIBYTE(MKey), VKKEYSCANSHIFTON))
    SendKeyDown(VK_SHIFT, 1, false);

  if (BitSet(HIBYTE(MKey), VKKEYSCANCTRLON))
    SendKeyDown(VK_CONTROL, 1, false);

  if (BitSet(HIBYTE(MKey), VKKEYSCANALTON))
    SendKeyDown(VK_MENU, 1, false);

  // Send the actual VKey
  SendKeyDown(LOBYTE(MKey), NumTimes, GenDownMsg, true);

  // toggle up shift keys
  if (BitSet(HIBYTE(MKey), VKKEYSCANSHIFTON))
    SendKeyUp(VK_SHIFT);

  if (BitSet(HIBYTE(MKey), VKKEYSCANCTRLON))
    SendKeyUp(VK_CONTROL);

  if (BitSet(HIBYTE(MKey), VKKEYSCANALTON))
    SendKeyUp(VK_MENU);
}

// Implements a simple binary search to locate special key name strings
WORD CSendKeys::StringToVKey(LPCSTR KeyString, int &idx)
{
  bool Found = false, Collided;
  int  Bottom = 0, 
       Top = MaxSendKeysRecs,
       Middle = (Bottom + Top) / 2;
  WORD retval = INVALIDKEY;

  idx    = -1;

  do
  {
    Collided = (Bottom == Middle) || (Top == Middle);
    int cmp = _strnicmp(KeyNames[Middle].keyName, KeyString, strlen(KeyString));
    if (cmp == 0)
    {
      Found = true;
      retval = KeyNames[Middle].VKey;
      idx    = Middle;
      break;
    }
    else
    {
      if (cmp < 0)
        Bottom = Middle;
      else
        Top = Middle;
      Middle = (Bottom + Top) / 2;
    }
  } while (!(Found || Collided));

  return retval;
}

// Releases all shift keys (keys that can be depressed while other keys are being pressed
// If we are in a modifier group this function does nothing
void CSendKeys::PopUpShiftKeys()
{
  if (!m_bUsingParens)
  {
    if (m_bShiftDown)
      SendKeyUp(VK_SHIFT);
    if (m_bControlDown)
      SendKeyUp(VK_CONTROL);
    if (m_bAltDown)
      SendKeyUp(VK_MENU);
    if (m_bWinDown)
      SendKeyUp(VK_LWIN);
    m_bWinDown = m_bShiftDown = m_bControlDown = m_bAltDown = false;
  }
}

// Sends a key string
bool CSendKeys::SendKeys(LPCSTR KeysString, bool Wait)
{
  WORD MKey, NumTimes;
  char KeyString[300] = {0};
  bool retval  = false;
  int  keyIdx;

  LPSTR pKey = (LPSTR) KeysString;
  char  ch;

  m_bWait = Wait;

  m_bWinDown = m_bShiftDown = m_bControlDown = m_bAltDown = m_bUsingParens = false;

  while (ch = *pKey)
  {
    switch (ch)
    {
    // begin modifier group
    case _TXCHAR('('):
      m_bUsingParens = true;
      break;

    // end modifier group
    case _TXCHAR(')'):
      m_bUsingParens = false;
      PopUpShiftKeys(); // pop all shift keys when we finish a modifier group close
      break;

    // ALT key
    case _TXCHAR('%'):
      m_bAltDown = true;
      SendKeyDown(VK_MENU, 1, false);
      break;

    // SHIFT key
    case _TXCHAR('+'):
      m_bShiftDown = true;
      SendKeyDown(VK_SHIFT, 1, false);
      break;

    // CTRL key
    case _TXCHAR('^'):
      m_bControlDown = true;
      SendKeyDown(VK_CONTROL, 1, false);
      break;

    // WINKEY (Left-WinKey)
    case '@':
      m_bWinDown = true;
      SendKeyDown(VK_LWIN, 1, false);
      break;

    // enter
    case _TXCHAR('~'):
      SendKeyDown(VK_RETURN, 1, true);
      PopUpShiftKeys();
      break;

    // begin special keys
    case _TXCHAR('{'):
      {
        LPSTR p = pKey+1; // skip past the beginning '{'
        size_t t;

        // find end of close
        while (*p && *p != _TXCHAR('}'))
          p++;

        t = p - pKey;
        // special key definition too big?
        if (t > sizeof(KeyString))
          return false;

        // Take this KeyString into local buffer
        strncpy_s(KeyString, pKey+1, t);

        KeyString[t-1] = _TXCHAR('\0');
        keyIdx = -1;

        pKey += t; // skip to next keystring

        // Invalidate key
        MKey = INVALIDKEY;

        // sending arbitrary vkeys?
        if (_strnicmp(KeyString, "VKEY", 4) == 0)
        {
          p = KeyString + 4;
          MKey = atoi(p);
        }
        else if (_strnicmp(KeyString, "BEEP", 4) == 0)
        {
          p = KeyString + 4 + 1;
          LPSTR p1 = p;
          DWORD frequency, delay;

          if ((p1 = strstr(p, " ")) != NULL)
          {
            *p1++ = _TXCHAR('\0');
            frequency = atoi(p);
            delay = atoi(p1);
            ::Beep(frequency, delay);
          }
        }
        // Should activate a window?
        else if (_strnicmp(KeyString, "APPACTIVATE", 11) == 0)
        {
          p = KeyString + 11 + 1;
        }
        // want to send/set delay?
        else if (_strnicmp(KeyString, "DELAY", 5) == 0)
        {
          // Advance to parameters
          p = KeyString + 5;
          // set "sleep factor"
          if (*p == _TXCHAR('='))
            m_nDelayAlways = atoi(p + 1); // Take number after the '=' character
          else
            // set "sleep now"
            m_nDelayNow = atoi(p);
        }
        // not command special keys, then process as keystring to VKey
        else
        {
          MKey = StringToVKey(KeyString, keyIdx);
          // Key found in table
          if (keyIdx != -1)
          {
            NumTimes = 1;

            // Does the key string have also count specifier?
            t = strlen(KeyNames[keyIdx].keyName);
            if (strlen(KeyString) > t)
            {
              p = KeyString + t;
              // Take the specified number of times
              NumTimes = atoi(p);
            }

            if (KeyNames[keyIdx].normalkey)
              MKey = ::VkKeyScan(KeyNames[keyIdx].VKey);
          }
        }

        // A valid key to send?
        if (MKey != INVALIDKEY)
        {
          SendKey(MKey, NumTimes, true);
          PopUpShiftKeys();
        }
      }
      break;

      // a normal key was pressed
    default:
      // Get the VKey from the key
      MKey = ::VkKeyScan(ch);
      SendKey(MKey, 1, true);
      PopUpShiftKeys();
    }
    pKey++;
  }

  m_bUsingParens = false;
  PopUpShiftKeys();
  return true;
}


// Carries the required delay and clears the m_nDelaynow value
void CSendKeys::CarryDelay()
{
  // Should we delay once?
  if (!m_nDelayNow)
    // should we delay always?
    m_nDelayNow = m_nDelayAlways;

  // No delay specified?
  if (m_nDelayNow)
    ::Sleep(m_nDelayNow); //::Beep(100, m_nDelayNow);

  // clear SleepNow
  m_nDelayNow = 0;
}

/*
Test Binary search
void CSendKeys::test()
{
  WORD miss(0);
  for (int i=0;i<MaxSendKeysRecs;i++)
  {
    char *p = (char *)KeyNames[i].keyName;
    WORD v = StringToVKeyB(p);
    if (v == INVALIDKEY)
    {
      miss++;
    }
  }
}
*/

/*
Search in a linear manner
WORD CSendKeys::StringToVKey(const char *KeyString, int &idx)
{
for (int i=0;i<MaxSendKeysRecs;i++)
{
size_t len = strlen(KeyNames[i].keyName);
if (strnicmp(KeyNames[i].keyName, KeyString, len) == 0)
{
idx = i;
return KeyNames[i].VKey;
}
}
idx = -1;
return INVALIDKEY;
}
*/
