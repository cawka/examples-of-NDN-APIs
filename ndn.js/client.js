var handler = new NDN ({host:"127.0.0.1"});
// var handler = new NDN (); // will automatically connect to some hub on NDN testbed
handler.verify = false; // don't care about data verification here

var InterestBaseName = new Name ("ccnx:/my-local-prefix/simple-fetch/file");
var InterestTemplate = new Interest ();
InterestTemplate.scope = 1; // not really working

// define closure class
$.Class ("OnDataOrTimeoutClosure", {}, {
    upcall: function(kind, upcallInfo) {
        if (kind == Closure.UPCALL_CONTENT || kind == Closure.UPCALL_CONTENT_UNVERIFIED) { // disable content verification
            convertedData = DataUtils.toString (upcallInfo.contentObject.content);
            $("#output").append (convertedData);

            console.log ("<< NDN.js " + upcallInfo.contentObject.name.to_uri ());
            seqno = DataUtils.bigEndianToUnsignedInt (upcallInfo.contentObject.name.components[upcallInfo.contentObject.name.components.length - 1]);

            if (seqno >= 10) return Closure.RESULT_OK;

            interestName = new Name ().add (InterestBaseName).add ("lecture.pptx").addSegment (seqno + 1);
            handler.expressInterest (interestName, new OnDataOrTimeoutClosure (), InterestTemplate)

            return Closure.RESULT_OK;
        }
        else {
            return Closure.RESULT_REEXPRESS;
        }
    }
});

// send interest
interestName = new Name ().add (InterestBaseName).add ("lecture.pptx").addSegment (1);
handler.expressInterest (interestName, new OnDataOrTimeoutClosure (), InterestTemplate);
