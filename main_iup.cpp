
#include <Windows.h>
#include <stdlib.h>
#include <iup.h>
#pragma comment(lib, "iup.lib")
#pragma comment(lib, "comctl32.lib")

#include "bob.h"
#include "serial.h"

Ihandle *gtext = NULL;
serial_t *gserial;

int proc_idle( Ihandle *ih ) { // Icallback
	// Check Buffers

	gserial->dump([&](str_c str) {
		IupSetAttribute(gtext, "APPEND", str);
	});

	return 0;
};

Ihandle* iup_item(const char* str, Icallback fn) {
	auto ih = IupItem( str, NULL);
	IupSetCallback( ih, "ACTION", fn);
	return ih;
}



#include <thread>
int main_iup( int argc, char *argv[] ) {
	block_buffer_t buffer;
	serial_t serial("COM2");
	gserial = &serial;

	Ihandle *dlg, *multitext, *menubar;

	HANDLE evt_stop = CreateEvent(nullptr, true, false, nullptr);
	std::thread serial_thread(serial_main, std::ref(serial), std::ref(buffer), evt_stop);

	IupOpen(&argc, &argv);

	gtext = multitext = IupText(NULL);
	IupSetAttribute(multitext, "MULTILINE", "YES");
	IupSetAttribute(multitext, "EXPAND", "YES");
	IupSetAttribute(multitext, "BORDER","NO");
	IupSetAttribute(multitext, "BGCOLOR","#111111");
	IupSetAttribute(multitext, "FGCOLOR","#DDDDDD");
	IupSetAttribute(multitext, "FONT","Consolas 12");
	IupSetAttribute(multitext, "READONLY","YES");
	IupSetAttribute(multitext, "APPENDNEWLINE","NO");

	IupSetFunction("IDLE_ACTION", proc_idle);
	
	menubar = IupMenu(
		IupSubmenu("COM", IupMenu(
			IupItem("Set Rate", NULL),
			IupSeparator(),
			IupItem("Logging", NULL),
			NULL)),
		iup_item("Heater Auto", [](Ihandle *ih) -> int {
			gserial->transmit("hau\r");
			return IUP_DEFAULT;
		}),
			iup_item("Heater Enable", [](Ihandle *ih) -> int {
			bool en = 0==strcmp("ON", IupGetAttribute(ih, "VALUE"));
			gserial->transmit( en ? "hds\r" : "hen\r");
			IupSetAttribute(ih, "VALUE", en ? "OFF" : "ON" );
			return IUP_DEFAULT;
		}),
		IupItem("Send", NULL),
		NULL);

	dlg = IupDialog(multitext);
	IupSetAttribute(dlg, "TITLE", "MX Testware");
	IupSetAttribute(dlg, "SIZE", "HALFxHALF");
	IupSetAttributeHandle(dlg, "MENU", menubar);
	IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

	IupMainLoop();

	IupClose();

	SetEvent(evt_stop);
	serial_thread.join();
	
	return 0;
}
