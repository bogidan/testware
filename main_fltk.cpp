
#include "stdafx.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Preferences.H>

#pragma comment(lib, "fltkd.lib")
#pragma comment(lib, "fltkformsd.lib")
#pragma comment(lib, "comctl32.lib")

#include "serial.h"
#include <thread>

nil load_script( str_c fn );
extern fastdelegate::FastDelegate1<const char*>                 delSend;
extern fastdelegate::FastDelegate3<const char*,int,const char*> delAdd;


#include "enumser/enumser.h"
auto dialog_SerialConfig( int w = 400, int h = 300 ) {
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
	Fl_Browser baud(5+w2, 5, w2-10, h-40);
		baud.column_widths(column_widths);
		baud.column_char(',');
		baud.type(FL_HOLD_BROWSER);
		for(u32 i : {CBR_38400, CBR_115200}) {
			sprintf_s(buf, "%u", i);
			baud.add(buf, (void*)i);
		}
	Fl_Return_Button ok(w-100, h-35, 90, 25, "Select");
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

class ConsoleWindow : public Fl_Double_Window {
	Fl_Menu_Bar menubar;
	Fl_Text_Display console;
	Fl_Text_Buffer buffer;

	serial_t       serial;
	block_buffer_t serial_buffer;
	HANDLE         serial_stop;
	std::thread    serial_thread;
public:
	static void callback_heater_auto( Fl_Widget *w, void *p ) {
		ConsoleWindow &win = *(ConsoleWindow*)p;
		const Fl_Menu_Item &item = *((Fl_Menu_Bar*)w)->mvalue();
		win.serial.transmit("hau\r");
	}
	ConsoleWindow( int w, int h, const char *title, DWORD baud = CBR_38400, const char *port = "COM2" )
		: Fl_Double_Window (w, h, title)
		, menubar (0,  0, w,     25)
		, console (0, 25, w, h - 25)
		, buffer ()
		, serial        ( port, nullptr, baud ) // CBR_115200 CBR_38400
		, serial_stop   ( CreateEvent(nullptr, true, false, nullptr) )
		, serial_thread ( serial_main, std::ref(serial), std::ref(serial_buffer), serial_stop )
	{
		menubar.add("Load Script",    FL_F+5, [](Fl_Widget *w, void *p) {
			load_script("script.lua");
		}, this, 0 );
		menubar.add("&Clear",           NULL, [](Fl_Widget *w, void *p) {
			((ConsoleWindow*)p)->buffer.text( NULL );
		}, this, 0 );
		menubar.add("Log &Serial",       "^s", [](Fl_Widget *w, void *p) {
			auto &obj = *((ConsoleWindow*)p);
			auto &item = *const_cast<Fl_Menu_Item*>(((Fl_Menu_Bar*)w)->mvalue());
			if( item.value() ) {
				Fl_Native_File_Chooser fnfc;
				fnfc.title("Select Log Location");
				fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
				fnfc.filter("Log\t*.log\n");
				//	fnfc.directory();
				switch( fnfc.show() ) {
				default:
					if( obj.serial.log( fnfc.filename() ) ) break;
				case -1: // Error
				case  1: // Cancel
					item.clear();
					w->redraw();
					return;
				}
			} else {
				obj.serial.log();
			}
		}, this, FL_MENU_TOGGLE );

		delSend = fastdelegate::MakeDelegate( &serial, serial_t::transmit );
		delAdd  = fastdelegate::MakeDelegate( this, ConsoleWindow::add_test );

		console.buffer( buffer );
		console.textfont(FL_COURIER);
		console.textsize(12);
		console.cursor_style( Fl_Text_Display::BLOCK_CURSOR );
		console.wrap_mode( Fl_Text_Display::WRAP_AT_BOUNDS, 0 );

		this->resizable( console );
		this->show();
	};
	~ConsoleWindow() {
		Fl::remove_timeout( ConsoleWindow::idle_callback, this );

		SetEvent(serial_stop);
		serial_thread.join();

		CloseHandle(serial_stop);

		console.buffer(0);
	};
	void add_test( const char* name, int shortcut, const char* function ) {
		menubar.add("&Heater/&Auto",    "^a", [](Fl_Widget *w, void *p) {
			((ConsoleWindow*)p)->serial.transmit( "hau\r" );
		}, this, FL_MENU_TOGGLE|FL_MENU_VALUE);
		menubar.add("&Heater/&Enable",  "^e", [](Fl_Widget *w, void *p) {
			((ConsoleWindow*)p)->serial.transmit( "hen\r" );
		}, this, 0 );
		menubar.add("&Heater/&Disable", "^d", [](Fl_Widget *w, void *p) {
			((ConsoleWindow*)p)->serial.transmit( "hds\r" );
		}, this, 0 );
	}
	void scroll_to_end() {
		console.scroll( console.count_lines(0, buffer.length(), 1), 0 );
	}
	static void idle_callback ( void *data ) {
		ConsoleWindow &win = *(ConsoleWindow*)data;

		bool scroll = win.serial.poll([&](const char *str) {
		//	_fwrite_nolock( str, sizeof(char), strlen(str), stdout );
			win.buffer.append( str );
		});

		if( scroll ) win.scroll_to_end();

		Fl::repeat_timeout(0.033, ConsoleWindow::idle_callback, &win);
	}
};

int main_fltk(int argc, char *argv[] ) {
	Fl::scheme("none"); // "gtk+", "gleam", "plastic"

	auto config = dialog_SerialConfig( 400, 300 );
	printf("Selected: %s at %u\n", std::get<0>(config).c_str(), std::get<1>(config) );

	ConsoleWindow win( 640, 480, "MX Testware", std::get<1>(config), std::get<0>(config).c_str() );
//	Fl::add_idle( ConsoleWindow::idle_callback, &win );
	Fl::add_timeout( 1.0, ConsoleWindow::idle_callback, &win);
	return Fl::run();

	Fl_Window *window = new Fl_Window(340,180);
	Fl_Box *box = new Fl_Box(20,40,300,100,"Hello, World!");

	box->box(FL_UP_BOX);
	box->labelfont(FL_BOLD+FL_ITALIC);
	box->labelsize(36);
	box->labeltype(FL_SHADOW_LABEL);

	window->end();
	window->show(argc, argv);

	return Fl::run();
}

