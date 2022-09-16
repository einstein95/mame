// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "windowqt.h"

#include "breakpointswindow.h"
#include "dasmwindow.h"
#include "deviceswindow.h"
#include "logwindow.h"
#include "memorywindow.h"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"

#include "util/xmlfile.h"

#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>

bool WindowQt::s_refreshAll = false;
bool WindowQt::s_hideAll = false;


// Since all debug windows are intended to be top-level, this inherited
// constructor is always called with a nullptr parent.  The passed-in parent widget,
// however, is often used to place each child window & the code to do this can
// be found in most of the inherited classes.

WindowQt::WindowQt(running_machine &machine, QWidget *parent) :
	QMainWindow(parent),
	m_machine(machine)
{
	setAttribute(Qt::WA_DeleteOnClose, true);

	// The Debug menu bar
	QAction *debugActOpenMemory = new QAction("New &Memory Window", this);
	debugActOpenMemory->setShortcut(QKeySequence("Ctrl+M"));
	connect(debugActOpenMemory, &QAction::triggered, this, &WindowQt::debugActOpenMemory);

	QAction *debugActOpenDasm = new QAction("New &Disassembly Window", this);
	debugActOpenDasm->setShortcut(QKeySequence("Ctrl+D"));
	connect(debugActOpenDasm, &QAction::triggered, this, &WindowQt::debugActOpenDasm);

	QAction *debugActOpenLog = new QAction("New Error &Log Window", this);
	debugActOpenLog->setShortcut(QKeySequence("Ctrl+L"));
	connect(debugActOpenLog, &QAction::triggered, this, &WindowQt::debugActOpenLog);

	QAction *debugActOpenPoints = new QAction("New (&Break|Watch)points Window", this);
	debugActOpenPoints->setShortcut(QKeySequence("Ctrl+B"));
	connect(debugActOpenPoints, &QAction::triggered, this, &WindowQt::debugActOpenPoints);

	QAction *debugActOpenDevices = new QAction("New D&evices Window", this);
	connect(debugActOpenDevices, &QAction::triggered, this, &WindowQt::debugActOpenDevices);

	QAction *dbgActRun = new QAction("Run", this);
	dbgActRun->setShortcut(Qt::Key_F5);
	connect(dbgActRun, &QAction::triggered, this, &WindowQt::debugActRun);

	QAction *dbgActRunAndHide = new QAction("Run And Hide Debugger", this);
	dbgActRunAndHide->setShortcut(Qt::Key_F12);
	connect(dbgActRunAndHide, &QAction::triggered, this, &WindowQt::debugActRunAndHide);

	QAction *dbgActRunToNextCpu = new QAction("Run to Next CPU", this);
	dbgActRunToNextCpu->setShortcut(Qt::Key_F6);
	connect(dbgActRunToNextCpu, &QAction::triggered, this, &WindowQt::debugActRunToNextCpu);

	QAction *dbgActRunNextInt = new QAction("Run to Next Interrupt on This CPU", this);
	dbgActRunNextInt->setShortcut(Qt::Key_F7);
	connect(dbgActRunNextInt, &QAction::triggered, this, &WindowQt::debugActRunNextInt);

	QAction *dbgActRunNextVBlank = new QAction("Run to Next VBlank", this);
	dbgActRunNextVBlank->setShortcut(Qt::Key_F8);
	connect(dbgActRunNextVBlank, &QAction::triggered, this, &WindowQt::debugActRunNextVBlank);

	QAction *dbgActStepInto = new QAction("Step Into", this);
	dbgActStepInto->setShortcut(Qt::Key_F11);
	connect(dbgActStepInto, &QAction::triggered, this, &WindowQt::debugActStepInto);

	QAction *dbgActStepOver = new QAction("Step Over", this);
	dbgActStepOver->setShortcut(Qt::Key_F10);
	connect(dbgActStepOver, &QAction::triggered, this, &WindowQt::debugActStepOver);

	QAction *dbgActStepOut = new QAction("Step Out", this);
	dbgActStepOut->setShortcut(QKeySequence("Shift+F11"));
	connect(dbgActStepOut, &QAction::triggered, this, &WindowQt::debugActStepOut);

	QAction *dbgActSoftReset = new QAction("Soft Reset", this);
	dbgActSoftReset->setShortcut(Qt::Key_F3);
	connect(dbgActSoftReset, &QAction::triggered, this, &WindowQt::debugActSoftReset);

	QAction *dbgActHardReset = new QAction("Hard Reset", this);
	dbgActHardReset->setShortcut(QKeySequence("Shift+F3"));
	connect(dbgActHardReset, &QAction::triggered, this, &WindowQt::debugActHardReset);

	QAction *dbgActClose = new QAction("Close &Window", this);
	dbgActClose->setShortcut(QKeySequence::Close);
	connect(dbgActClose, &QAction::triggered, this, &WindowQt::debugActClose);

	QAction *dbgActQuit = new QAction("&Quit", this);
	dbgActQuit->setShortcut(QKeySequence::Quit);
	connect(dbgActQuit, &QAction::triggered, this, &WindowQt::debugActQuit);

	// Construct the menu
	QMenu *debugMenu = menuBar()->addMenu("&Debug");
	debugMenu->addAction(debugActOpenMemory);
	debugMenu->addAction(debugActOpenDasm);
	debugMenu->addAction(debugActOpenLog);
	debugMenu->addAction(debugActOpenPoints);
	debugMenu->addAction(debugActOpenDevices);
	debugMenu->addSeparator();
	debugMenu->addAction(dbgActRun);
	debugMenu->addAction(dbgActRunAndHide);
	debugMenu->addAction(dbgActRunToNextCpu);
	debugMenu->addAction(dbgActRunNextInt);
	debugMenu->addAction(dbgActRunNextVBlank);
	debugMenu->addSeparator();
	debugMenu->addAction(dbgActStepInto);
	debugMenu->addAction(dbgActStepOver);
	debugMenu->addAction(dbgActStepOut);
	debugMenu->addSeparator();
	debugMenu->addAction(dbgActSoftReset);
	debugMenu->addAction(dbgActHardReset);
	debugMenu->addSeparator();
	debugMenu->addAction(dbgActClose);
	debugMenu->addAction(dbgActQuit);
}


WindowQt::~WindowQt()
{
}

void WindowQt::debugActOpenMemory()
{
	MemoryWindow *foo = new MemoryWindow(m_machine, this);
	// A valiant effort, but it just doesn't wanna' hide behind the main window & not make a new toolbar icon
	// foo->setWindowFlags(Qt::Dialog);
	// foo->setWindowFlags(foo->windowFlags() & ~Qt::WindowStaysOnTopHint);
	foo->show();
}


void WindowQt::debugActOpenDasm()
{
	DasmWindow *foo = new DasmWindow(m_machine, this);
	// A valiant effort, but it just doesn't wanna' hide behind the main window & not make a new toolbar icon
	// foo->setWindowFlags(Qt::Dialog);
	// foo->setWindowFlags(foo->windowFlags() & ~Qt::WindowStaysOnTopHint);
	foo->show();
}


void WindowQt::debugActOpenLog()
{
	LogWindow *foo = new LogWindow(m_machine, this);
	// A valiant effort, but it just doesn't wanna' hide behind the main window & not make a new toolbar icon
	// foo->setWindowFlags(Qt::Dialog);
	// foo->setWindowFlags(foo->windowFlags() & ~Qt::WindowStaysOnTopHint);
	foo->show();
}


void WindowQt::debugActOpenPoints()
{
	BreakpointsWindow *foo = new BreakpointsWindow(m_machine, this);
	// A valiant effort, but it just doesn't wanna' hide behind the main window & not make a new toolbar icon
	// foo->setWindowFlags(Qt::Dialog);
	// foo->setWindowFlags(foo->windowFlags() & ~Qt::WindowStaysOnTopHint);
	foo->show();
}


void WindowQt::debugActOpenDevices()
{
	DevicesWindow *foo = new DevicesWindow(m_machine, this);
	// A valiant effort, but it just doesn't wanna' hide behind the main window & not make a new toolbar icon
	// foo->setWindowFlags(Qt::Dialog);
	// foo->setWindowFlags(foo->windowFlags() & ~Qt::WindowStaysOnTopHint);
	foo->show();
}


void WindowQt::debugActRun()
{
	m_machine.debugger().console().get_visible_cpu()->debug()->go();
}

void WindowQt::debugActRunAndHide()
{
	m_machine.debugger().console().get_visible_cpu()->debug()->go();
	hideAll();
}

void WindowQt::debugActRunToNextCpu()
{
	m_machine.debugger().console().get_visible_cpu()->debug()->go_next_device();
}

void WindowQt::debugActRunNextInt()
{
	m_machine.debugger().console().get_visible_cpu()->debug()->go_interrupt();
}

void WindowQt::debugActRunNextVBlank()
{
	m_machine.debugger().console().get_visible_cpu()->debug()->go_vblank();
}

void WindowQt::debugActStepInto()
{
	m_machine.debugger().console().get_visible_cpu()->debug()->single_step();
}

void WindowQt::debugActStepOver()
{
	m_machine.debugger().console().get_visible_cpu()->debug()->single_step_over();
}

void WindowQt::debugActStepOut()
{
	m_machine.debugger().console().get_visible_cpu()->debug()->single_step_out();
}

void WindowQt::debugActSoftReset()
{
	m_machine.schedule_soft_reset();
	m_machine.debugger().console().get_visible_cpu()->debug()->single_step();
}

void WindowQt::debugActHardReset()
{
	m_machine.schedule_hard_reset();
}

void WindowQt::debugActClose()
{
	close();
}

void WindowQt::debugActQuit()
{
	m_machine.schedule_exit();
}


void WindowQt::saveConfiguration(util::xml::data_node &parentnode)
{
	util::xml::data_node *const node = parentnode.add_child(osd::debugger::NODE_WINDOW, nullptr);
	if (node)
		saveConfigurationToNode(*node);
}


void WindowQt::saveConfigurationToNode(util::xml::data_node &node)
{
	node.set_attribute_int(osd::debugger::ATTR_WINDOW_POSITION_X, geometry().topLeft().x());
	node.set_attribute_int(osd::debugger::ATTR_WINDOW_POSITION_Y, geometry().topLeft().y());
	node.set_attribute_int(osd::debugger::ATTR_WINDOW_WIDTH, size().width());
	node.set_attribute_int(osd::debugger::ATTR_WINDOW_HEIGHT, size().height());
}


//=========================================================================
//  WindowQtConfig
//=========================================================================

void WindowQtConfig::applyToQWidget(QWidget *widget)
{
	widget->setGeometry(m_position.x(), m_position.y(), m_size.x(), m_size.y());
}


void WindowQtConfig::recoverFromXmlNode(util::xml::data_node const &node)
{
	m_size.setX(node.get_attribute_int(osd::debugger::ATTR_WINDOW_WIDTH, m_size.x()));
	m_size.setY(node.get_attribute_int(osd::debugger::ATTR_WINDOW_HEIGHT, m_size.y()));
	m_position.setX(node.get_attribute_int(osd::debugger::ATTR_WINDOW_POSITION_X, m_position.x()));
	m_position.setY(node.get_attribute_int(osd::debugger::ATTR_WINDOW_POSITION_Y, m_position.y()));
	m_type = node.get_attribute_int(osd::debugger::ATTR_WINDOW_TYPE, m_type);
}
