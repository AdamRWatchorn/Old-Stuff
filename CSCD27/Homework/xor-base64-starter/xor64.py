from Crypto.Cipher import XOR
import base64

def keyResize(key, plaintext):

    while(len(key) < len(plaintext)):
        key += key

    if(len(key) > len(plaintext)):
        key = key[:len(plaintext)]

    return key.encode('UTF-8')

''' encrypts the plaintext (utf-8) with a key
    based on the xor cipher algorithm
    and return the ciphertext (base64 encoded)
    (string, string) -> string
'''
def encrypt(key, plaintext):

    byteKey = keyResize(key, plaintext)

    bytePlaintext = plaintext.encode('UTF-8')

    cipherObj = XOR.new(byteKey)

    encryptTextByte = cipherObj.encrypt(bytePlaintext)

    ciphertextByte = base64.b64encode(encryptTextByte)

    ciphertext = ciphertextByte.decode()

    return ciphertext

''' decrypts the ciphertext (base64 encoded) with a key
    based on the xor cipher algorithm
    and returns the plaintext (utf-8)
    (string, string) -> string
'''    
def decrypt(key, ciphertext):

    byteKey = keyResize(key, ciphertext)

    cipherObj = XOR.new(byteKey)

    byte64Cipher = ciphertext.encode()

    byteCipher = base64.b64decode(byte64Cipher)

    decryptTextByte = cipherObj.decrypt(byteCipher)

    plaintext = decryptTextByte.decode('UTF-8')

    return plaintext
