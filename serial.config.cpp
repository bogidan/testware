
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
	tcs_t file;
	tcs_c log;
	DWORD baud;
	BYTE  bytesize;
	BYTE  parity;
	BYTE  stopbits;
} comm = { _T("COM2"), NULL, CBR_115200, 8, NOPARITY, ONESTOPBIT };

void options_baud( const char *s ) {
	if( s[0] == L':' ) s++;
	if( _tcslen(s) <= 1 ) return;
	else if(_tcsstr(s, "mx" )) comm.baud = CBR_38400;
	else if(_tcsstr(s, "vic")) comm.baud = CBR_115200;
	else if( int val = _ttoi(s) ) {
		int validCBR[] = { CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400, CBR_4800, CBR_9600, CBR_14400,
			CBR_19200, CBR_38400, CBR_56000, CBR_57600, CBR_115200, CBR_128000, CBR_256000 };
		for( int i : validCBR ) if( i == val ) { comm.baud = val; return; }
		goto invalid_baud;
	} else { invalid_baud:
	// Invalid Baud-Rate
	fprintf(stderr, "Invalid baud rate: %s\n", s);
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

int main_args( int argc, char *argv[] ) {

	str_t s;
	for( int opt = 1; opt < argc; opt++ ) {
		if( argv[opt][0] != '-' ) {

		} else switch( s = argv[opt] + 2, (argv[opt][0] == '-') ? argv[opt][1] : 0 ) {
			#define _opt(ch, str, uvar, def, ex )\
				case L ## ch: options_##uvar (s); break;
			#include "options.h"
		case 'h': default:
			fprintf(stderr, "Usage: %ls [-flags] [com-port filename]\n", argv[0]);
			#define _opt(ch, str, uvar, def, ex ) \
				fprintf(stderr, "  -%c | --"str" - "def"\n    ex:'"ex"'\n",ch);
			#include "options.h"
			exit(EXIT_FAILURE);
			break;
		case '-': // Long Options
			if(wcslen(s) <= 1) break;
			#define _opt(ch, str, uvar, def, ex ) \
				else if(0== wcscmp(s, L ## str)) options_##uvar (s);
			#include "options.h"
			break;
		case  0 : // Com Port
			comm.file = argv[opt];
		}
	}
}