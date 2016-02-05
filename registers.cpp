
#include "stdafx.h"
#include "bob.h"

template<typename T, u8 START, u8 SIZE>
struct Bits {
	const static mask = ((1 << SIZE) - 1);
	Bits& operator=(T v) { raw = (raw & ~mask) | ((v << (START - SIZE)) & mask) };
	operator T() const   { return (raw << (START - SIZE)) & mask; };
private:
	T raw;
};

union MX_DEBUG { public:
	struct v100{
		typedef Bits<u32, 32, 1> Bits;
		enum { DISABLE = 1, ENABLE = 0 };
	};
	struct Shutdown_N {
		enum { DISABLE = 0, ENABLE = 1 };
	};
	struct v_53 : Shutdown_N {
		typedef Bits<u32, 31, 1> Bits;
	};
	struct v__5 : Shutdown_N {
		typedef Bits<u32, 30, 1> Bits;
	};
	struct RX_Chain {
		typedef Bits<u32, 30, 4> Bits;
		enum { ALL = 0xF, TOP = 0x3, BOT = 0xC, NONE = 0x0 };
	};
	//...
	union Bits {
		MX_DEBUG::v100::Bits     v100;
		MX_DEBUG::v_53::Bits     v_53;
		MX_DEBUG::v__5::Bits     v__5;
		MX_DEBUG::RX_Chain::Bits RX_Chain;
	} bits;

	void EnablePowerSupplies() {
		bits.v100 = v100::ENABLE;
		bits.v_53 = v_53::ENABLE;
		bits.v__5 = v__5::ENABLE;
	}
	//...
	u32 raw;
};

union FPGA_TX {

	u32 raw;
};

int test() {
	MX_DEBUG &reg_mx_debug = *(MX_DEBUG*) 0x40021002;

	reg_mx_debug.EnablePowerSupplies();
	reg_mx_debug.bits.v100.
}
