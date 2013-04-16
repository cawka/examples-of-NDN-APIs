#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ccn/ccn.h>
#include <ccn/uri.h>

int NeedStop = 0;

enum ccn_upcall_res
on_interest_callback (struct ccn_closure *selfp, enum ccn_upcall_kind kind, struct ccn_upcall_info *info)
{
    switch (kind)
      {
      case CCN_UPCALL_INTEREST:
        {
          // reply to interest
          struct ccn_signing_params sp = CCN_SIGNING_PARAMS_INIT; // can specify KeyLocator

          sp.template_ccnb = ccn_charbuf_create();
          ccn_charbuf_append_tt(sp.template_ccnb, CCN_DTAG_SignedInfo, CCN_DTAG);
          
          ccnb_tagged_putf(sp.template_ccnb, CCN_DTAG_FreshnessSeconds, "%ld", 5);
          sp.sp_flags |= CCN_SP_TEMPL_FRESHNESS;
          ccn_charbuf_append_closer (sp.template_ccnb);
          
          struct ccn_charbuf *data = ccn_charbuf_create ();
          struct ccn_charbuf *data_name = ccn_charbuf_create ();
          ccn_name_init(data_name);
          ccn_name_append_components (data_name, info->interest_ccnb,
                                      info->interest_comps->buf[0],
                                      info->interest_comps->buf[info->interest_comps->n - 1]);

          // debug
          struct ccn_charbuf *resultBuf = ccn_charbuf_create ();
          ccn_uri_append (resultBuf, data_name->buf, data_name->length, 0);
          fprintf (stderr, "<< '%s'\n", ccn_charbuf_as_string (resultBuf));
          ccn_charbuf_destroy (&resultBuf);
          
          static int COUNTER = 0;
          char dataBuffer[1024];
          sprintf (dataBuffer, "LINE #%d\n", COUNTER++);

          // create content object
          ccn_sign_content (info->h, data, data_name, &sp, dataBuffer, strlen (dataBuffer)+1);

          // reply to the interest
          ccn_put (info->h, data->buf, data->length);

          ccn_charbuf_destroy (&data_name);
          ccn_charbuf_destroy (&data);

          return CCN_UPCALL_RESULT_INTEREST_CONSUMED;
        }
      default:
        return CCN_UPCALL_RESULT_OK;
      }
}

int
main()
{
  // create a handle
  struct ccn *handle = ccn_create ();
  int res = ccn_connect (handle, NULL);
  if (res < 0)
    {
      fprintf (stderr, "%s\n", "Cannot connect to ccnd");
      return 1;
    }

  // create name for the first segment of the file
  struct ccn_charbuf *listenOnPrefix = ccn_charbuf_create ();
  ccn_name_from_uri (listenOnPrefix, "ccnx:/my-local-prefix/simple-fetch/file");

  struct ccn_closure interestClosure = {.p = on_interest_callback};
  ccn_set_interest_filter (handle, listenOnPrefix, &interestClosure);

  while (NeedStop == 0)
    {
      int res = ccn_run (handle, 500);
      if (res < 0)
        break;
    }
  
  // may need stop everything and cleanup other memory
  ccn_disconnect (handle);
  ccn_destroy (&handle);

  ccn_charbuf_destroy (&listenOnPrefix);
  
  return 0;
}
