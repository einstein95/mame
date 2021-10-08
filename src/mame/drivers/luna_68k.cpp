// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Omron Luna M68K systems.
 *
 * Sources:
 *   - https://wiki.netbsd.org/ports/luna68k/
 *
 * TODO:
 *   - skeleton only
 */
/*
 * WIP
 *
 * This driver is currently based on a VME-based Luna with a 25MHz 68030, which
 * differs from the systems supported by NetBSD. Boards installed are:
 *
 *  C25   CPU, FPU, serial, RTC, 68030 + 68882 @ 25MHz?
 *  IOC2  I/O controller (floppy, SCSI, serial), 68000 @ 10MHz?
 *  GPU8  graphics processor + serial, 68020 @ 20MHz? + +68881 @ 16MHz?
 *  DPU8  video/framebuffer, Bt458 @ 108MHz
 *  CMC   communications (GPIB, Ethernet, serial), 68020 @ 12.5MHz?
 *
 */
#include "emu.h"

#include "cpu/m68000/m68000.h"

// memory
#include "machine/ram.h"

// various hardware
#include "machine/mc146818.h"
#include "machine/z80sio.h"
#include "machine/am9513.h"

// busses and connectors
#include "bus/rs232/rs232.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class luna_68k_state : public driver_device
{
public:
	luna_68k_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_rtc(*this, "rtc")
		, m_sio(*this, "sio")
		, m_stc(*this, "stc")
		, m_serial(*this, "serial%u", 0U)
	{
	}

	void luna(machine_config &config);
	void init();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void cpu_map(address_map &map);
	void cpu_autovector_map(address_map &map);

	// machine config
	void common(machine_config &config);

private:
	// HACK: this timer drives the am9513 gate1 line
	void timer(void *ptr, s32 param)
	{
		m_stc->gate1_w(1);
		m_stc->gate1_w(0);
	}

	// devices
	required_device<m68030_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<ds1287_device> m_rtc;
	required_device<upd7201_device> m_sio;
	required_device<am9513_device> m_stc;
	required_device_array<rs232_port_device, 2> m_serial;

	emu_timer *m_timer;
};

void luna_68k_state::init()
{
	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(luna_68k_state::timer), this));
}

void luna_68k_state::machine_start()
{
	// HACK: arbitrary/unknown cycle time
	m_timer->adjust(attotime::from_hz(1'000'000), 0, attotime::from_hz(1'000'000));
}

void luna_68k_state::machine_reset()
{
}

void luna_68k_state::cpu_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom().region("eprom", 0);

	map(0x40000000, 0x4001ffff).rom().region("eprom", 0);

	map(0x50000000, 0x50000007).rw(m_sio, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask32(0xff00ff00);
	map(0x60000000, 0x60000003).rw(m_stc, FUNC(am9513_device::read16), FUNC(am9513_device::write16));

	map(0x70000000, 0x70000003).lr32([]() { return 0x00fe0000; }, "sw3"); // FIXME: possibly CPU board DIP switch?
	map(0x78000000, 0x78000003).lw32(
		[this](u32 data) { m_cpu->space(0).install_ram(0, m_ram->mask(), m_ram->pointer()); }, "ram_size"); // FIXME: ram size/enable?
}

void luna_68k_state::cpu_autovector_map(address_map &map)
{
	map(0xfffffff3, 0xfffffff3).lr8(NAME([]() { return m68000_base_device::autovector(1); }));
	map(0xfffffff5, 0xfffffff5).lr8(NAME([]() { return m68000_base_device::autovector(2); }));
	map(0xfffffff7, 0xfffffff7).lr8(NAME([]() { return m68000_base_device::autovector(3); }));
	map(0xfffffff9, 0xfffffff9).lr8(NAME([]() { return m68000_base_device::autovector(4); }));
	map(0xfffffffb, 0xfffffffb).lr8(NAME([]() { return m68000_base_device::autovector(5); }));
	map(0xfffffffd, 0xfffffffd).lr8(NAME([]() { return m68000_base_device::autovector(6); }));
	map(0xffffffff, 0xffffffff).lr8(NAME([]() { return m68000_base_device::autovector(7); }));
}

void luna_68k_state::luna(machine_config &config)
{
	M68030(config, m_cpu, 50_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &luna_68k_state::cpu_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &luna_68k_state::cpu_autovector_map);

	// 8 SIMMs for RAM arranged as two groups of 4, soldered
	RAM(config, m_ram);
	m_ram->set_default_size("16M");

	DS1287(config, m_rtc, 32'768);

	UPD7201(config, m_sio, 9'830'000 / 2); // D9.83B0

	// TODO: one of these is for console, other is keyboard
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	m_sio->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_serial[0]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxa_w));
	m_serial[0]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsa_w));

	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_sio->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_serial[1]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxb_w));
	m_serial[1]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsb_w));

	AM9513(config, m_stc, 9'830'000 / 2); // FIXME: clock? sources?
	m_stc->out4_cb().set(m_sio, FUNC(upd7201_device::rxca_w));
	m_stc->out4_cb().append(m_sio, FUNC(upd7201_device::txca_w));
	m_stc->out5_cb().set(m_sio, FUNC(upd7201_device::rxcb_w));
	m_stc->out5_cb().append(m_sio, FUNC(upd7201_device::txcb_w));
}

ROM_START(luna)
	ROM_REGION32_BE(0x20000, "eprom", 0)
	ROM_LOAD16_WORD_SWAP("0283__ac__8117__1.05.ic88", 0x00000, 0x20000, CRC(c46dec54) SHA1(22ef9274f4ef85d446d56cce13a68273dc55f10a))
ROM_END

} // anonymous namespace

/*   YEAR   NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS           INIT  COMPANY  FULLNAME  FLAGS */
COMP(1989?, luna, 0,      0,      luna,    0,     luna_68k_state, init, "Omron", "Luna",   MACHINE_IS_SKELETON)
