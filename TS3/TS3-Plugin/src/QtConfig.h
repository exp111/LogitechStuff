#pragma once

#include <QWidget>
#include <qmainwindow.h>
#include "ui_QtConfig.h"

class QtConfig : public QMainWindow
{
	Q_OBJECT

public:
	QtConfig(QWidget *parent = Q_NULLPTR);
	~QtConfig();
public slots:
	void Init();
	void Shutdown();
private:
	Ui::QtConfig ui;
};
