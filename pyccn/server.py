#!/usr/bin/env python

import pyccn
import sys

handler = pyccn.CCN ()

InterestBaseName = pyccn.Name ("ccnx:/my-local-prefix/simple-fetch/file")

class InterestClosure (pyccn.Closure):
    COUNTER = 0

    def upcall(self, kind, upcallInfo):
        if kind == pyccn.UPCALL_INTEREST:
            interest = upcallInfo.Interest

            print >> sys.stderr, "<< PyCCN %s" % interest.name
            
            content = "PyCCN LINE #%d\n" % self.COUNTER
            self.COUNTER = self.COUNTER + 1

            co = pyccn.ContentObject (name=interest.name, content=content, 
                                      signed_info=pyccn.SignedInfo (key_digest=handler.getDefaultKey ().publicKeyID, freshness=5))
            co.sign (handler.getDefaultKey ())

            handler.put (co)

        return pyccn.RESULT_OK

closure = InterestClosure ()
handler.setInterestFilter (InterestBaseName, closure)

while True:
    handler.run (500)
