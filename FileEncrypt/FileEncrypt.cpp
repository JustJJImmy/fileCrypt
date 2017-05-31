#include "FileEncrypt.h"

#include <QMessageBox>
#include <QFileDialog>

#include <windows.h>
#include <shellapi.h>
#include <QDebug>
#include "DataEncrypt.h"

#define __STDC_CONSTANT_MACROS

extern "C" {
#include "libavcodec/avcodec.h"
}


namespace {
	static QString lastPath = ".";
}

FileEncrypt::FileEncrypt(QWidget *parent)
	: QMainWindow(parent)
{

	if (objectName().isEmpty())
		setObjectName(QStringLiteral("FileEncryptClass"));
	resize(600, 400);

	menuBar = new QMenuBar(this);
	menuBar->setObjectName(QStringLiteral("menuBar"));
	setMenuBar(menuBar);

	mainToolBar = new QToolBar(this);
	mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
	addToolBar(mainToolBar);

	centralWidget = new QWidget(this);
	centralWidget->setObjectName(QStringLiteral("centralWidget"));
	setCentralWidget(centralWidget);

	statusBar = new QStatusBar(this);
	statusBar->setObjectName(QStringLiteral("statusBar"));
	setStatusBar(statusBar);

	setWindowTitle(QApplication::translate("FileEncryptClass", "FileEncrypt", 0));

	QMetaObject::connectSlotsByName(this);

	btnEnCrypt = new QPushButton(this);
	btnEnCrypt->setText("加密");
	btnEnCrypt->setGeometry(QRect(20, 40, 80, 40));

	btnDeCrypt = new QPushButton(this);
	btnDeCrypt->setText("解密");
	btnDeCrypt->setGeometry(QRect(20, 120, 80, 40));

	btnDataDecrypt = new QPushButton(this);
	btnDataDecrypt->setText("数据解密");
	btnDataDecrypt->setGeometry(QRect(20, 200, 80, 40));

	datainput = new QLineEdit(this);
	datainput->setGeometry(QRect(20, 250, width() - 40, 30));

	dataDecryptOut = new QLabel(this);
	dataDecryptOut->setGeometry(QRect(20, 280, width() - 40, 30));

	connect(btnEnCrypt, SIGNAL(clicked()), this, SLOT(btnEnCryptClick()));
	connect(btnDeCrypt, SIGNAL(clicked()), this, SLOT(btnDeCryptClick()));
	connect(btnDataDecrypt, SIGNAL(clicked()), this, SLOT(btnDataDeCryptClick()));

	qDebug() << QString().fromLocal8Bit(avcodec_configuration());

}

void FileEncrypt::btnEnCryptClick() {

	QString path = QFileDialog::getOpenFileName(this, tr("选择加密文件"), lastPath);
	if (path.length() <= 0) {
		return;
	}
	lastPath = path;
	QFile file(path);
	if ( !file.exists() ) {
		return;
	}

	quint64 length = file.size();
	if ( length <= 0 ){
		return;
	}

	quint64 length2 = fileEncryption(path, path + (".") + QString().setNum(length));

	QString detail;

	if (length != length2) {
		detail = tr("加密失败") + "\n " + path + "\n " + length + ", " + length2;
	} else {
		detail = tr("加密成功") + "\n" + path + "\n" + length;
	}

	QMessageBox::information(NULL, tr("加密"), detail);

}



void FileEncrypt::btnDeCryptClick() {
	QString path = QFileDialog::getOpenFileName(this, tr("选择解密文件"), lastPath);
	if (path.length() <= 0) {
		return;
	}
	lastPath = path;

	QStringList tList = path.split(".");
	if ( tList.count() <= 0 ){
		return;
	}

	quint64 length = tList.last().toULongLong();
	if (length <= 0 ){
		return;
	}
	
	QString dePath = path.left(path.lastIndexOf("."));
	if ( dePath.length() <= 0 ){
		return;
	}

	quint64 length2 = fileDecryption(path, dePath, length);


	QString detail;

	if (length != length2) {
		detail = tr("解密失败") + "\n " + path + "\n " + length + ", " + length2;
	} else {
		detail = tr("解密成功") + "\n" + dePath + "\n" + length;
	}

	QMessageBox::information(NULL, tr("解密"), detail);



	QString nameEscaped = QDir::toNativeSeparators(dePath).replace('"', tr("\"\""));
	ShellExecute(0, 0, tr("explorer").toStdWString().c_str(), (tr("/select,") + nameEscaped).toStdWString().c_str(), 0, SW_SHOWNORMAL);


}


void FileEncrypt::btnDataDeCryptClick() {
	QString dataToDecrypt = datainput->text();
	if (dataToDecrypt.count() <= 0 ){
		return;
	}
	QString plainData = messageDecryption(dataToDecrypt);

	dataDecryptOut->setText(plainData);

}
