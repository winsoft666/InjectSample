#include "DlgTroy.h"

DlgTroy::DlgTroy() {

}

DlgTroy::~DlgTroy() {

}

void DlgTroy::OnFinalMessage(HWND hWnd) {
	WindowImplBase::OnFinalMessage(hWnd);
}

void DlgTroy::Notify(TNotifyUI &msg) {
	WindowImplBase::Notify(msg);
}

LRESULT DlgTroy::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
}

LRESULT DlgTroy::ResponseDefaultKeyEvent(WPARAM wParam) {
	if (wParam == VK_RETURN) {
		return 1;
	}
	else if (wParam == VK_ESCAPE) {
		return 1;
	}

	return WindowImplBase::ResponseDefaultKeyEvent(wParam);
}
