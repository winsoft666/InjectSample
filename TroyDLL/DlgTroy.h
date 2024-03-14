#pragma once

#include "UIlib.h"
#include "resource.h"
using namespace DuiLib;

class DlgTroy : public WindowImplBase {
public:
	DlgTroy();
	~DlgTroy();
protected:
	CDuiString GetSkinFile() {
		return TEXT("dlg_troy.xml");
	}

	CDuiString GetSkinFolder() {
		return TEXT("");
	}

	UILIB_RESOURCETYPE GetResourceType() const {
		return UILIB_ZIPRESOURCE;
	}

	LPCTSTR GetResourceID() const {
		return MAKEINTRESOURCE(IDR_ZIPRES1);
	}

	LPCTSTR GetWindowClassName() const {
		return TEXT("DlgInject");
	}

	void OnFinalMessage(HWND hWnd);
	void Notify(TNotifyUI &msg);
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
};
