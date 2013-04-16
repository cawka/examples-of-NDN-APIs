// this code is not fully working yet

var handler = new NDN ({host:"127.0.0.1"});
// var handler = new NDN (); // will automatically connect to some hub on NDN testbed

handler.verify = false; // don't care about data verification here

var InterestBaseName = new Name ("ccnx:/my-local-prefix/simple-fetch/file");

var COUNTER = 0;

// define closure class
$.Class ("OnInterestClosure", {}, {
    upcall: function(kind, upcallInfo) {
        console.log (kind);
        if (kind == Closure.UPCALL_INTEREST) { // disable content verification
            console.log ("<< NDN.js " + upcallInfo.interest.name.to_uri ());

            content = "NDN.js LINE #" + COUNTER;
            COUNTER++;

            var co = new ContentObject (new Name(upcallInfo.interest.name), content);
            co.to_ccnb ();
            co.sign ();
            upcallInfo.contentObject = co;

            return Closure.RESULT_INTEREST_CONSUMED;
        }
        else {
            return Closure.RESULT_REEXPRESS;
        }
    }
});

// send interest
handler.registerPrefix (InterestBaseName, new OnInterestClosure ());
