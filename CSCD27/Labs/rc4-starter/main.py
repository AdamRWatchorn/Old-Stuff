#!/usr/local/bin/python3
import os, sys, getopt

from rc4 import rc4

def run(keyFile, outputFile, inputFile):
    with open(keyFile, "rb") as keyStream:
        key = keyStream.read()
        with open(inputFile, "rb") as inputStream:
            data = inputStream.read()
            output = rc4(key, data)
            with open(outputFile, "wb") as outputStream:
                outputStream.write(output)
    
def usage():
    print ('Usage:	' + os.path.basename(__file__) + ' options file ')
    print ('Options:')
    print ('\t -k key_file, --key=key_file')
    print ('\t -o output_file, --output=output_file')
    sys.exit(2)

def main(argv):
   try:
      opts, args = getopt.getopt(argv,"hk:o:",["help", "key=", "output="])
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
        elif opt in ("-k", "--key"):
           keyFile = arg
        elif opt in ("-o", "--output"):
           outputFile = arg
   # check arguments
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
   run(keyFile, outputFile, inputFile)

if __name__ == "__main__":
   main(sys.argv[1:])