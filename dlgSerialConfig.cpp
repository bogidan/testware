#include "stdafx.h"
#include <tuple>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Return_Button.H>


#include "enumser/enumser.h"
std::tuple<std::string, u32> dialog_SerialConfig( int w = 400, int h = 300 )
{
	std::vector<UINT> ports;
	std::vector<std::string> friendlyNames;
	if( !CEnumerateSerial::UsingSetupAPI2(ports, friendlyNames) )
		throw "Failure in EnumSer Library";

	char buf[40];
	int w2 = w/2, column_widths[] = { w-10, 0 };
	Fl_Double_Window win(w, h);
	Fl_Browser port(5,    5, w2-10, h-40);
		port.column_widths(column_widths);
		port.column_char(',');
		port.type(FL_HOLD_BROWSER);
		for(u32 i = 0; i < ports.size(); i++) {
			sprintf_s(buf, "COM%u", ports[i]);
			port.add(buf, (void*)ports[i]);
		}
		port.select(1,1);
	Fl_Browser baud(5+w2, 5, w2-10, h-40);
		baud.column_widths(column_widths);
		baud.column_char(',');
		baud.type(FL_HOLD_BROWSER);
		for(u32 i : {CBR_38400, CBR_115200}) {
			sprintf_s(buf, "%u", i);
			baud.add(buf, (void*)i);
		}
		baud.select(1,1);
//	Fl_Button refresh(w-200, h-31, 90, 25, "Refresh");
//		refresh.callback([](Fl_Widget *w, void *p){
//			// TODO
//		}, &win);		
	Fl_Return_Button ok(w-100, h-31, 90, 25, "Select");
		ok.callback([](Fl_Widget *w, void *p){
			((Fl_Window*)p)->hide();
		}, &win);		

    win.add(port);
	win.add(baud);
	win.add(ok);
    win.end();

	win.set_modal();
    win.show();
	while( win.shown() ) Fl::wait();
	
	sprintf_s(buf, "\\\\.\\COM%u", (u32)port.data(port.value()));
	return std::make_tuple(std::string(buf), (u32)baud.data(baud.value()));	
}
