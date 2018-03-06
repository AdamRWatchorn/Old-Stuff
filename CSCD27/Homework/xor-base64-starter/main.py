#!/usr/local/bin/python3
import os, sys, getopt

from xor64 import encrypt, decrypt

def run(mode, keyFile, outputFile, inputFile):
    with open(keyFile, "r") as keyStream:
        key = keyStream.read()
        with open(inputFile, "r") as inputStream:
            data = inputStream.read()
            output = mode(key, data)
            with open(outputFile, "w") as outputStream:
                outputStream.write(output)
    
def usage():
    print ('Usage:	' + os.path.basename(__file__) + ' options file ')
    print ('Options:')
    print ('\t -e, --encrypt')
    print ('\t -d, --decrypt')
    print ('\t -k key_file, --key=key_file')
    print ('\t -o output_file, --output=output_file')
    sys.exit(2)

def main(argv):
   try:
      opts, args = getopt.getopt(argv,"hedk:o:",["help", "encrypt", "decrypt", "key=", "output="])
   except getopt.GetoptError as err:
      print(err)
      usage()
   # extract arguments
   mode = None
   keyFile = None
   outputFile = None
   inputFile = args[0] if len(args) > 0 else None
   for opt, arg in opts:
        if opt in ("-h", "--help"):
           usage()
        elif opt in ("-e", "--encrypt"):
           mode = encrypt
        elif opt in ("-d", "--decrypt"):
           mode = decrypt
        elif opt in ("-k", "--key"):
           keyFile = arg
        elif opt in ("-o", "--output"):
           outputFile = arg
   # check arguments
   if (mode is None):
       print('encrypt/decrypt option is missing\n')
       usage()
   if (keyFile is None):
       print('key option is missing\n')
       usage()
   if (outputFile is None):
       print('output option is missing\n')
       usage()
   if (inputFile is None):
       print('input file is missing\n')
       usage()
   # run command
   run(mode, keyFile, outputFile, inputFile)

if __name__ == "__main__":
   main(sys.argv[1:])