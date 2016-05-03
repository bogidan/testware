#pragma once

#include <atomic>
#include <functional>

constexpr bool is_powerOf2(u32 v) {
	// http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
	return v && ((v & (v - 1)) == 0);
}

template<class T, size_t dim = 4>
class pool_t {
	static_assert( is_powerOf2(dim), "pool_t" " dimension must be a power of 2" );
	T blocks[dim];
public:
	T& operator[] (u32 idx) {
		return blocks[idx & (dim-1)];
	}
	T& get (u32 idx) {
		return blocks[idx & (dim-1)];
	}
};


typedef struct {
	size_t count;
	char data[4096];
} block_t;
typedef pool_t<block_t, 4u> block_buffer_t;
typedef std::function<void(str_c, size_t)> sink_ft;

struct serial_t {
	OVERLAPPED oCom;
	HANDLE     hCom;
	OVERLAPPED oLog;
	HANDLE     hLog;
	FILE      *fLog;

	std::atomic<u32> block_idx, print_idx;
	block_buffer_t *buf;

	serial_t( str_c fn_com = "COM2", str_c fn_log = NULL, DWORD rate = CBR_115200 );//CBR_38400 );
	~serial_t();

	void start( block_buffer_t *buffer );
	bool log( str_c fn_log = NULL );
	bool poll(sink_ft sink);
	void transmit(const char* msg);
	void transmit_bytes(const char* msg, size_t len);

	void CALLBACK read_done(DWORD error, DWORD count);
	void CALLBACK logw_done(DWORD error, DWORD count);

	static void CALLBACK rx_cb (DWORD error, DWORD count, OVERLAPPED *overlapped);
	static void CALLBACK log_cb(DWORD error, DWORD count, OVERLAPPED *overlapped);
};

int serial_main( serial_t &serial, block_buffer_t &buffer, HANDLE evt_stop );
