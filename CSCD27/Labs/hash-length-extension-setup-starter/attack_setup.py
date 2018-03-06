from md5p import md5, padding

KEY = "secret"

#############
### Alice ###
#############

def createToken(message):
    return md5(KEY + message).hexdigest()

###########
### bob ###
###########

def verifyToken(message, token):

    return (token == md5(KEY + message).hexdigest())

###############
### Mallory ###
###############


def forgeIllegalPayload(message, token, extension):
    '''
    In this function, you are not allowed to:
    - call the function createToken
    - use the secret key
    However, you might know the length of the secret key (6)
    '''

    keyLen = 6

    h = md5(state=token.decode("hex"), count=512)

    h.update(extension)

    message2 = message + padding((len(message) + keyLen)*8) + extension

    return h.hexdigest(), message2
    

#############
### main ####
#############

if __name__ == "__main__":
    
    ################################
    # exchange between Alice and Bob
    ################################
    
    message = "get balance"
    # alice creates a message and its corresponding token and send them to Bob
    token = createToken(message)
     # Bobs receives the token and verifies it
    # prints true since the token is legitimate
    print(verifyToken(message, token))

    ##################################
    # exchange between Mallory and Bob
    ##################################
    
    extension = " after withdrawing 100"
     # Mallory forges an illegal token for the message + extension and send them to Bob
    illegalToken, IllegalMessage = forgeIllegalPayload(message, token, extension)
    # Bobs receives the token and verifies it
    # prints true, if the illegalToken is valid
    print(verifyToken(IllegalMessage, illegalToken))

