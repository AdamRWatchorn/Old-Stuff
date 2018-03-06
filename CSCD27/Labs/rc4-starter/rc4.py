#KS Algorithm given modified for Python

def KSA(key):

    seed = [0] * 256

    for i in range(0, 256):
        seed[i] = i

    j = 0

    for i in range(0, 256):

        j = (j + seed[i] + key[i % len(key)]) % 256
        temp = seed[i]
        seed[i] = seed[j]
        seed[j] = temp
    return seed

#PRGA Given modified for Python

def PRGA(seed):

    i = 0
    j = 0

    while True:
        i = (i + 1) % 256
        j = (j + seed[i]) % 256
        temp = seed[i]
        seed[i] = seed[j]
        seed[j] = temp
        prbytes = seed[(seed[i] + seed[j]) % 256]
        yield prbytes


def rc4(key, inputStream):
    ''' returns the RC4 encoding of inputStream based on the key
    (bytes, bytes) -> bytes
    '''

    seed = KSA(key)

    prbytes = PRGA(seed)

    #Python Bitwise Operator: ^

    outputStream = b''

    for c in inputStream:
        outputStream += bytes([c ^ prbytes.__next__()])


    return outputStream
