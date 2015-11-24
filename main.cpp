
#include <Windows.h>
#include <stdlib.h>
#include <iup.h>
#pragma comment(lib, "iup.lib")
#pragma comment(lib, "comctl32.lib")

#include "bob.h"
#include <stdio.h>

#include <chrono>
std::chrono::seconds check(1);


namespace bob {
	struct time {
		u32 prev;
		time() { prev = GetTickCount(); };
		template <u32 elapsed>
		bool elapsed() {
			u32 curr = GetTickCount();
			s32 diff = (curr - prev) - elapsed;
			if( diff < 0 ) return false;
			prev = curr + diff;
			return true;
		}
	};
}

Ihandle *gtext = NULL;

int proc_idle( Ihandle *ih ) { // Icallback
	// Check Buffers



	return 0;
};

int main( int argc, char *argv[] ) {
	Ihandle *dlg, *multitext, *menubar;

	IupOpen(&argc, &argv);

	gtext = multitext = IupText(NULL);
	IupSetAttribute(multitext, "MULTILINE", "YES");
	IupSetAttribute(multitext, "EXPAND", "YES");
	IupSetAttribute(multitext, "BORDER","NO");
	IupSetAttribute(multitext, "BGCOLOR","#111111");
	IupSetAttribute(multitext, "FGCOLOR","#DDDDDD");
	IupSetAttribute(multitext, "FONT","Consolas 12");
	IupSetAttribute(multitext, "READONLY","YES");
	IupSetAttribute(multitext, "APPENDNEWLINE","YES");

	IupSetFunction("IDLE_ACTION", proc_idle);

	menubar = IupMenu(
		IupSubmenu("COM", IupMenu(
			IupItem("Set Rate", NULL),
			IupSeparator(),
			IupItem("Logging", NULL),
			NULL)),
		IupSubmenu("Heater", IupMenu(
			IupItem("Auto", NULL),
			IupItem("Enable", NULL),
			NULL)),
		IupItem("Send", NULL),
		NULL);

	dlg = IupDialog(multitext);
	IupSetAttribute(dlg, "TITLE", "MX Testware");
	IupSetAttribute(dlg, "SIZE", "QUARTERxQUARTER");
	IupSetAttributeHandle(dlg, "MENU", menubar);
	IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

	IupMainLoop();

	IupClose();
	return 0;
}