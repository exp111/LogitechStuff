#include "QtConfig.h"

#include "config.h"

QtConfig::QtConfig(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

QtConfig::~QtConfig()
{
}

void QtConfig::Init()
{
	screen->Init();
}

void QtConfig::Shutdown()
{
	screen->Shutdown();
}