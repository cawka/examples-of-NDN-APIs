#!/usr/bin/env python

import ndn
import sys

FILENAME = sys.argv[1]

face = ndn.Face ()

InterestBaseName = ndn.Name ("ccnx:/my-local-prefix/simple-fetch/file")
interest_tmpl = ndn.Interest (scope=1)

def OnData (name, interest, data, kind):
    print >> sys.stderr, "PyNDN %s" % data.name
    print data.content,

    seqnum = ndn.Name.seg2num (data.name[len(data.name)-1])
    if seqnum >= 10:
        return ndn.RESULT_OK;
    
    face.expressInterestSimple (ndn.Name (InterestBaseName).appendSegment (seqnum+1), 
                                OnData, OnTimeout, interest_tmpl)

    return ndn.RESULT_OK

def OnTimeout (name, interest):
    return ndn.RESULT_REEXPRESS

face.expressInterestSimple (ndn.Name (InterestBaseName).appendSegment (0), 
                            OnData, OnTimeout, interest_tmpl)

face.run (3000)
