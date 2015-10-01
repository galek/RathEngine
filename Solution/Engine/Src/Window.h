#pragma once
#include "EngineState.h"

namespace Rath
{
	class Window
	{
		// Public types
	public:
		typedef std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> BoundMsgFunction;

		// Constructor and destructor
	public:

		Window(EngineState* state,
			HINSTANCE hinstance,
			LPCWSTR name,
			DWORD style,
			DWORD exStyle,
			LPCWSTR iconResource,
			LPCWSTR smallIconResource,
			LPCWSTR menuResource,
			LPCWSTR accelResource);
		~Window();
		// Public methods
	public:

		HWND GetHwnd() const;
		HMENU GetMenu() const;
		HINSTANCE GetHinstance() const;
		void MessageLoop();

		BOOL IsAlive() const;
		BOOL IsMinimized() const;
		LONG_PTR GetWindowStyle() const;
		LONG_PTR GetExtendedStyle() const;
		void SetWindowStyle(DWORD newStyle);
		void SetExtendedStyle(DWORD newExStyle);
		void Maximize();
		void SetWindowPos(INT posX, INT posY);
		void GetWindowPos(INT& posX, INT& posY) const;
		void ShowWindow(bool show = true);
		void SetClientArea(INT clientX, INT clientY);
		void GetClientArea(INT& clientX, INT& clientY) const;
		void SetWindowTitle(LPCWSTR title);
		void SetScrollRanges(INT scrollRangeX,
			INT scrollRangeY,
			INT posX,
			INT posY);
		void Destroy();

		INT	CreateMessageBox(LPCWSTR message, LPCWSTR title = NULL, UINT type = MB_OK);

		template<class ClassT>
		void RegisterMessageCallback(LRESULT(ClassT::*MsgFunction)(HWND, UINT, WPARAM, LPARAM), ClassT* c)
		{
			BoundMsgFunction bound_member_fn = std::bind(MsgFunction, c, 
											   std::placeholders::_1, 
											   std::placeholders::_2, 
											   std::placeholders::_3, 
											   std::placeholders::_4);

			messageCallbacks.emplace_back(bound_member_fn);
		};

		operator HWND() { return hwnd; }		//conversion operator

		// Private methods
	private:
		void MakeWindow(LPCWSTR iconResource, LPCWSTR smallIconResource, LPCWSTR menuResource);

		LRESULT	MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		// Private members
	private:

		// Window properties
		HWND hwnd;			        // The window handle
		HINSTANCE hinstance;		// The HINSTANCE of the application
		std::wstring appName;		// The name of the application
		DWORD style;			    // The current window style
		DWORD exStyle;		        // The extended window style
		HACCEL accelTable;		    // Accelerator table handle

		std::vector<BoundMsgFunction> messageCallbacks;
	};
};
