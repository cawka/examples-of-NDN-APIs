
#include <ccnx-cpp.h>
#include <iostream>

using namespace std;

Ccnx::Name InterestBaseName;

// create a global handler
Ccnx::Wrapper handler;

void OnInterest (Ccnx::Name name, Ccnx::Selectors selectors)
{
  cerr << name << endl;
  
  static int COUNTER = 0;

  ostringstream os;
  os << "C++ LINE #" << (COUNTER++) << endl;
  
  handler.publishData (name, os.str (), 5);
}

int
main (int argc, char **argv)
{
  InterestBaseName = Ccnx::Name ("ccnx:/my-local-prefix/simple-fetch/file");

  handler.setInterestFilter (InterestBaseName, OnInterest);
  
  while (true)
    {
      sleep (1);
    }
  return 0;
}
