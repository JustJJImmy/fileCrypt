//
//  askey.h
//  Telegram
//
//  Created by Allen Xu on 15/12/28.
//
//

#pragma once

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>


#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#define AES_CBC_BLOCK_SIZE  (16)
#define MAX_AES_DATA_BLOCK  (512)
#define MAX_AES_DATA_MTU    (AES_CBC_BLOCK_SIZE * 1024)
#define AES_CBC_IV_SIZE     (16)


inline bool aes256Encrypt(const quint8 * plain, quint32 plainLen, quint8 * cipher, quint32 * cipherLen, const quint8 * key, const quint8 * iv) {
	bool result = true;

	EVP_CIPHER_CTX *ctx;

	int len;

	quint32 ciphertextLen;

	quint32 block = plainLen / AES_CBC_BLOCK_SIZE;
	if (plainLen % AES_CBC_BLOCK_SIZE > 0) {
		block += 1;
	}

	quint8 * newPlain = (quint8 *)malloc(block * AES_CBC_BLOCK_SIZE);
	memset(newPlain, 0, block * AES_CBC_BLOCK_SIZE);
	memcpy(newPlain, plain, plainLen);

	/* Create and initialise the context */
	if (!(ctx = EVP_CIPHER_CTX_new())) {
		result = false;
		goto finalExit;
	}

	/* Initialise the encryption operation. IMPORTANT - ensure you use a key
	* and IV size appropriate for your cipher
	* In this example we are using 256 bit AES (i.e. a 256 bit key). The
	* IV size for *most* modes is the same as the block size. For AES this
	* is 128 bits */
	if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
		result = false;
		goto exit;
	}

	/* Provide the message to be encrypted, and obtain the encrypted output.
	* EVP_EncryptUpdate can be called multiple times if necessary
	*/
	if (1 != EVP_EncryptUpdate(ctx, cipher, &len, newPlain, block * AES_CBC_BLOCK_SIZE)) {
		result = false;
		goto exit;
	}

	ciphertextLen = len;

	/* Finalise the encryption. Further ciphertext bytes may be written at
	* this stage.
	*/
	if (1 != EVP_EncryptFinal_ex(ctx, cipher + len, &len)) {
		result = false;
		goto exit;
	}

	ciphertextLen += len;
	*cipherLen = ciphertextLen;

exit:
	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

finalExit:
	free(newPlain);
	return result;
}

inline bool aes256Decrypt(const quint8 * cipher, quint32 cipherLen, quint8 * plain, quint32 * plainLen, const quint8 * key, const quint8 * iv) {
	bool result = true;

	EVP_CIPHER_CTX *ctx;

	int len;

	int plaintextLen;

	/* Create and initialise the context */
	if (!(ctx = EVP_CIPHER_CTX_new())) {
		result = false;
		goto finalExit;
	}

	/* Initialise the decryption operation. IMPORTANT - ensure you use a key
	* and IV size appropriate for your cipher
	* In this example we are using 256 bit AES (i.e. a 256 bit key). The
	* IV size for *most* modes is the same as the block size. For AES this
	* is 128 bits */
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
		result = false;
		goto exit;
	}

	/* Provide the message to be decrypted, and obtain the plaintext output.
	* EVP_DecryptUpdate can be called multiple times if necessary
	*/
	if (1 != EVP_DecryptUpdate(ctx, plain, &len, cipher, cipherLen)) {
		result = false;
		goto exit;
	}

	plaintextLen = len;

	/* Finalise the decryption. Further plaintext bytes may be written at
	* this stage.
	*/
	if (1 != EVP_DecryptFinal_ex(ctx, plain + len, &len)) {
		result = false;
		goto exit;
	}

	plaintextLen += len;
	*plainLen = plaintextLen;

exit:
	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

finalExit:
	return result;
}

QString messageEncryption(QString plain);

QString messageDecryption(QString cipher);

bool dataEncryption(QByteArray & plainData, QByteArray & cipherData);
quint32 fileEncryption(QString plainFilename, QString cipherFilename);
bool dataDecryption(QByteArray & cipherData, QByteArray & plainData, quint32 originSize);
quint32 fileDecryption(QString cipherFilename, QString plainFilename, quint32 originSize);
