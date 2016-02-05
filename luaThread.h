#pragma once

#include <thread>
#include "lua/include/lua.c++.hpp"
//#include "lua\include\selene.h"
// http://www.jeremyong.com/blog/2014/01/10/interfacing-lua-with-templates-in-c-plus-plus-11/

using namespace fastdelegate;
extern const lua_State *lua_abort;

class luaThread {
	HANDLE quitEvent, execEvent;
	std::thread thread;
	std::string script;
	int m_func;
	lua_State *m_L;
	static void stProc( luaThread *ptr ) { ptr->proc(); };
	void proc();
public:
	int running;
	FastDelegate3<str_c,str_c,void*> delMenuAdd;
	FastDelegate1<str_c>             delTransmit;

	luaThread(FastDelegate3<str_c,str_c,void*> menu_add, FastDelegate1<str_c> transmit)
		: quitEvent   (CreateEvent(0,FALSE,FALSE,0))
		, execEvent   (CreateEvent(0,FALSE,FALSE,0))
		, delMenuAdd  (menu_add)
		, delTransmit (transmit)
		, thread      (stProc, this)
		, m_func      (0)
		, m_L         (nullptr)
		, running     (0)
	{};
	~luaThread() {
		SetEvent(quitEvent);
		thread.join();
	}

	nil exec(int func) {
		m_func = func;
		SetEvent(execEvent);
	};
	nil abort() {
		lua_abort = m_L;
	}
	nil reset() {
		if( running ) abort();
		::SetEvent(quitEvent);
		thread.join();
		::ResetEvent(quitEvent);
		thread = std::thread(stProc, this);
	}

private:
	static int lua_menu_register( lua_State *L );
	static int lua_transmit( lua_State *L );
	static int lua_transmit_bytes( lua_State *L );
	static int lua_sleep( lua_State *L );
	static int lua_timeout( lua_State *L );
};
