#pragma once

#include <QWidget>
#include <qmainwindow.h>
#include "ui_QtConfig.h"
#include <qlineedit.h>

class QtConfig : public QMainWindow
{
	Q_OBJECT

public:
	QtConfig(QWidget *parent = Q_NULLPTR);
	~QtConfig();
public slots:
	void Init();
	void Shutdown();
	void addPreset();
	void save();
private:
	std::vector<QLineEdit*> inputLines;
	Ui::QtConfig ui;
};
