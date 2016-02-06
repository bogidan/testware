
#include "stdafx.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Input.H>
#include <FL/fl_ask.H>

#include "luaThread.h"

#pragma comment(lib, "fltkd.lib")
#pragma comment(lib, "fltkformsd.lib")
#pragma comment(lib, "comctl32.lib")

#include "serial.h"
#include <thread>

#define DEFAULT_FONT FL_COURIER_BOLD

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
/*
void timer_callback ( void *data ) {
	ConsoleWindow &win = *(ConsoleWindow*)data;

	bool scroll = win.serial.poll([&](const char *str) {
	//	_fwrite_nolock( str, sizeof(char), strlen(str), stdout );
		win.buffer.append( str );
	});

	if( scroll ) win.scroll_to_end();

	Fl::repeat_timeout(0.033, ConsoleWindow::idle_callback, &win);
}
void setTimeout(str_c name, )
{
	Fl::add_timeout( 1.0, timer_callback, );
}//*/

void transmit(str_c msg) {
	printf(msg);
}

namespace Monokai {
	const char name[] = "Monokai";
	const int console_fontsize = 14;
	const Fl_Color background = FL_BLACK; //(Fl_Color) 0x27282200;
	const Fl_Color selection  = FL_GRAY;  //(Fl_Color) 0x49483E00;
	const Fl_Color text       = FL_WHITE; //(Fl_Color) 0xF8F8F200;
	const Fl_Color menu_bg    = (Fl_Color) 0xEDEDED00;
	const Fl_Color menu_txt   = (Fl_Color) 0x40404000;
	const Fl_Color menu_sel   = (Fl_Color) 0x00BFFF00;
};

#define log_info(...) printf(__VA_ARGS__)
#include <FL/Fl_Menu_Window.H>
#include <functional>

template<class Widget>
class Popup : public Widget {
	std::function<void(void)> _bind;
public:
	Popup( int x, int y, int w, int h, const char* label = 0 )
		: Widget (x,y,w,h,label)
	{}
	int handle(int e) {
		if( e == FL_UNFOCUS || e == FL_LEAVE ) {
			hide();
		}
		return Widget::handle(e);
	}
	void hide( ) {
		if( _bind ) _bind();
		Widget::hide();
	}
	void bind( std::function<void(void)> &&func ) {
		_bind = func;
	}
	void add_unique( const char* cmd ) {
		add(cmd);
	}
};
class CommandInput: public Fl_Group {
	const int split;
	Fl_Input           command;
	Fl_Button          dropdown;
	static const int dw;
public:
	Popup<Fl_Browser>  history;
public:
	CommandInput( int x, int y, int w, int h )
		: history  (x + dw,    y + h, split, 300)
		, Fl_Group (x, y, w, h * 2, NULL)
		, split    (w - dw)
		, command  (x,         y,     split, h)
		, dropdown (x + split, y,     dw,    h, "@2>")
	{
		command.when( FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED );
		command.textfont(DEFAULT_FONT);
		command.callback([](Fl_Widget *w, void *v){
			((CommandInput*)v)->do_command();
		}, this);
		
		dropdown.color( Monokai::menu_bg, Monokai::menu_sel );
		dropdown.type(FL_TOGGLE_BUTTON);
		dropdown.callback(do_dropdown, this);
		history.bind( [&]() {
			dropdown.value(0);
			dropdown.clear_visible_focus();
		});

		history.type(FL_HOLD_BROWSER);
		history.textfont(DEFAULT_FONT);
        history.textsize(14);
		history.color( Monokai::menu_bg, Monokai::menu_sel );
		history.textcolor( Monokai::menu_txt );
		history.add("This"); history.add("is"); history.add("a");
		history.hide();
		history.callback([](Fl_Widget *w, void *v){
			((CommandInput*)v)->do_history();
		}, this);

		add(command);
		add(dropdown);
		//add(history);
		end();
	}
	nil do_transmit( const char* cmd ) {
		log_info("Command: %s\n", cmd);
	//	((serial_t*)v)->transmit(cmd);
	//	((serial_t*)v)->transmit("\r");
	}
	nil do_history() {
		str_c cmd = history.text( history.value() );
		do_transmit( cmd );
		history.hide();
	}
	nil do_command() {
		str_c cmd = command.value();
		history.add_unique(cmd);
		do_transmit( cmd );
	}
	static nil do_dropdown( Fl_Widget *w, void *v ) {
		CommandInput *inp = ((CommandInput*) v);

		if( inp->dropdown.value() ) inp->history.show();
		else                        inp->history.hide();
	}
};
const int CommandInput::dw = 22;

class ConsoleWindow: public Fl_Double_Window {
	Fl_Menu_Bar menubar;
	CommandInput command;
	Fl_Text_Display console;
	Fl_Text_Buffer buffer;
	luaThread lua;
	HWND dbgConsole;

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
		, menubar (0, 0, w, 26)
		, command (w-252, 2, 250, 22)
		, console (0, 26, w, h - 26)
		, buffer ()
		, serial        ( port, nullptr, baud ) // CBR_115200 CBR_38400
		, serial_stop   ( CreateEvent(nullptr, true, false, nullptr) )
		, serial_thread ( serial_main, std::ref(serial), std::ref(serial_buffer), serial_stop )
		, lua(menu_add, MakeDelegate( &serial, &serial_t::transmit ))
		, dbgConsole (NULL)
	{
        tests_menu.textfont(DEFAULT_FONT);
        tests_menu.textsize(14);
		tests_menu.color( Monokai::menu_bg, Monokai::menu_sel );
		tests_menu.textcolor( Monokai::menu_txt );

		AllocConsole(); 
		freopen("CONOUT$", "w", stdout);
		freopen("CONIN$",  "r", stdin);
		dbgConsole = GetConsoleWindow();
		ShowWindow(dbgConsole, SW_HIDE);
		
		menubar.add("&Edit/&Console FontSize", NULL, [](Fl_Widget *w, void *p) {
			auto fontsize = atoi(fl_input("Font Size...", "14"));
			((Fl_Text_Display*)p)->textsize(fontsize ? fontsize : 14);
		}, &console, 0);
		menubar.add("Load Script",    FL_F+5, [](Fl_Widget *w, void *p) {
			tests_menu.clear();
			((ConsoleWindow*)p)->lua.reset();
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
		}, this, FL_MENU_TOGGLE | FL_MENU_INACTIVE );
		menubar.add("DbgConsole", NULL, [](Fl_Widget *w, void *p) {
			static int console = false;
			ShowWindow((HWND)p, (console = !console) ? SW_SHOW : SW_HIDE);
		}, dbgConsole, FL_MENU_TOGGLE );
        //menubar.add("Tests", 0, 0, (void*)&tests_menu.menu()[1], FL_SUBMENU_POINTER );

		/* command.callback( [](Fl_Widget *w, void *v) {
			Fl_Input *input = ((Fl_Input*)w);
			cstr_t buf;
			strcpy_s(buf, input->value());
			strcat_s(buf, "\r");
			printf("Command: %s\r", buf);
			((serial_t*)v)->transmit(buf);
		}, &serial); // */

		console.buffer( buffer );
		console.color( Monokai::background, Monokai::selection );
		console.textcolor( Monokai::text );
		console.cursor_color( Monokai::selection );
		console.textfont(DEFAULT_FONT);
		console.textsize(Monokai::console_fontsize);
		console.cursor_style( Fl_Text_Display::SIMPLE_CURSOR );
		console.wrap_mode( Fl_Text_Display::WRAP_AT_BOUNDS, 0 );


		this->resizable( console );
		this->show();
	};
	~ConsoleWindow() {
		Fl::remove_timeout( ConsoleWindow::idle_callback, this );

		SetEvent(serial_stop);
		serial_thread.join();

		CloseHandle(serial_stop);
		FreeConsole();

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
		return Fl_Double_Window::handle(e);
	}
	void scroll_to_end() {
		auto len   = buffer.length();
		auto lines = console.count_lines(0, len, 1);
		if(lines > 10) {
			int idx = 0;
			if(buffer.findchar_forward(0,'\n', &idx )) {
				buffer.remove(0, idx);
				lines--;
			}
		}
		console.scroll( lines, 0 );
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
	Fl::scheme("gtk+"); // "none", "gtk+", "gleam", "plastic"

	auto config = dialog_SerialConfig( 400, 300 );
	printf("Selected: %s at %u\n", std::get<0>(config).c_str(), std::get<1>(config) );

	ConsoleWindow win( 640, 480, "Testware", std::get<1>(config), std::get<0>(config).c_str() );
//	Fl::add_idle( ConsoleWindow::idle_callback, &win );
	Fl::add_timeout( 1.0, ConsoleWindow::idle_callback, &win);
	return Fl::run(); // */

	Fl_Window *window = new Fl_Window(340,180);
	Fl_Box *box = new Fl_Box(20,40,300,100,"Hello, World!");

	box->box(FL_UP_BOX);
	box->labelfont(FL_BOLD+FL_ITALIC);
	box->labelsize(36);
	box->labeltype(FL_SHADOW_LABEL);

	CommandInput *cmd = new CommandInput(0,0,200,26);

	cmd->box(FL_UP_BOX);
	window->add(box);
	window->add(cmd);
	window->add(cmd->history);
	window->end();
	window->show(argc, argv);

	return Fl::run();
}

