#!/usr/bin/env python

import pyccn
import sys

FILENAME = sys.argv[1]

handler = pyccn.CCN ()

InterestBaseName = pyccn.Name ("ccnx:/my-local-prefix/simple-fetch/file")
interest_tmpl = pyccn.Interest (scope=1)

class DataTimeoutClosure (pyccn.Closure):
    def upcall(self, kind, upcallInfo):
        if kind == pyccn.UPCALL_CONTENT or kind == pyccn.UPCALL_CONTENT_UNVERIFIED:
            co = upcallInfo.ContentObject

            print >> sys.stderr, "PyCCN %s" % co.name
            print co.content,

            seqnum = pyccn.Name.seg2num (co.name[len(co.name)-1])
            if seqnum >= 10:
                return pyccn.RESULT_OK;

            handler.expressInterest (pyccn.Name (InterestBaseName).appendSegment (seqnum+1), 
                                     self, interest_tmpl)

        elif kind == pyccn.UPCALL_INTEREST_TIMED_OUT:
            return pyccn.RESULT_REEXPRESS

        return pyccn.RESULT_OK

closure = DataTimeoutClosure ()
handler.expressInterest (pyccn.Name (InterestBaseName).appendSegment (0), 
                         closure, interest_tmpl)

handler.run (3000)
