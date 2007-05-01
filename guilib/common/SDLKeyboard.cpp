#include "../include.h"
#include "../Key.h"
#include "SDLKeyboard.h"

#define FRAME_DELAY 5

CKeyboard g_Keyboard; // global

CKeyboard::CKeyboard()
{
  m_bShift = false;
  m_bCtrl = false;
  m_bAlt = false;
  m_cAscii = '\0';
  m_VKey = 0;
}

CKeyboard::~CKeyboard()
{
}

void CKeyboard::Initialize(HWND hWnd)
{
  SDL_EnableUNICODE(1);
  Acquire();
}

void CKeyboard::Update()
{  
  memset(&m_keyEvent, 0, sizeof(m_keyEvent));
  if (SDL_PeepEvents(&m_keyEvent, 1, SDL_GETEVENT, SDL_KEYUPMASK | SDL_KEYDOWNMASK) > 0)
  { 
    m_cAscii = 0;
    m_bShift = false;
    m_bCtrl = false;
    m_bAlt = false;
    m_VKey = 0;
    
    if (m_keyEvent.type == SDL_KEYDOWN)
    {
    	m_bCtrl = m_keyEvent.key.keysym.mod & KMOD_CTRL;
    	m_bShift = m_keyEvent.key.keysym.mod & KMOD_SHIFT;
    	m_bAlt = m_keyEvent.key.keysym.mod & KMOD_ALT;

		if (m_keyEvent.key.keysym.unicode < 0x80 && m_keyEvent.key.keysym.unicode > 0)
		{
			m_cAscii = m_keyEvent.key.keysym.unicode;
			m_VKey = m_cAscii | KEY_ASCII;
		}
		else
		{
			if (m_keyEvent.key.keysym.sym == SDLK_KP0) m_VKey = 0x60;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP1) m_VKey = 0x61;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP2) m_VKey = 0x62;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP3) m_VKey = 0x63;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP4) m_VKey = 0x64;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP5) m_VKey = 0x65;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP6) m_VKey = 0x66;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP7) m_VKey = 0x67;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP8) m_VKey = 0x68;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP9) m_VKey = 0x69;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP_ENTER) m_VKey = 0x6C;
			else if (m_keyEvent.key.keysym.sym == SDLK_UP) m_VKey = 0x26;
			else if (m_keyEvent.key.keysym.sym == SDLK_DOWN) m_VKey = 0x28;
			else if (m_keyEvent.key.keysym.sym == SDLK_LEFT) m_VKey = 0x25;
			else if (m_keyEvent.key.keysym.sym == SDLK_RIGHT) m_VKey = 0x27;
			else if (m_keyEvent.key.keysym.sym == SDLK_INSERT) m_VKey = 0x2D;
			else if (m_keyEvent.key.keysym.sym == SDLK_DELETE) m_VKey = 0x2E;
			else if (m_keyEvent.key.keysym.sym == SDLK_HOME) m_VKey = 0x24;
			else if (m_keyEvent.key.keysym.sym == SDLK_END) m_VKey = 0x23;
			else if (m_keyEvent.key.keysym.sym == SDLK_F1) m_VKey = 0x70;
			else if (m_keyEvent.key.keysym.sym == SDLK_F2) m_VKey = 0x71;
			else if (m_keyEvent.key.keysym.sym == SDLK_F3) m_VKey = 0x72;
			else if (m_keyEvent.key.keysym.sym == SDLK_F4) m_VKey = 0x73;
			else if (m_keyEvent.key.keysym.sym == SDLK_F5) m_VKey = 0x74;
			else if (m_keyEvent.key.keysym.sym == SDLK_F6) m_VKey = 0x75;
			else if (m_keyEvent.key.keysym.sym == SDLK_F7) m_VKey = 0x76;
			else if (m_keyEvent.key.keysym.sym == SDLK_F8) m_VKey = 0x77;
			else if (m_keyEvent.key.keysym.sym == SDLK_F9) m_VKey = 0x78;
			else if (m_keyEvent.key.keysym.sym == SDLK_F10) m_VKey = 0x79;
			else if (m_keyEvent.key.keysym.sym == SDLK_F11) m_VKey = 0x7a;
			else if (m_keyEvent.key.keysym.sym == SDLK_F12) m_VKey = 0x7b;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP_PERIOD) m_VKey = 0x6e;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP_MULTIPLY) m_VKey = 0x6a;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP_MINUS) m_VKey = 0x6d;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP_PLUS) m_VKey = 0x6b;
			else if (m_keyEvent.key.keysym.sym == SDLK_KP_DIVIDE) m_VKey = 0x6f;
			else if (m_keyEvent.key.keysym.sym == SDLK_PAGEUP) m_VKey = 0x21;
			else if (m_keyEvent.key.keysym.sym == SDLK_PAGEDOWN) m_VKey = 0x22;
			else if (m_keyEvent.key.keysym.sym == SDLK_LSUPER) m_VKey = 0x5b;
			else if (m_keyEvent.key.keysym.sym == SDLK_RSUPER) m_VKey = 0x5c;
			else if (m_keyEvent.key.keysym.mod && KMOD_LSHIFT) m_VKey = 0xa0;
			else if (m_keyEvent.key.keysym.mod && KMOD_RSHIFT) m_VKey = 0xa1;
			else if (m_keyEvent.key.keysym.mod && KMOD_LALT) m_VKey = 0xa4;
			else if (m_keyEvent.key.keysym.mod && KMOD_RALT) m_VKey = 0xa5;
			else if (m_keyEvent.key.keysym.mod && KMOD_LCTRL) m_VKey = 0xa2;
			else if (m_keyEvent.key.keysym.mod && KMOD_RCTRL) m_VKey = 0xa3;
    	}
    }
  }  
}

void CKeyboard::Acquire()
{
}