
#include "stdafx.h"



static char line[256];
static nil parse_line( char ch )
{
}

static u08 msg[256] = {0};
static u08 *pos = msg, checksum = 0;
static u08 &len = msg[2];
static int parse_msg( u08 ch )
{

	if( pos == msg && ch != 0xA5 ) return 0;

	*pos = ch;
	
	if( pos - msg >= (len + 3) ) {
	    const bool valid = (checksum == ch);
	    printf("msg: ");
	    for(u08 *s = msg; s <= pos; s++)
    	    printf("%02X ", *s);
    	printf(valid ? "\n" : "(checksum: 0x%02X) invalid\n", checksum);
    	
		pos = msg;
		len = 0;
		checksum = 0;
		return 1;
	}

	pos++;
	checksum += ch;
	return 1;
}

static char filtered[4096];
const char* filter_rx(const char *data, size_t len)
{
	char *pos = filtered;

	for(size_t i = 0; i < len; i++) {
		const char ch = data[i];
		if(!parse_msg((u08) ch))
			parse_line(*pos++ = ch);
	}

	return *pos = '\0', filtered;
}
