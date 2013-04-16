
#include <ccnx-all.h>
#include <iostream>

using namespace std;

const char *FILENAME = NULL;
Ccnx::Name InterestBaseName;

// create a global handler
Ccnx::CcnxWrapper handler;

void OnData (Ccnx::Name name, Ccnx::PcoPtr pco);
void OnTimeout (Ccnx::Name name, const Ccnx::Closure &closure, Ccnx::Selectors selectors);

void OnData (Ccnx::Name name, Ccnx::PcoPtr pco)
{
  Ccnx::BytesPtr content = pco->contentPtr ();
  cout << string ((char*)Ccnx::head (*content), content->size ());

  int seqnum = name.getCompFromBackAsInt (0);
  if (seqnum >= 10)
    {
      return;
    }

  cerr << ">> C++ " << Ccnx::Name (InterestBaseName)(seqnum + 1) << endl; // a shortcut to construct name
  handler.sendInterest (Ccnx::Name (InterestBaseName)(seqnum + 1),
                        Ccnx::Closure (OnData, OnTimeout),
                        Ccnx::Selectors ().scope (Ccnx::Selectors::SCOPE_LOCAL_HOST));
}

void OnTimeout (Ccnx::Name name, const Ccnx::Closure &closure, Ccnx::Selectors selectors)
{
  // re-express interest
  handler.sendInterest (name, closure, selectors);
}

int
main (int argc, char **argv)
{
  // this code does not check for most of the bad conditions
  FILENAME = argv[1];

  InterestBaseName = Ccnx::Name ("ccnx:/my-local-prefix/simple-fetch/file");
  InterestBaseName.appendComp (FILENAME);

  cerr << ">> C++ " << Ccnx::Name (InterestBaseName)(0) << endl;
  handler.sendInterest (Ccnx::Name (InterestBaseName)(0),
                        Ccnx::Closure (OnData, OnTimeout),
                        Ccnx::Selectors ().scope (Ccnx::Selectors::SCOPE_LOCAL_HOST));

  sleep (3);
  return 0;
}
