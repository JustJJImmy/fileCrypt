//
//  askey.cpp
//  Telegram
//
//  Created by Allen Xu on 15/12/28.
//
//
#include "DataEncrypt.h"
#include "stdlib.h"

#include <QFile>

static quint8 SEK[] = {
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xAB,
	0xCD,
	0xEF,
	0xFE,
	0xDC,
	0xBA,
	0x98,
	0x76,
	0x54,
	0x32,
	0x10,
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xAB,
	0xCD,
	0xEF,
	0xFE,
	0xDC,
	0xBA,
	0x98,
	0x76,
	0x54,
	0x32,
	0x10
};

static quint8 IV[] = {
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xAB,
	0xCD,
	0xEF,
	0xFE,
	0xDC,
	0xBA,
	0x98,
	0x76,
	0x54,
	0x32,
	0x10
};

QMutex askeyMutex;

QString printableBuf(quint8 * p, quint32 s) {
	QString result;
	const uchar *buf((const uchar*)p);
	const char *hex = "0123456789ABCDEF";
	result.reserve(s * 3);
	for (quint32 i = 0; i < s; ++i) {
		result += hex[(buf[i] >> 4)];
		result += hex[buf[i] & 0x0F];
		result += ' ';
	}
	result.chop(1);
	return result;
}

QString messageEncryption(QString plain) {
	QMutexLocker locker(&askeyMutex);
	quint8 * buffer = (quint8 *)malloc(MAX_AES_DATA_MTU);
	quint32 cipherLen = 0;
	QByteArray pba = plain.toUtf8();

	if (!aes256Encrypt((const quint8 *)pba.data(), pba.length(), buffer, &cipherLen, SEK, IV)) {
		return QString();
	}

	QByteArray cba((const char *)buffer, cipherLen);
	QByteArray hex = cba.toHex();

	delete buffer;
	return QString(hex);
}

QString messageDecryption(QString cipher) {
	QMutexLocker locker(&askeyMutex);
	quint8 * buffer = (quint8 *)malloc(MAX_AES_DATA_MTU);
	memset(buffer, 0, MAX_AES_DATA_MTU);
	quint32 plainLen = 0;
	QByteArray cba = QByteArray::fromHex(cipher.toLocal8Bit());

	if (!aes256Decrypt((const quint8 *)cba.data(), cba.length(), buffer, &plainLen, SEK, IV)) {
		return QString();
	}

	QByteArray pba((const char *)buffer, plainLen);
	delete buffer;
	return QString(pba).trimmed();
}

bool dataEncryption(QByteArray & plainData, QByteArray & cipherData) {

	quint32 totalBlockNum = plainData.length() / MAX_AES_DATA_BLOCK;
	quint32 blockNum = 0;

	if (plainData.length() % MAX_AES_DATA_BLOCK != 0) {
		totalBlockNum += 1;
	}

	quint8 *cipherBytes = (quint8 *)malloc(MAX_AES_DATA_BLOCK + AES_CBC_IV_SIZE);
	quint8 *plainBytes = (quint8 *)malloc(MAX_AES_DATA_BLOCK);

	quint32 plainLength = 0;
	quint32 cipherLength = 0;

	quint8 * plainRawData = (quint8 *)plainData.data();

	while (blockNum < totalBlockNum) {
		memset(plainBytes, 0, (MAX_AES_DATA_BLOCK));
		memset(cipherBytes, 0, (MAX_AES_DATA_BLOCK + AES_CBC_IV_SIZE));

		if (blockNum == totalBlockNum - 1) {
			plainLength = plainData.length() % MAX_AES_DATA_BLOCK;
			if (plainLength == 0) {
				plainLength = MAX_AES_DATA_BLOCK;
			}
		}
		else {
			plainLength = MAX_AES_DATA_BLOCK;
		}

		memcpy(plainBytes, plainRawData, plainLength);

		if (!aes256Encrypt((const quint8 *)plainBytes, MAX_AES_DATA_BLOCK, cipherBytes, &cipherLength, SEK, IV)) {
			free(plainBytes);
			free(cipherBytes);
			return false;
		}

		cipherData.append(QByteArray((const char *)cipherBytes, cipherLength));

		blockNum++;
		plainRawData += MAX_AES_DATA_BLOCK;
	}

	free(plainBytes);
	free(cipherBytes);
	return true;
}

quint32 fileEncryption(QString plainFilename, QString cipherFilename) {

	QMutexLocker locker(&askeyMutex);

	quint32 plainFileLen = 0;
	quint32 cipherFileLen = 0;

	QFile plainFile;
	plainFile.setFileName(plainFilename);
	if (!plainFile.open(QFile::ReadOnly)) {
		return 0;
	}

	QFile cipherFile;
	cipherFile.setFileName(cipherFilename);
	if (!cipherFile.open(QFile::WriteOnly)) {
		plainFile.close();
		return 0;
	}
	cipherFile.resize(0);

	QByteArray plainFileData;
	QByteArray cipherFileData;

	int failedTimes = 0;
	int plainBlockSize = MAX_AES_DATA_BLOCK; // 512
	while (!plainFile.atEnd()) {
		plainFileData.clear();
		cipherFileData.clear();
		// 一次读�?1000 �?block 加密
		plainFileData = plainFile.read(1000 * plainBlockSize);
		failedTimes = 0;
		for (; failedTimes < 3; failedTimes++) {

			if (dataEncryption(plainFileData, cipherFileData)) {
				break;
			}
		}
		if (failedTimes >= 3) {
			cipherFile.close();
			plainFile.close();
			return 0;
		}
		plainFileLen += plainFileData.length();
		cipherFile.write(cipherFileData);
		cipherFile.flush();

	}

	cipherFile.close();
	plainFile.close();


	return plainFileLen;
}

bool dataDecryption(QByteArray & cipherData, QByteArray & plainData, quint32 originSize) {

	quint32 totalBlockNum = cipherData.length() / (MAX_AES_DATA_BLOCK + AES_CBC_IV_SIZE);
	quint32 blockNum = 0;
	quint32 deltaBytesNum = totalBlockNum * MAX_AES_DATA_BLOCK - originSize;

	if (deltaBytesNum >= MAX_AES_DATA_BLOCK) {
		deltaBytesNum = 0;
	}

	quint8 *plainBytes = (quint8 *)malloc(MAX_AES_DATA_BLOCK + AES_CBC_IV_SIZE);

	quint32 blockLength = MAX_AES_DATA_BLOCK + AES_CBC_IV_SIZE;
	quint32 plainLength = 0;

	quint8 * cipherBytes = (quint8 *)cipherData.data();
	while (blockNum < totalBlockNum) {
		memset(plainBytes, 0, (MAX_AES_DATA_BLOCK + AES_CBC_IV_SIZE));

		if (!aes256Decrypt((const quint8 *)cipherBytes, (MAX_AES_DATA_BLOCK + AES_CBC_IV_SIZE), plainBytes, &plainLength, SEK, IV)) {
			free(plainBytes);
			return false;
		}

		if (blockNum == totalBlockNum - 1) {
			plainLength = plainLength - deltaBytesNum;
		}

		plainData.append(QByteArray((const char *)plainBytes, plainLength));

		blockNum++;
		cipherBytes += MAX_AES_DATA_BLOCK + AES_CBC_IV_SIZE;
	}

	free(plainBytes);
	return true;
}

quint32 fileDecryption(QString cipherFilename, QString plainFilename, quint32 originSize) {
	QMutexLocker locker(&askeyMutex);
	quint32 _plainAllSize = originSize;
	QFile cipherFile;
	cipherFile.setFileName(cipherFilename);
	if (!cipherFile.open(QFile::ReadOnly)) {
		return 0;
	}

	QFile plainFile;
	plainFile.setFileName(plainFilename);
	if (!plainFile.open(QFile::WriteOnly)) {
		return 0;
	}
	plainFile.resize(0);

	quint32 plainFileLen = 0;
	quint32 cipherFileLen = 0;

	QByteArray plainFileData;
	QByteArray cipherFileData;

	int failedTimes = 0;
	int cipherBlockSize = MAX_AES_DATA_BLOCK + AES_CBC_IV_SIZE; // 512 + 16
	while (!cipherFile.atEnd()) {
		plainFileData.clear();
		cipherFileData.clear();
		// 一次读�?1000 �?block  解密
		cipherFileData = cipherFile.read(1000 * cipherBlockSize);
		failedTimes = 0;
		for (; failedTimes < 3; failedTimes++) {
			if (dataDecryption(cipherFileData, plainFileData, _plainAllSize)) {
				break;
			}
		}
		if (failedTimes >= 3) {
			return 0;
		}
		_plainAllSize -= plainFileData.length();
		cipherFileLen += cipherFileData.length();
		plainFileLen += plainFileData.length();
		plainFile.write(plainFileData);
		plainFile.flush();

	}

	plainFile.close();
	cipherFile.close();

	return plainFileLen;
}


