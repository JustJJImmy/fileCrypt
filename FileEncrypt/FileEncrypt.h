#pragma once

#include <QtWidgets/QMainWindow>


#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include <QPushButton> 
#include <QtWidgets/qlineedit.h>
#include <QLabel>

class FileEncrypt : public QMainWindow
{
    Q_OBJECT

public:
    FileEncrypt(QWidget *parent = Q_NULLPTR);


public slots:
	void btnEnCryptClick();
	void btnDeCryptClick();
	void btnDataDeCryptClick();

private:

	QMenuBar *menuBar;
	QToolBar *mainToolBar;
	QWidget *centralWidget;
	QStatusBar *statusBar;


	QPushButton *btnEnCrypt;
	QPushButton *btnDeCrypt;

	QPushButton *btnDataDecrypt;
	QLineEdit *datainput;
	QLabel *dataDecryptOut;
};


