/*
 * CMessageBox.cpp
 *
 *  Created on: 04.12.2009
 *      Author: gerstrong
 */

#include "CMessageBox.h"
#include "sdl/CVideoDriver.h"
#include "sdl/input/CInput.h"
#include "graphics/CGfxEngine.h"
#include "StringUtils.h"

// This is a local functor that is invoked when the dialog has be closed
struct CloseDialog : public InvokeFunctorEvent
{
	CloseDialog(bool &mustClose) : mMustClose(mustClose) {}

	void operator()()
	{		mMustClose = true;	}

	bool &mMustClose;
};

CMessageBox::CMessageBox(const std::string& Text, bool lower, bool keymsg, bool leftbound) :
CGUIDialog(CRect<float>(0.1f, 0.1f, 0.8f, 0.8f)),
m_mustclose(false)
{
	const char closeChar = 0x1F;
	std::string closeString;
	closeString = closeChar;
	CGUIButton*	pButton	= new CGUIButton( closeString, new CloseDialog(m_mustclose), CGUIButton::NONE );

	addControl( pButton, CRect<float>(0.0f, 0.0f, 0.06f/0.8f, 0.06f/0.8f) );

	mpReturnButton = pButton;

	mpTextCtrl = new CGUIText( Text );

	// Those formulas work well with our constellation but I don't think they are perfect.
	// They transform the Message Box the way the text fits perfectly in.
	const float screenW = g_pVideoDriver->getGameResolution().w;
	const float screenH = g_pVideoDriver->getGameResolution().h;
	mRect.w = static_cast<float>( (mpTextCtrl->mTextDim.w+4)*8 )/screenW;
	mRect.h = static_cast<float>( (mpTextCtrl->mTextDim.h+2)*8 )/screenH;
	mRect.x = (1.0f - mRect.w)/2.0f;
	mRect.y = (1.0f - mRect.h)/2.0f;

	// now let's center that long text...
	CRect<float> TextRect;
	TextRect.w = ((mpTextCtrl->mTextDim.w*8)/screenW) / mRect.w;
	TextRect.h = ((mpTextCtrl->mTextDim.h*8)/screenH) / mRect.h;
	TextRect.x = (2.5f*TextRect.w)/(static_cast<float>(mpTextCtrl->mTextDim.w));
	TextRect.y = (1.0f*TextRect.h)/(static_cast<float>(mpTextCtrl->mTextDim.h));

	if(lower) // if lower is enabled, try to snap the Dlg to the bottom off the screen.
	{
		mRect.y = 1.0f - mRect.h;
	}

	addControl( mpTextCtrl, TextRect );

	CRect<float> closeRect = pButton->mRect;
	closeRect.x = mRect.x;
	closeRect.y = mRect.y;
	pButton->setRect(closeRect);
	initBackground();
}

void CMessageBox::processLogic()
{
	if(g_pInput->getPressedCommand(IC_STATUS))
		m_mustclose = true;

	CGUIDialog::processLogic();
}

bool CMessageBox::isFinished()
{ return m_mustclose; }
