#include <stdio.h>
#include <stdlib.h>

#include <ccn/ccn.h>
#include <ccn/uri.h>

const char *FILENAME = NULL;
struct ccn_charbuf *InterestBaseName = NULL;
struct ccn_charbuf *InterestTemplate = NULL;

// define a callback for the “closure”
enum ccn_upcall_res
on_data_callback(struct ccn_closure *selfp, enum ccn_upcall_kind kind, struct ccn_upcall_info *info)
{
  switch (kind)
    {
    case CCN_UPCALL_INTEREST_TIMED_OUT:
      // reexpress interest
      return CCN_UPCALL_RESULT_REEXPRESS; 
    case CCN_UPCALL_CONTENT:
    case CCN_UPCALL_CONTENT_UNVERIFIED:
      {
        const unsigned char *ccnb = info->content_ccnb;
        size_t ccnb_size = info->pco->offset[CCN_PCO_E];
        const unsigned char *ib = info->interest_ccnb;
        struct ccn_indexbuf *ic = info->content_comps;

        const unsigned char *data = NULL;
        size_t data_size = 0;
        
        int res = ccn_content_get_value(ccnb, ccnb_size, info->pco, &data, &data_size);
        if (res < 0)
          {
            fprintf (stderr, "%s\n", "Something is wrong");
            return CCN_UPCALL_RESULT_OK;
          }

        fwrite (data, 1, data_size, stdout);

        // figure out which segment it is
        int seqnum = 0;
        const unsigned char *seqno = NULL;
        size_t seqno_size = 0;
        res = ccn_name_comp_get (ccnb, ic, ic->n-2, &seqno, &seqno_size);
        if (seqno[0] == CCN_MARKER_SEQNUM) 
          {
            for (int i = 1; i < seqno_size; i++)
              {
                seqnum <<= 8;
                seqnum |= seqno [i];
              }
          }

        if (seqnum == 10)
          {
            return CCN_UPCALL_RESULT_OK;
          }
        
        // send interest for the next segment
        struct ccn_charbuf *interestName = ccn_charbuf_create ();
        ccn_charbuf_append_charbuf (interestName, InterestBaseName);
        ccn_name_append_numeric (interestName, CCN_MARKER_SEQNUM, seqnum+1);

        // debug created name
        struct ccn_charbuf *resultBuf = ccn_charbuf_create ();
        ccn_uri_append (resultBuf, interestName->buf, interestName->length, 0);
        fprintf (stderr, ">> %s\n", ccn_charbuf_as_string (resultBuf));
        ccn_charbuf_destroy (&resultBuf);

        ccn_express_interest (info->h, interestName, selfp, InterestTemplate);
        ccn_charbuf_destroy (&interestName);
        
        return CCN_UPCALL_RESULT_OK;
      }
    case CCN_UPCALL_FINAL: // can free custom memory in selpf->data, if necessary
      return CCN_UPCALL_RESULT_OK;
      
    default:
      fprintf (stderr, "%s: %d\n", "Unhandled upcall_kind", kind);
      return CCN_UPCALL_RESULT_ERR;
    }
}

int
main (int argc, char **argv)
{
  // create a handle
  struct ccn *handle = ccn_create ();
  int res = ccn_connect (handle, NULL);
  if (res < 0)
    {
      fprintf (stderr, "%s\n", "Cannot connect to ccnd");
      return 1;
    }

  // this code does not check for most of the bad conditions
  FILENAME = argv[1];

  // create name for the first segment of the file
  InterestBaseName = ccn_charbuf_create ();
  ccn_name_from_uri (InterestBaseName, "ccnx:/my-local-prefix/simple-fetch/file");
  ccn_name_append_str (InterestBaseName, FILENAME);

  // create a "template" for the interest (to specify scope 1: local apps)
  InterestTemplate = ccn_charbuf_create ();
  ccn_charbuf_append_tt (InterestTemplate, CCN_DTAG_Interest, CCN_DTAG);
  ccn_charbuf_append_tt (InterestTemplate, CCN_DTAG_Name, CCN_DTAG);
  ccn_charbuf_append_closer (InterestTemplate); /* </Name> */
  ccn_charbuf_append_tt (InterestTemplate, CCN_DTAG_Scope, CCN_DTAG);
  ccnb_append_number (InterestTemplate, 1);
  ccn_charbuf_append_closer (InterestTemplate); /* </Scope> */
  ccn_charbuf_append_closer (InterestTemplate); /* </Interest> */

  // create a closure
  struct ccn_closure *closure = calloc (1, sizeof(struct ccn_closure));
  closure->p = on_data_callback;
  // closure->data can specify custom data associated with the closure

  // send interest for the first segment
  struct ccn_charbuf *interestName = ccn_charbuf_create ();
  ccn_charbuf_append_charbuf (interestName, InterestBaseName);
  ccn_name_append_numeric (interestName, CCN_MARKER_SEQNUM, 0);

  // debug created name
  struct ccn_charbuf *resultBuf = ccn_charbuf_create ();
  ccn_uri_append (resultBuf, interestName->buf, interestName->length, 0);
  fprintf (stderr, ">> %s\n", ccn_charbuf_as_string (resultBuf));
  ccn_charbuf_destroy (&resultBuf);

  ccn_express_interest (handle, interestName, closure, InterestTemplate);
  ccn_charbuf_destroy (&interestName);

  ////////////////////////////////////////
  // start and wait in the processing loop
  ccn_run (handle, 4000);
  
  // may need stop everything and cleanup other memory
  ccn_disconnect (handle);
  ccn_destroy (&handle);
  
  // free memory (there are some memory "leaks")
  free (closure);
  ccn_charbuf_destroy (&InterestBaseName);

  return 0;
}
