
#include "stdafx.h"
#include <conio.h>

#include <functional>
#include "bob.h"
#include "serial.h"


bool serial_t::log( str_c fn_log ) {
	if( hLog != INVALID_HANDLE_VALUE ) {
		HANDLE hTemp = hLog;
		hLog = INVALID_HANDLE_VALUE;

		CancelIoEx( hTemp, &oLog );
		CloseHandle( hTemp );
	}

	if( fn_log ) {
		HANDLE hTemp = CreateFile( fn_log, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, 0);
		switch (DWORD err = GetLastError()) {
		case NO_ERROR:
		case ERROR_ALREADY_EXISTS:
			hLog = hTemp;
			break;
		DEFAULT("Failed to open '%s' logfile" _ln, fn_log)
		}
	}
	return (hLog != INVALID_HANDLE_VALUE);
}

bool serial_t::poll( std::function<void(str_c)> sink ) {
	bool any = (print_idx != block_idx);
	while( print_idx != block_idx ) {
		sink( buf->get( print_idx ) );
		print_idx++;
	} return any;
}

void serial_t::transmit(const char* msg) {
	while( *msg != '\0' ) {
		TransmitCommChar(hCom, *msg++);
	}
}


// Callback Handlers

void CALLBACK
serial_t::rx_cb(DWORD error, DWORD count, OVERLAPPED *overlapped) {
	((serial_t*)overlapped->Pointer)->read_done(error, count);
}
void CALLBACK
serial_t::log_cb(DWORD error, DWORD count, OVERLAPPED *overlapped) {
	((serial_t*)overlapped->Pointer)->logw_done(error, count);
}

void CALLBACK serial_t::read_done(DWORD error, DWORD count) {
	if( error != NO_ERROR ) DebugBreak();

	char *curr = buf->get( block_idx ); curr[count] = '\0';
	block_idx++;
	void *next = buf->get( block_idx );

	ReadFileEx(hCom, next, (sizeof(block_t)-1), &oCom, rx_cb);

	if( hLog != INVALID_HANDLE_VALUE )
		WriteFileEx(hLog, curr, count, &oLog, log_cb);

	// Log to Console
//	if( count > 0 )
//		_fwrite_nolock(curr, 1, count, stdout);
}

void CALLBACK serial_t::logw_done(DWORD error, DWORD count) {
	if( error != NO_ERROR ) DebugBreak();
}

serial_t::serial_t( str_c fn_com, str_c fn_log, DWORD rate )
	: hCom      ( INVALID_HANDLE_VALUE )
	, oCom      {                    0 }
	, hLog      ( INVALID_HANDLE_VALUE )
	, oLog      {                    0 }
	, block_idx (                    0 )
	, print_idx (                    0 )
{
	hCom = CreateFile( fn_com, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (hCom == INVALID_HANDLE_VALUE) switch (DWORD err = GetLastError()) {
	case ERROR_PATH_NOT_FOUND:
	case ERROR_FILE_NOT_FOUND:
		error("Comm not found"); throw err;
	case ERROR_ACCESS_DENIED:
		error("Comm busy"); throw err;
	DEFAULT("Failed to open comm port" _ln) throw err;
	}

	COMMCONFIG   cfg;
	COMMTIMEOUTS timeouts;

	if (GetCommState(hCom, &cfg.dcb) == 0) DebugBreak();

	cfg.dcb.BaudRate = rate;       //  comm.baud;
	cfg.dcb.ByteSize = 8;          //  comm.bytesize;
	cfg.dcb.Parity   = NOPARITY;   //  comm.parity;;
	cfg.dcb.StopBits = ONESTOPBIT; //  comm.stopbits;
	switch(DWORD err = SetCommState(hCom, &cfg.dcb) ? NO_ERROR : GetLastError() ) {
		case NO_ERROR: break;
		DEFAULT("Failed to set COMM port settings" _ln) throw err;
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
		DEFAULT("Failed to set COMM port timeouts" _ln) throw err;
	}

	if( fn_log ) log( fn_log );
}

serial_t::~serial_t() {
	CancelIoEx( hCom, &oCom );
	CloseHandle( hCom );

	log( NULL );
}

void serial_t::start( block_buffer_t *buffer ) {
	buf = buffer;

	// Setup
	oCom.Pointer = this;
	oLog.Pointer = this;

	// Kick off first Read
	ReadFileEx(hCom, buf->get(block_idx), (sizeof(block_t)-1), &oCom, rx_cb);
}

int serial_main( serial_t &serial, block_buffer_t &buffer, HANDLE evt_stop ) {
	serial.start(&buffer);

	HANDLE handles[] = { GetStdHandle(STD_INPUT_HANDLE), evt_stop };

//	char cmd[1024];
	while( true ) {
		DWORD result = WaitForMultipleObjectsEx( lengthOf(handles), handles, FALSE, INFINITE, TRUE );
		switch( result ) {
		case WAIT_OBJECT_0 + 0: // STDIN
								//	if(_kbhit()) putchar(_getch());
			if(_kbhit()) TransmitCommChar(serial.hCom, _getch());
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
			break;
		case WAIT_OBJECT_0 + 1: // Event Stop
			return 0;
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
	return 0;
}
