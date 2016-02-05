#include "stdafx.h"
#include "luaThread.h"

#pragma comment(lib, "lua53.lib")



const lua_State *lua_abort = nullptr;

nil lua_hook(lua_State *L, lua_Debug *ar)
{
	if( lua_abort != L ) return;

	lua_sethook(L, lua_hook, LUA_MASKLINE, 0); 
	luaL_error(L, "abort");
}


void luaThread::proc()
{
	lua_State *L = m_L = luaL_newstate();
	luaL_openlibs(L);

	// Initialize Functions
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, lua_transmit, 1);
	lua_setglobal(L, "tx");

	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, lua_menu_register, 1);
	lua_setglobal(L, "register");

	lua_pushcfunction(L, lua_sleep);
	lua_setglobal(L, "sleep");

	// Run Hook every 100 instructions
	lua_sethook(L, lua_hook, LUA_MASKCOUNT, 100); 

	if( luaL_dofile(L, "./script.lua") ) {
		printf("%s\n", lua_tostring(L, -1));
	}

	HANDLE handles[] = { quitEvent, execEvent };
	while(true) switch(WaitForMultipleObjects(lengthOf(handles), handles, FALSE, INFINITE)) {
	case WAIT_OBJECT_0 + 0: // Quit
		lua_close(L);
		return;
	case WAIT_OBJECT_0 + 1: // Exec
		::ResetEvent(execEvent);
		lua_rawgeti(L, LUA_REGISTRYINDEX, m_func);
		
		running = true;

		// Call the function
		if (lua_pcall(L, 0, 0, 0) != 0) {
			const char* err = lua_tostring(L,-1);
			const char* msg = strchr(strchr(err, ':') + 1, ':');

			if( strcmp(msg,": abort") == 0 ) printf("Aborted\n");
			else printf("Error running function f:%s`", lua_tostring(L, -1));
		}
		
		if( lua_abort == L ) lua_abort = NULL;
		running = false;
		break;
	default:
	case WAIT_FAILED:
		__debugbreak();
		printf("Wait Failed");
	}
}


int luaThread::lua_menu_register( lua_State *L )
{
	luaThread *lua = (luaThread*) lua_touserdata(L, lua_upvalueindex(1));
	const char *key, *menu;
	key  = lua_tostring(L, 1);
	menu = lua_tostring(L, 2);
	int func = luaL_ref(L, LUA_REGISTRYINDEX);
	
	lua->delMenuAdd(menu, key, (void*) func);
	
	return 0;
}
int luaThread::lua_transmit( lua_State *L )
{
	luaThread *lua = (luaThread*) lua_touserdata(L, lua_upvalueindex(1));
	const char *msg = luaL_checkstring(L, 1);
	lua->delTransmit(msg);
	return 0;
}
int luaThread::lua_transmit_bytes( lua_State *L )
{
	luaThread *lua = (luaThread*) lua_touserdata(L, lua_upvalueindex(1));
	return 0;
}
int luaThread::lua_sleep( lua_State *L )
{
	auto ms = luaL_checkinteger(L, 1);
	if( lua_abort == L ) luaL_error(L, "abort");
	else                 Sleep( (DWORD)ms );
	return 0;
}
int luaThread::lua_timeout( lua_State *L )
{
	luaThread *lua = (luaThread*) lua_touserdata(L, lua_upvalueindex(1));
	const char *name;
	name = lua_tostring(L, 1);
	int func = luaL_ref(L, LUA_REGISTRYINDEX);
	
//	lua->delMenuAdd(name, (void*) func);
	
	return 0;
}

struct message {
	u8 __padding, start, type, length;
	u8 raw[256];
};
