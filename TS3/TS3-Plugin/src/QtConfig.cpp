#include "QtConfig.h"

#include "config.h"

QtConfig::QtConfig(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	int toMove = screen->sendItems.size();
	if (toMove > 0)
	{
		this->resize(size().width(), size().height() + 30 * toMove);
		ui.closeButton->move(ui.closeButton->pos().x(), ui.closeButton->pos().y() + 30 * toMove);
		ui.addButton->move(ui.addButton->pos().x(), ui.addButton->pos().y() + 30 * toMove);
	}

	for (int i = 0; i < screen->sendItems.size(); i++)
	{
		QLineEdit* neu = new QLineEdit(this);
		neu->resize(261, 20);
		neu->move(10, 80 + 30 * i);
		neu->setText(QString(screen->sendItems[i].c_str()));
		inputLines.push_back(neu);
	}
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

void QtConfig::addPreset()
{
	
	int toMove = inputLines.size();
	this->resize(size().width(), size().height() + 30);
	ui.closeButton->move(ui.closeButton->pos().x(), ui.closeButton->pos().y() + 30);
	ui.addButton->move(ui.addButton->pos().x(), ui.addButton->pos().y() + 30);
	
	QLineEdit* neu = new QLineEdit(this);
	neu->resize(261, 20);
	neu->move(10, 80 + 30 * inputLines.size());
	neu->show();
	inputLines.push_back(neu);
}

void QtConfig::save()
{
	for (int i = 0; i < inputLines.size(); i++)
	{
		if (i >= screen->sendItems.size())
		{
			screen->sendItems.push_back(inputLines[i]->text().toStdString());
		}
	}

	for (int i = inputLines.size() - 1; i >= 0 ; i--)
	{
		if (inputLines[i]->text().isEmpty())
		{
			if (i < screen->sendItems.size())
				screen->sendItems.erase(screen->sendItems.begin() + i);
		}
	}
}
