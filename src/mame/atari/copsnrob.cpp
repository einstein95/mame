// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Atari Cops'n Robbers hardware

    driver by Zsolt Vasvari

    Games supported:
        * Cops'n Robbers

    Known issues:
        * none at this time

NOTE: While more revisions of Cops'n Robbers are suspected to exist, as yet
      no boards have been found.

****************************************************************************

    Cops'n Robbers memory map (preliminary)


    0000-00ff RAM
    0c00-0fff Video RAM
    1200-1fff ROM

    I/O Read:

    1000 Vertical Sync
    1002 Bit 0-6 Player 1 Gun Position
         Bit 7   Player 1 Gas Pedal
    1006 Same as above for Player 2
    100A Same as above for Player 3
    100E Same as above for Player 4
    1012 DIP Switches
         0-1 Coinage
         2-3 Time Limit
         4-7 Fire button for Player 1-4
    1016 Bit 6 - Start 1
         Bit 7 - Coin 1
    101A Bit 6 - Start 2
         Bit 7 - Coin 2

    I/O Write:

    0500-0507 (sounds/enable) - 0506: LED 1
    0600      Beer Truck Y
    0700-07ff Beer Truck Sync Area
    0800-08ff Bullets RAM
    0900-0903 Car Sprite #
    0a00-0a03 Car Y Pos
    0b00-0bff Car Sync Area
    1000      Sound effect and start led triggers must be here - 1000: LED 2
    1001-1003 ???


2008-08
Added Dip locations according to manual.

***************************************************************************/

#include "emu.h"
#include "copsnrob.h"

#include "cpu/m6502/m6502.h"

#include "copsnrob.lh"


/*************************************
 *
 *  I/O
 *
 *************************************/

uint8_t copsnrob_state::copsnrob_misc_r()
{
	return m_screen->vblank() ? 0x00 : 0x80;
}

void copsnrob_state::copsnrob_misc2_w(uint8_t data)
{
	m_misc = data & 0x7f;
	/* Multi Player Start */
	m_leds[1] = BIT(~data, 6);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void copsnrob_state::main_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x01ff).ram();
	map(0x0500, 0x0507).w("latch", FUNC(f9334_device::write_d0));
	map(0x0600, 0x0600).writeonly().share("trucky");
	map(0x0700, 0x07ff).writeonly().share("truckram");
	map(0x0800, 0x08ff).ram().share("bulletsram");
	map(0x0900, 0x0903).writeonly().share("carimage");
	map(0x0a00, 0x0a03).writeonly().share("cary");
	map(0x0b00, 0x0bff).ram();
	map(0x0c00, 0x0fff).ram().share("videoram");
	map(0x1000, 0x1000).r(FUNC(copsnrob_state::copsnrob_misc_r));
	map(0x1000, 0x1000).w(FUNC(copsnrob_state::copsnrob_misc2_w));
	map(0x1002, 0x1002).portr("CTRL1");
	map(0x1006, 0x1006).portr("CTRL2");
	map(0x100a, 0x100a).portr("CTRL3");
	map(0x100e, 0x100e).portr("CTRL4");
	map(0x1012, 0x1012).portr("DSW");
	map(0x1016, 0x1016).portr("IN1");
	map(0x101a, 0x101a).portr("IN2");
	map(0x1200, 0x1fff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static const ioport_value gun_table[] = {0x3f, 0x5f, 0x6f, 0x77, 0x7b, 0x7d, 0x7e};

static INPUT_PORTS_START( copsnrob )
	PORT_START("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!2,!1")
	PORT_DIPSETTING(    0x03, "1 Coin/1 Player" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Players" )
	PORT_DIPSETTING(    0x01, "1 Coin/Game" )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Player" )
	PORT_DIPNAME( 0x0c, 0x00, "Time Limit" ) PORT_DIPLOCATION("SW:!4,!3")
	PORT_DIPSETTING(    0x0c, "1min" )
	PORT_DIPSETTING(    0x08, "1min 45sec" )
	PORT_DIPSETTING(    0x04, "2min 20sec" )
	PORT_DIPSETTING(    0x00, "3min" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)

	PORT_START("CTRL1")
	PORT_BIT( 0x7f, 0x03, IPT_POSITIONAL_V ) PORT_POSITIONS(7) PORT_REMAP_TABLE(gun_table) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("CTRL2")
	PORT_BIT( 0x7f, 0x03, IPT_POSITIONAL_V ) PORT_POSITIONS(7) PORT_REMAP_TABLE(gun_table) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("CTRL3")
	PORT_BIT( 0x7f, 0x03, IPT_POSITIONAL_V ) PORT_POSITIONS(7) PORT_REMAP_TABLE(gun_table) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)

	PORT_START("CTRL4")
	PORT_BIT( 0x7f, 0x03, IPT_POSITIONAL_V ) PORT_POSITIONS(7) PORT_REMAP_TABLE(gun_table) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout carlayout =
{
	32,32,
	16,
	1,
	{ 0 },
	{  7,  6,  5,  4,  3,  2,  1,  0,
		15, 14, 13, 12, 11, 10,  9,  8,
		23, 22, 21, 20, 19, 18, 17, 16,
		31, 30, 29, 28, 27, 26, 25, 24},
	{  0*32,  1*32,  2*32,  3*32,  4*32,  5*32,  6*32,  7*32,
		8*32,  9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32,
		24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32 },
	32*32
};


static const gfx_layout trucklayout =
{
	16,32,
	2,
	1,
	{ 0 },
	{ 3*256+4, 3*256+5, 3*256+6, 3*256+7, 2*256+4, 2*256+5, 2*256+6, 2*256+7,
		1*256+4, 1*256+5, 1*256+6, 1*256+7, 0*256+4, 0*256+5, 0*256+6, 0*256+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8 },
	32*32
};


static GFXDECODE_START( gfx_copsnrob )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, carlayout,   0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, trucklayout, 0, 1 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void copsnrob_state::machine_start()
{
	save_item(NAME(m_ic_h3_data));
	save_item(NAME(m_misc));
	m_leds.resolve();
}

void copsnrob_state::machine_reset()
{
	m_ic_h3_data = 0;
	m_misc = 0;
}


void copsnrob_state::copsnrob(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 14.318181_MHz_XTAL / 16);      /* 894886.25 kHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &copsnrob_state::main_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 457, 0, 256, 261, 0, 200);
	// H RESET (synchronous) = 256H & 8H & 64H & 128H
	// V RESET (synchronous) = 256V & 4V
	// H BLANK signal should begin at 304, but 256H gates all relevant graphics
	m_screen->set_screen_update(FUNC(copsnrob_state::screen_update_copsnrob));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_copsnrob);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	copsnrob_audio(config);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( copsnrob )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "005777-01.l7",      0x1200, 0x0200, CRC(2b62d627) SHA1(ff4d3546ad931b8e8c5cffd65469814ba7200925) )
	ROM_LOAD( "005776-01.k7",      0x1400, 0x0200, CRC(7fb12a49) SHA1(8cd2f4bd2405835d06eb4d76d028e1b14a97b500) )
	ROM_LOAD( "005775-01.j7",      0x1600, 0x0200, CRC(627dee63) SHA1(6066ba9f5e12aa0c595eb60bcb468efa9f4495ef) )
	ROM_LOAD( "005774-01.h7",      0x1800, 0x0200, CRC(dfbcb7f2) SHA1(ccfda15f5f3e0caa1b44928e111469f337c39eca) )
	ROM_LOAD( "005773-01.e7",      0x1a00, 0x0200, CRC(ff7c95f4) SHA1(fd66d7e655ab96ec6ca4f8cf0d078c68b86ac75a) )
	ROM_LOAD( "005772-01.d7",      0x1c00, 0x0200, CRC(8d26afdc) SHA1(367f7e25c08a79277550d018681fffcdbd578029) )
	ROM_LOAD( "005771-01.b7",      0x1e00, 0x0200, CRC(d61758d6) SHA1(7ce9ad1096405126a8bf57c1f8bad1afa178b751) )

	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "005782-01.m3",      0x0000, 0x0200, CRC(82b86852) SHA1(17cf6698ceeb3b917d8ef13ed8242062d3bd57b8) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "005778-01.p1",      0x0000, 0x0200, CRC(78bff86a) SHA1(8bba352ff5e320abda9c897cac4c898862f3c3f5) )
	ROM_LOAD( "005779-01.m1",      0x0200, 0x0200, CRC(8b1d0d83) SHA1(3e45f55ddf10c7e9a221736c1f5a4cc5b4c8a317) )
	ROM_LOAD( "005780-01.l1",      0x0400, 0x0200, CRC(6f4c6bab) SHA1(88d4ce8e86116cabd6e522360c01538930268074) )
	ROM_LOAD( "005781-01.j1",      0x0600, 0x0200, CRC(c87f2f13) SHA1(18f31f46a3c7795e5d31ee55e8c98adc4c400328) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "005770-01.m2",      0x0000, 0x0100, CRC(b00bbe77) SHA1(3fd6113aa3a572ec9f5ff248ba1bf53fc9225dfb) )

	ROM_REGION( 0x0260, "proms", 0 )     /* misc. PROMs */
	ROM_LOAD( "005765-01.h8",      0x0000, 0x0020, CRC(6cd58931) SHA1(a90ae8ddffdfc33f60cb9ff8f42f9155c2b09ca1) ) // "µp Enable"
	ROM_LOAD( "005766-01.k8",      0x0020, 0x0020, CRC(e63edf4f) SHA1(1dc8691dde033062491b03d4c926047229c45a14) ) // "Enable A"
	ROM_LOAD( "005767-01.j8",      0x0040, 0x0020, CRC(381b5ae4) SHA1(91cd237878c0e092197e3025c2498b8f26f90109) ) // "Enable B"
	ROM_LOAD( "005768-01.n4",      0x0060, 0x0100, CRC(cb7fc836) SHA1(dc115c8dcee9298623f1e91add2dc17d0ed870e4) ) // horizontal timing
	ROM_LOAD( "005769-01.d5",      0x0160, 0x0100, CRC(75081a5a) SHA1(c7d60fc4c44cf9c160b874de92d37600c079e7b6) ) // vertical timing
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1976, copsnrob, 0, copsnrob, copsnrob, copsnrob_state, empty_init, ROT0, "Atari", "Cops'n Robbers", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_copsnrob )
