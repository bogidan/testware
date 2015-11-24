
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <conio.h>

#include "bob.h"

#include <initializer_list>
#include <algorithm>
bool wcsstr( wcs_c opt, std::initializer_list<wcs_c> valid ) {
	return std::any_of( std::begin(valid), std::end(valid), [&](wcs_c valid){ return wcsstr(opt, valid); } );
}

struct config_comm_s {
	wcs_t file;
	wcs_c log;
	DWORD baud;
	BYTE  bytesize;
	BYTE  parity;
	BYTE  stopbits;
} comm = { L"COM2", NULL, CBR_115200, 8, NOPARITY, ONESTOPBIT };

void options_baud( wcs_c s ) {
	if( s[0] == L':' ) s++;
	if( wcslen(s) <= 1 ) return;
	else if(wcsstr(s, L"mx" )) comm.baud = CBR_38400;
	else if(wcsstr(s, L"vic")) comm.baud = CBR_115200;
	else if( int val = _wtoi(s) ) {
		int validCBR[] = { CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400, CBR_4800, CBR_9600, CBR_14400,
			CBR_19200, CBR_38400, CBR_56000, CBR_57600, CBR_115200, CBR_128000, CBR_256000 };
		for( int i : validCBR ) if( i == val ) { comm.baud = val; return; }
		goto invalid_baud;
	} else { invalid_baud:
	// Invalid Baud-Rate
	fprintf(stderr, "Invalid baud rate: %ls\n", s);
	exit(EXIT_FAILURE);
	}
}
void options_bytesize( wcs_c s ) {
	if( s[0] == L':' ) s++;
	if( wcslen(s) <= 1 ) return;
}
void options_parity( wcs_c s ) {
	if( s[0] == L':' ) s++;
	if( wcslen(s) <= 1 ) return;
	else if(wcsstr(s, L"none" )) comm.parity = NOPARITY;
	else if(wcsstr(s, L"odd"  )) comm.parity = ODDPARITY;
	else if(wcsstr(s, L"even" )) comm.parity = EVENPARITY;
	else if(wcsstr(s, L"mark" )) comm.parity = MARKPARITY;
	else if(wcsstr(s, L"space")) comm.parity = SPACEPARITY;
}
void options_stopbits( wcs_c s ) {
	if( s[0] == L':' ) s++;
	if( wcslen(s) <= 1 ) return;
	else if(wcsstr(s,{L"one", L"1"  })) comm.parity = ONESTOPBIT;
	else if(wcsstr(s,{L"half",L"1.5"})) comm.parity = ONE5STOPBITS;
	else if(wcsstr(s,{L"two", L"2"  })) comm.parity = TWOSTOPBITS;
}
void options_log( wcs_c s ) {
	comm.log = ( s[0] == L':' ) ? s+1 : s;
}