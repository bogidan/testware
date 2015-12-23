
#include <Windows.h>
#include <nana/gui.hpp>
#include <nana/gui/widgets/menubar.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/msgbox.hpp>
#include <nana/gui/filebox.hpp>
#pragma comment(lib, "nana_v140_Debug_x86.lib")

#include <stdexcept>
#include <vector>
using std::runtime_error;
using std::vector;

std::wstring utf8toUtf16(const std::string & str)
{
	if (str.empty())
		return std::wstring();

	size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0, 
		str.data(), (int)str.size(), NULL, 0);
	if (charsNeeded == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	vector<wchar_t> buffer(charsNeeded);
	int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0, 
		str.data(), (int)str.size(), &buffer[0], buffer.size());
	if (charsConverted == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	return std::wstring(&buffer[0], charsConverted);
}

#include "bob.h"
#include "serial.h"

using namespace nana;

#include <codecvt>
#include <thread>

class notepad_form : public form
{
	place   place_;
	menubar menubar_;
	textbox textbox_;
	timer   timer_;

	block_buffer_t buffer;
	serial_t       serial;
	HANDLE         serial_stop;
	std::thread    serial_thread;
	bool           tx_ready;
	char           tx_buf[256];
public:
	~notepad_form() {
		SetEvent(serial_stop);
		serial_thread.join();

		CloseHandle(serial_stop);
	}
	notepad_form()
		: form          ( API::make_center(640,480) )
		, serial        ( "COM2", nullptr, CBR_38400 )// CBR_115200 CBR_38400
		, serial_stop   ( CreateEvent(nullptr, true, false, nullptr) )
		, serial_thread ( serial_main, std::ref(serial), std::ref(buffer), serial_stop )
	{
		caption("MX Testware");

		menubar_.create(*this);
		menubar_.push_back(L"&Heater");
		menubar_.at(0).append(L"Auto",[this](menu::item_proxy& ip) {
			serial.transmit( "hau\r" );
		//	printf("\nSEND: %s\n\n", ip.checked() ? "hen\r" : "hds\r" );
		}).check_style( menu::checks::highlight ).checked(true);
		menubar_.at(0).append(L"Enable",[this](menu::item_proxy& ip) {
			serial.transmit( ip.checked() ? "hen\r" : "hds\r" );
		//	printf("\nSEND: %s\n\n", ip.checked() ? "hen\r" : "hds\r" );
		}).check_style( menu::checks::highlight );

		textbox_.create(*this);
		textbox_.line_wrapped(true);
		textbox_.editable(false);
		textbox_.typeface( paint::font( L"consolas", 10) );
		textbox_.bgcolor ( color(0x2, 0x2, 0x2) );
		textbox_.fgcolor ( colors::white_smoke );
	//	textbox_.borderless(true);
	//	API::effects_edge_nimbus(textbox_, effects::edge_nimbus::none);
	//	textbox_.enable_dropfiles(true);
	//	textbox_.events().mouse_dropfiles([this](const arg_dropfiles& arg)
	//	{
	//		if (arg.files.size() && _m_ask_save())
	//			textbox_.load(arg.files.at(0).data());
	//	});

		timer_.interval(1000 / 30);
		timer_.elapse([&](){
			serial.dump([&](const char *str){
				_fwrite_nolock( str, sizeof(char), strlen(str), stdout );
			//	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			//	textbox_.append( converter.from_bytes(str), false);
				textbox_.append( utf8toUtf16(str), true );
			});
		});
		timer_.start();

		place_.bind(*this);
		place_.div("vert<menubar weight=28><textbox>");
		place_.field("menubar") << menubar_;
		place_.field("textbox") << textbox_;
		place_.collocate();

	//	events().unload([this](const arg_unload& arg){
	//		if (!_m_ask_save())
	//			arg.cancel = true;
	//	});
	}
private:
	nana::string _m_pick_file(bool is_open) const
	{
		filebox fbox(*this, is_open);
		fbox.add_filter(L"Text", L"*.txt");
		fbox.add_filter(L"All Files", L"*.*");

		return (fbox.show() ? fbox.file() : nana::string());
	}

	bool _m_ask_save()
	{
		if (textbox_.edited())
		{
			auto fs = textbox_.filename();
			msgbox box(*this, L"Simple Notepad", msgbox::button_t::yes_no_cancel);
			box << L"Do you want to save these changes?";

			switch (box.show())
			{
			case msgbox::pick_yes:
				if (fs.empty())
				{
					fs = _m_pick_file(false);
					if (fs.empty())
						break;
					if (fs.find(L".txt") == fs.npos)
						fs += L".txt";
				}
				textbox_.store(fs.data());
				break;
			case msgbox::pick_no:
				break;
			case msgbox::pick_cancel:
				return false;
			}
		}
		return true;
	}

	void _m_make_menus()
	{
		auto &mb = menubar_;

		mb.push_back(L"&File");
		mb.at(0).append(L"New", [this](menu::item_proxy& ip)
		{
			if(_m_ask_save())
				textbox_.reset();
		});
		mb.at(0).append(L"Open", [this](menu::item_proxy& ip)
		{
			if (_m_ask_save())
			{
				auto fs = _m_pick_file(true);
				if (fs.size())
					textbox_.load(fs.data());
			}
		});
		mb.at(0).append(L"Save", [this](menu::item_proxy&)
		{
			auto fs = textbox_.filename();
			if (fs.empty())
			{
				fs = _m_pick_file(false);
				if (fs.empty())
					return;
			}
			textbox_.store(fs.data());
		});

		mb.push_back(L"F&ORMAT");
		mb.at(1).append(L"Line Wrap", [this](menu::item_proxy& ip)
		{
			textbox_.line_wrapped(ip.checked());
		});
		mb.at(1).check_style(0, menu::checks::highlight);

		mb.push_back(L"&Heater");
		mb.at(2).append(L"Auto",[this](menu::item_proxy& ip) {

		});
		mb.at(2).append(L"Enable",[this](menu::item_proxy& ip) {

		}).check_style( menu::checks::highlight );
	}
};

int main_nana( int argc, char *argv[] )
{
	notepad_form npform;
	npform.show();
	exec();
	return 0;
}