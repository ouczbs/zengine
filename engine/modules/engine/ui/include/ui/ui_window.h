#pragma once
#include <NsCore/Noesis.h>
#include <NsGui/UserControl.h>
#include <NsCore/ReflectionDeclare.h>
namespace api {
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Provides the ability to create, configure, show, and manage the lifetime of windows.
	///
	/// http://msdn.microsoft.com/en-us/library/system.windows.window.aspx
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class UIWindow : public Noesis::UserControl
	{
	public:
		UIWindow();
		~UIWindow();
		virtual void InitializeComponent();
		NS_DECLARE_REFLECTION(UIWindow, Noesis::UserControl)
	};
}