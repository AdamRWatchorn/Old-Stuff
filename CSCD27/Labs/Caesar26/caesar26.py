''' encrypts the plaintext with a key
    based on the caesar cipher algorithm
    and return the ciphertext
    (string, string) -> string
    REQ: key matches [0-9]*
    REQ: plaintext matches [a-z]*
'''
def encrypt(key, plaintext): 

    numKey = int(key)
    ciphertext = ""
    plaintext = plaintext.strip('\n')


    for character in plaintext:

        ciphertext += chr((ord(character) + numKey - ord('a')) % 26 + ord('a'))
        # E(x) = (ord(character) + numKey) mod 26

    return ciphertext

''' decrypts the ciphertext with a key
    based on the caesar cipher algorithm
    and returns the plaintext
    (string, string) -> string
    REQ: key matches [0-9]*
    REQ: ciphertext matches [a-z]*'''    
def decrypt(key, ciphertext):

    numKey = int(key)
    plaintext = ""
    ciphertext = ciphertext.strip('\n')


    for character in ciphertext:

        plaintext += chr((ord(character) - numKey - ord('a')) % 26 + ord('a'))
        # D(x) = (x - n) mod 26


    return plaintext
