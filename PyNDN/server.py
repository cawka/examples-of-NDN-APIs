#!/usr/bin/env python

import ndn
import sys

face = ndn.Face ()

InterestBaseName = ndn.Name ("ccnx:/my-local-prefix/simple-fetch/file")
COUNTER = 0

def OnInterest (name, interest):
    global COUNTER
    print >> sys.stderr, "<< PyNDN %s" % interest.name
            
    content = "PyNDN LINE #%d\n" % COUNTER
    COUNTER = COUNTER + 1
    
    data = ndn.ContentObject (name = interest.name, content = content, 
                              signed_info = ndn.SignedInfo (key_digest = ndn.Key.getDefaultKey ().publicKeyID, 
                                                            freshness = 5,
                                                            key_locator = ndn.KeyLocator.getDefaultKeyLocator ()))
    data.sign (ndn.Key.getDefaultKey ())
    
    face.put (data)

    return ndn.RESULT_OK

face.setInterestFilterSimple (InterestBaseName, OnInterest)

while True:
    face.run (500)
