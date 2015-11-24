
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <conio.h>

#include <functional>
#include "bob.h"


typedef char block_t[4096];
struct block_buffer_t {
	block_t blocks[4];
	char* curr(u32  inp) { return blocks[   inp  & 3]; }
	char* next(u32 &inp) { return blocks[(++inp) & 3]; }
};

struct serial_t {
	OVERLAPPED     oCom;
	HANDLE         hCom;
	OVERLAPPED     oLog;
	HANDLE         hLog;
	u32            block_idx, print_idx;
	block_buffer_t buf;

	serial_t( str_c conf );

	void CALLBACK read_done(DWORD error, DWORD count);
	void CALLBACK logw_done(DWORD error, DWORD count);
	void print(std::function<void(str_c)> sink);

	#define $cb static void CALLBACK
	$cb rx_cb(DWORD error, DWORD count, OVERLAPPED *overlapped) {
		((serial_t*)overlapped->Pointer)->read_done(error, count);
	}
	$cb log_cb(DWORD error, DWORD count, OVERLAPPED *overlapped) {
		((serial_t*)overlapped->Pointer)->logw_done(error, count);
	}
};

void CALLBACK serial_t::read_done(DWORD error, DWORD count) {
	if( error != NO_ERROR ) DebugBreak();

	char *curr = buf.curr( block_idx ); curr[count] = '\0';
	char *next = buf.next( block_idx );

	ReadFileEx(hCom, next, sizeof(block_t), &oCom, rx_cb);

	if( hLog != INVALID_HANDLE_VALUE )
		WriteFileEx(hLog, curr, count, &oLog, log_cb);

	if( count > 0 )
		_fwrite_nolock(curr, 1, count, stdout);
}

void CALLBACK serial_t::logw_done(DWORD error, DWORD count) {
}

void serial_t::print( std::function<void(str_c)> sink ) {
	while( print_idx != block_idx ) {
		sink( buf.curr( print_idx++ ) );
	}
}


serial_t::serial_t( str_c conf )
	: hCom      ( INVALID_HANDLE_VALUE )
	, oCom      {                    0 }
	, hLog      ( INVALID_HANDLE_VALUE )
	, oLog      {                    0 }
	, block_idx (                    0 )
	, print_idx (                    0 )
{
	oCom.Pointer = this;
	oLog.Pointer = this;


	hCom = CreateFile( comm.file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (hCom == INVALID_HANDLE_VALUE) switch (DWORD err = GetLastError()) {
	case ERROR_PATH_NOT_FOUND:
	case ERROR_FILE_NOT_FOUND:
		puts("File Not Found");
	default: DebugBreak();
	}

	COMMCONFIG   cfg;
	COMMTIMEOUTS timeouts;

	if (GetCommState(hCom, &cfg.dcb) == 0) DebugBreak();

	cfg.dcb.BaudRate = comm.baud;
	cfg.dcb.ByteSize = comm.bytesize;
	cfg.dcb.Parity   = comm.parity;;
	cfg.dcb.StopBits = comm.stopbits;
	switch(DWORD err = SetCommState(hCom, &cfg.dcb) ? NO_ERROR : GetLastError() ) {
	case NO_ERROR: break;
	default: DebugBreak();
	}

	// Specify time-out between charactor for receiving.
	timeouts.ReadIntervalTimeout = 3;
	// Specify value that is multiplied 
	// by the requested number of bytes to be read. 
	timeouts.ReadTotalTimeoutMultiplier = 3;
	// Specify value is added to the product of the 
	// ReadTotalTimeoutMultiplier member
	timeouts.ReadTotalTimeoutConstant = 2;
	// Specify value that is multiplied 
	// by the requested number of bytes to be sent. 
	timeouts.WriteTotalTimeoutMultiplier = 3;
	// Specify value is added to the product of the 
	// WriteTotalTimeoutMultiplier member
	timeouts.WriteTotalTimeoutConstant = 2;
	switch (DWORD err = SetCommTimeouts(hCom, &timeouts) ? NO_ERROR : GetLastError()) {
	case NO_ERROR: break;
	default: DebugBreak();
	}

}




#ifdef SYNC_COM_READ
#define FILE_FLAGS ( 0 )
#else
#define FILE_FLAGS ( FILE_FLAG_OVERLAPPED )
#endif
int ____wmain( int argc, wchar_t* argv[] ) {
	wcs_t s;
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

	if( comm.log ) _wfopen_s( &logfile, comm.log, L"w" );

	// http://www.codeproject.com/Articles/3061/Creating-a-Serial-communication-on-Win
	hCom = CreateFile( comm.file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAGS, 0);
	if (hCom == INVALID_HANDLE_VALUE) switch (DWORD err = GetLastError()) {
	case ERROR_PATH_NOT_FOUND:
	case ERROR_FILE_NOT_FOUND:
		puts("File Not Found");
	default: DebugBreak();
	}

	COMMCONFIG   cfg;
	COMMTIMEOUTS timeouts;

	if (GetCommState(hCom, &cfg.dcb) == 0) DebugBreak();

	cfg.dcb.BaudRate = comm.baud;
	cfg.dcb.ByteSize = comm.bytesize;
	cfg.dcb.Parity   = comm.parity;;
	cfg.dcb.StopBits = comm.stopbits;
	switch(DWORD err = SetCommState(hCom, &cfg.dcb) ? NO_ERROR : GetLastError() ) {
	case NO_ERROR: break;
	default: DebugBreak();
	}

	// Specify time-out between charactor for receiving.
	timeouts.ReadIntervalTimeout = 3;
	// Specify value that is multiplied 
	// by the requested number of bytes to be read. 
	timeouts.ReadTotalTimeoutMultiplier = 3;
	// Specify value is added to the product of the 
	// ReadTotalTimeoutMultiplier member
	timeouts.ReadTotalTimeoutConstant = 2;
	// Specify value that is multiplied 
	// by the requested number of bytes to be sent. 
	timeouts.WriteTotalTimeoutMultiplier = 3;
	// Specify value is added to the product of the 
	// WriteTotalTimeoutMultiplier member
	timeouts.WriteTotalTimeoutConstant = 2;
	switch (DWORD err = SetCommTimeouts(hCom, &timeouts) ? NO_ERROR : GetLastError()) {
	case NO_ERROR: break;
	default: DebugBreak();
	}


#ifdef SYNC_COM_READ
	// Without FILE_FLAG_OVERLAPPED in CreateFile
	ULONG bytes = 0;
	char buf[4096];
	while (ReadFile(hCom, buf, 4096, &bytes, NULL) != 0) {
		if (bytes == 0) {
			Sleep(5);
			continue;
		}
		_fwrite_nolock(buf, 1, bytes, stdout);
	}
#else
	memset(&overlappedI, 0, sizeof(overlappedI));
	ReadFileEx(hCom, readbuf, lengthOf(readbuf), &overlappedI, readCompletion);

	HANDLE handles[] = {
		GetStdHandle(STD_INPUT_HANDLE)
	};
	char cmd[1024];
	while( true ) {
		DWORD result = WaitForMultipleObjectsEx( lengthOf(handles), handles, FALSE, INFINITE, TRUE );
		switch( result ) {
		case WAIT_OBJECT_0 + 0: // STDIN
								//	if(_kbhit()) putchar(_getch());
			if(_kbhit()) TransmitCommChar(hCom, _getch());
			else {
				INPUT_RECORD r[512];
				DWORD read;
				ReadConsoleInput( *handles, r, 512, &read);
			}

			//	INPUT_RECORD record; DWORD numRead;
			//	if( ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &record, 1, &numRead) ) {
			//		if(record.EventType != KEY_EVENT) continue;
			//		if(record.Event.KeyEvent.bKeyDown) {
			//			record.Event.KeyEvent.uChar.AsciiChar;
			//		}
			//	} else DebugBreak();
			//		}
			//
			//			while (fgets(cmd, lengthOf(cmd), stdin)) {
			//				if (strstr(cmd, "quit")) {
			//					exit(1);
			//				}
			//			}
			//			break;
		case WAIT_FAILED:
			switch (DWORD err = GetLastError()) {
			case ERROR_SUCCESS: break;
			case ERROR_INVALID_HANDLE:
				DebugBreak();
				exit(1);
			} break;
		case WAIT_IO_COMPLETION: break;
		case WAIT_TIMEOUT:       DebugBreak(); break;
		default:                 DebugBreak(); break;
		}
	}
#endif
Clean:
	if( logfile ) fclose(logfile);
}