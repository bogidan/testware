
#include "stdafx.h"
#include <stdarg.h>

#include "bob.h"

int main_iup ( int argc, char *argv[] );
int main_nana( int argc, char *argv[] );
int main_fltk( int argc, char *argv[] );


int __cdecl _eprintf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	auto r = vprintf(fmt, args);

	va_end(args);
	return r;
}
print_ft eprintf = _eprintf;


#include <utility>
int main( int argc, char *argv[] ) {
//	return main_iup( argc, argv );
//	return main_nana( argc, argv );
	return main_fltk( argc, argv );
}