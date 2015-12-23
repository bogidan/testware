
// This file will be included multiple times to unwrap the cluster

//#define _opt(ch, str, uvar, def, ex )
#ifdef _opt
_opt('b', "baud",    baud,     "sets serial baudrate", "-b:[rate]"                     )
_opt('p', "parity",  parity,   "sets serial parity",   "-p:[none|odd|even|mark|space]" )
_opt('s', "stopbits",stopbits, "sets serial stopbits", "-s:[]"                         )
_opt('l', "log",     log,      "sets log file",        "-l:filename.log"               )
#undef _opt
#endif