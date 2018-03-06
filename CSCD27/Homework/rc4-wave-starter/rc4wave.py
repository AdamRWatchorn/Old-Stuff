from Crypto.Cipher import ARC4
import wave

def encrypt(key_filename, input_filename, output_filename):
    ''' 
    Encrypts the wave input file (input_filename) with the key (key_filename) using the rc4 cipher (from pycrypto)
    and writes the wave output file (output_filename). 
    The wave output file must be a playable wave file.
    (string, string, string) -> None
    '''

    keyFile = open(key_filename, "r")

    strKey = keyFile.read()

    strKey = strKey.strip('\n')

    key = strKey.encode()

    wRead = wave.open(input_filename, "rb")

    wWrite = wave.open(output_filename, "wb")

    #gives nchannels, sampwidth, framerate, nframes, comptype, compname
    readParams = wRead.getparams()

    wWrite.setparams(readParams)

    frames = wRead.getnframes()

    cipher = ARC4.new(key)

    for frame in range(0, frames):
        readFrame = wRead.readframes(1)
        wWrite.writeframes(cipher.encrypt(readFrame))

    wRead.close()

    wWrite.close()


    None
    
def decrypt(key_filename, input_filename, output_filename):
    ''' 
    Decrypts the wave input file (input_filename) with the key (key_filename) using the rc4 cipher (from pycrypto)
    and writes the wave output file (output_filename). 
    The wave output file must be a playable wave file. 
    (string, string, string) -> None
    '''

    keyFile = open(key_filename, "r")

    strKey = keyFile.read()

    strKey = strKey.strip('\n')

    key = strKey.encode()

    wRead = wave.open(input_filename, "rb")

    wWrite = wave.open(output_filename, "wb")

    #gives nchannels, sampwidth, framerate, nframes, comptype, compname
    readParams = wRead.getparams()

    wWrite.setparams(readParams)

    frames = wRead.getnframes()

    cipher = ARC4.new(key)

    for frame in range(0, frames):
        readFrame = wRead.readframes(1)
        wWrite.writeframes(cipher.decrypt(readFrame))

    wRead.close()

    wWrite.close()

    None
