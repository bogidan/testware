#include "stdafx.h"

#include "lua\include\lua.c++.hpp"

#pragma comment(lib, "lua53.lib")

/*
int load_config() {
char buf[256];
int err;

int sip = 0;

lua_State *L = luaL_newstate();
luaL_openlibs(L);

err = luaL_loadstring(L, "sip = 304 // 24 + 1") || lua_pcall(L,0,0,0);

if( err ) {
fprintf(stderr, "%s", lua_tostring(L,-1));
lua_pop(L,1);
} else {
// Push sip variable onto the stack
lua_getglobal(L, "sip");
if( lua_isnumber(L, -1) ) {
sip = (int) lua_tonumber(L,-1);
} else {
fprintf(stderr, "sip not a number");
}
}

lua_close(L);
return 0;
}

int lua_config( const char *script, const char* variable ) {
char * value;
lua_State *L = luaL_newstate();
luaL_openlibs(L);

if( luaL_loadstring(L, script) || lua_pcall(L,0,0,0) ) {
fprintf(stderr, "%s", lua_tostring(L,-1));
lua_pop(L,1);
} else {
// Push sip variable onto the stack
lua_getglobal(L, variable);
if( lua_isstring(L, -1) ) {
variable = lua_tostring(L,-1);
} else {
fprintf(stderr, "sip not a number");
}
}

lua_close(L);
return 0;
} // */

int lua_config_account( const wchar_t *wscript, cstr_t &account, cstr_t &codecs ) {
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	// Convert to multi-byte string
	u32 len = wcslen(wscript) + 1, converted = 0;
	char *script = new char[len];
	wcstombs_s( &converted, script, len, wscript, len*2 );

	// Load and run script
	if( luaL_loadstring(L, script) || lua_pcall(L,0,0,0) ) {
		error("%s", lua_tostring(L,-1));
		lua_pop(L,1);
		goto Clean;
	}
	// Push sip_account variable onto the stack
	lua_getglobal(L, "sip_account");
	if( lua_isstring(L, -1) ) {
		strcpy_s( account, lua_tostring(L,-1) );
	} else {
		error("lua variable sip_account is non-string\n");
	}
	// Push sip_codec onto stack
	lua_getglobal(L, "sip_codecs");
	if( lua_isstring(L, -1) ) {
		strcpy_s( codecs, lua_tostring(L,-1) );
	} else {
		error("lua variable sip_codecs is non-string\n");
	}

Clean:
	lua_close(L);
	delete script;
	return 0;
}


fastdelegate::FastDelegate1<const char*>                 delSend;
fastdelegate::FastDelegate3<const char*,int,const char*> delAdd;

static int register_test( lua_State *L ) {
	// Check if argument is a string
	const char* name = luaL_checkstring(L, 1);
	const char* key  = luaL_checkstring(L, 2);
	const char* func = luaL_checkstring(L, 3);

	if( name && key && func ) {
		
	}

	return 0; // Return number of Results
}

static int transmit( lua_State *L ) {
	// Check if argument is a string
	const char* msg = luaL_checkstring(L, 1);

	// Send Message
	if( msg ) {
		puts(msg);
	}

	return 0; // Return number of Results
}


lua_State *L = nullptr;

nil load_script( str_c fn )
{
	// Close Current instance.
	if( L != NULL ) lua_close(L);
	// Create instance and load libraries
	L = luaL_newstate();
	luaL_openlibs(L);
	
	// Initialize Tx Functions
	lua_pushcfunction(L, transmit);
	lua_setglobal(L, "tx");

	if( luaL_dofile(L, fn) ) {
		printf("%s\n", lua_tostring(L, -1));
	}
	// Offload Functions
}

nil call_function( lua_State *L, str_c func )
{
	// Push function to stack
	lua_getglobal(L, func);
	
	// Check if the vairable is avalid function
	if( lua_isfunction(L, -1) ) {
		// Push args
	//	lua_pushnumber(L, 123);

		// Call the function
		if (lua_pcall(L, 2, 1, 0) != 0) {
			printf("Error running function f:%s`", lua_tostring(L, -1));
		}
	}
}
