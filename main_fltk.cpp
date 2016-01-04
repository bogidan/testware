
#include "stdafx.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Preferences.H>

#include "luaThread.h"

#pragma comment(lib, "fltkd.lib")
#pragma comment(lib, "fltkformsd.lib")
#pragma comment(lib, "comctl32.lib")

#include "serial.h"
#include <thread>

std::tuple<std::string, u32> dialog_SerialConfig( int w = 400, int h = 300 );
Fl_Menu_Button tests_menu(0,0,80,1, "Loaded Tests");
Fl_Menu_Item running_menu[] = {
	{"Abort", 0, [](Fl_Widget *w, void *v){ ((luaThread*)w)->abort(); }, 0 },
	{0}
};

void menu_add(str_c menu, str_c key, void* v)
{
	int keyi = fl_old_shortcut(key);
	if( menu ) tests_menu.add(menu, keyi, [](Fl_Widget *w, void *v) {
		if( w && v ) ((luaThread*)w)->exec( (int)v );
	}, v, 0);
}

void transmit(str_c msg) {
	printf(msg);
}

class ConsoleWindow: public Fl_Double_Window {
	Fl_Menu_Bar menubar;
	Fl_Text_Display console;
	Fl_Text_Buffer buffer;
	luaThread lua;

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
		, lua(menu_add, MakeDelegate( &serial, &serial_t::transmit ))
	{
		menubar.add("Load Script",    FL_F+5, [](Fl_Widget *w, void *p) {
			//load_script("script.lua");
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

	int handle(int e) {
		switch(e) {
		case FL_SHORTCUT:
			if( !lua.running && tests_menu.handle(e) ) return 1;
			break;
		case FL_PUSH:
			if( Fl::event_button() == FL_RIGHT_MOUSE ) {
				tests_menu.position(Fl::event_x(), Fl::event_y());
				tests_menu.type( Fl_Menu_Button::POPUP2 );
				const auto &m = (lua.running)
					? running_menu->popup(Fl::event_x(), Fl::event_y(), 0, running_menu)
					: tests_menu.popup();
				if( m && m->callback_ ) m->do_callback((Fl_Widget*)&lua, m->user_data());
				return 1;
			}
			break;
		case FL_RELEASE:
			if( Fl::event_button() == FL_RIGHT_MOUSE ) return 1;
			break;
		}
		return Fl_Widget::handle(e);
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

