/**
 * Copyright (C) 2021, Axis Communications AB, Lund, Sweden
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/**
 * brief This example illustrates how to use external openssll and curl libraries
 * in an ACAP application
 *
 * This example shows how curl and openssl library can be used to get https
 * content from the URL using CA certificate and store the content locally.
 *
 */

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <curl.h>
#include <stdio.h>
#include <syslog.h>

/***** Struct declarations ***************************************************/
struct File {
  const char *filename;
  FILE *stream;
};

/***** Callback function  ****************************************************/

/**
 * brief This callback function is called when there is data to be written
 *
 * param buffer − This is the pointer to the array of elements to be written.
 * param size   − This is the size in bytes of each element to be written.
 * param nmemb  − This is the number of elements, each one with a size of size
 * bytes.  param stream − This is the pointer to a FILE object that specifies
 * an output stream.
 */
static size_t writefunction(void *buffer, size_t size, size_t nmemb, void *stream)
{
  struct File *out = (struct File *)stream;
  if(!out->stream) {
    // open file for writing
    out->stream = fopen(out->filename, "wb");
    if(!out->stream)
      return -1; // failure, can't open file to write
  }
  return fwrite(buffer, size, nmemb, out->stream);
}

/**
 * brief Curl SSL context callback for OpenSSL
 *
 * param curl   − Curl handle to perform the ssl operation on.
 * param sslctx − Pointer to ssl library.
 * param parm   − Argument to pass CURLOPT_SSL_CTX_DATA.
 */

static CURLcode sslctx_function(CURL *curl, void *sslctx, void *parm)
{
  CURLcode rv = CURLE_ABORTED_BY_CALLBACK;

  /** This example uses www.example.com certificates **/
  static const char mypem[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIG1TCCBb2gAwIBAgIQD74IsIVNBXOKsMzhya/uyTANBgkqhkiG9w0BAQsFADBP\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMSkwJwYDVQQDEyBE\n"
"aWdpQ2VydCBUTFMgUlNBIFNIQTI1NiAyMDIwIENBMTAeFw0yMDExMjQwMDAwMDBa\n"
"Fw0yMTEyMjUyMzU5NTlaMIGQMQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZv\n"
"cm5pYTEUMBIGA1UEBxMLTG9zIEFuZ2VsZXMxPDA6BgNVBAoTM0ludGVybmV0IENv\n"
"cnBvcmF0aW9uIGZvciBBc3NpZ25lZCBOYW1lcyBhbmQgTnVtYmVyczEYMBYGA1UE\n"
"AxMPd3d3LmV4YW1wbGUub3JnMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n"
"AQEAuvzuzMoKCP8Okx2zvgucA5YinrFPEK5RQP1TX7PEYUAoBO6i5hIAsIKFmFxt\n"
"W2sghERilU5rdnxQcF3fEx3sY4OtY6VSBPLPhLrbKozHLrQ8ZN/rYTb+hgNUeT7N\n"
"A1mP78IEkxAj4qG5tli4Jq41aCbUlCt7equGXokImhC+UY5IpQEZS0tKD4vu2ksZ\n"
"04Qetp0k8jWdAvMA27W3EwgHHNeVGWbJPC0Dn7RqPw13r7hFyS5TpleywjdY1nB7\n"
"ad6kcZXZbEcaFZ7ZuerA6RkPGE+PsnZRb1oFJkYoXimsuvkVFhWeHQXCGC1cuDWS\n"
"rM3cpQvOzKH2vS7d15+zGls4IwIDAQABo4IDaTCCA2UwHwYDVR0jBBgwFoAUt2ui\n"
"6qiqhIx56rTaD5iyxZV2ufQwHQYDVR0OBBYEFCYa+OSxsHKEztqBBtInmPvtOj0X\n"
"MIGBBgNVHREEejB4gg93d3cuZXhhbXBsZS5vcmeCC2V4YW1wbGUuY29tggtleGFt\n"
"cGxlLmVkdYILZXhhbXBsZS5uZXSCC2V4YW1wbGUub3Jngg93d3cuZXhhbXBsZS5j\n"
"b22CD3d3dy5leGFtcGxlLmVkdYIPd3d3LmV4YW1wbGUubmV0MA4GA1UdDwEB/wQE\n"
"AwIFoDAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwgYsGA1UdHwSBgzCB\n"
"gDA+oDygOoY4aHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0VExTUlNB\n"
"U0hBMjU2MjAyMENBMS5jcmwwPqA8oDqGOGh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNv\n"
"bS9EaWdpQ2VydFRMU1JTQVNIQTI1NjIwMjBDQTEuY3JsMEwGA1UdIARFMEMwNwYJ\n"
"YIZIAYb9bAEBMCowKAYIKwYBBQUHAgEWHGh0dHBzOi8vd3d3LmRpZ2ljZXJ0LmNv\n"
"bS9DUFMwCAYGZ4EMAQICMH0GCCsGAQUFBwEBBHEwbzAkBggrBgEFBQcwAYYYaHR0\n"
"cDovL29jc3AuZGlnaWNlcnQuY29tMEcGCCsGAQUFBzAChjtodHRwOi8vY2FjZXJ0\n"
"cy5kaWdpY2VydC5jb20vRGlnaUNlcnRUTFNSU0FTSEEyNTYyMDIwQ0ExLmNydDAM\n"
"BgNVHRMBAf8EAjAAMIIBBQYKKwYBBAHWeQIEAgSB9gSB8wDxAHcA9lyUL9F3MCIU\n"
"VBgIMJRWjuNNExkzv98MLyALzE7xZOMAAAF1+73YbgAABAMASDBGAiEApGuo0EOk\n"
"8QcyLe2cOX136HPBn+0iSgDFvprJtbYS3LECIQCN6F+Kx1LNDaEj1bW729tiE4gi\n"
"1nDsg14/yayUTIxYOgB2AFzcQ5L+5qtFRLFemtRW5hA3+9X6R9yhc5SyXub2xw7K\n"
"AAABdfu92M0AAAQDAEcwRQIgaqwR+gUJEv+bjokw3w4FbsqOWczttcIKPDM0qLAz\n"
"2qwCIQDa2FxRbWQKpqo9izUgEzpql092uWfLvvzMpFdntD8bvTANBgkqhkiG9w0B\n"
"AQsFAAOCAQEApyoQMFy4a3ob+GY49umgCtUTgoL4ZYlXpbjrEykdhGzs++MFEdce\n"
"MV4O4sAA5W0GSL49VW+6txE1turEz4TxMEy7M54RFyvJ0hlLLNCtXxcjhOHfF6I7\n"
"qH9pKXxIpmFfJj914jtbozazHM3jBFcwH/zJ+kuOSIBYJ5yix8Mm3BcC+uZs6oEB\n"
"XJKP0xgIF3B6wqNLbDr648/2/n7JVuWlThsUT6mYnXmxHsOrsQ0VhalGtuXCWOha\n"
"/sgUKGiQxrjIlH/hD4n6p9YJN6FitwAntb7xsV5FKAazVBXmw8isggHOhuIr4Xrk\n"
"vUzLnF7QYsJhvYtaYrZ2MLxGD+NFI8BkXw==\n"
"-----END CERTIFICATE-----\n";

  BIO *cbio = BIO_new_mem_buf(mypem, sizeof(mypem));
  X509_STORE  *cts = SSL_CTX_get_cert_store((SSL_CTX *)sslctx);
  int i;
  STACK_OF(X509_INFO) *inf;
  (void)curl;
  (void)parm;

  if(!cts || !cbio) {
    return rv;
  }

  inf = PEM_X509_INFO_read_bio(cbio, NULL, NULL, NULL);

  if(!inf) {
    BIO_free(cbio);
    return rv;
  }

  for(i = 0; i < sk_X509_INFO_num(inf); i++) {
    X509_INFO *itmp = sk_X509_INFO_value(inf, i);
    if(itmp->x509) {
      X509_STORE_add_cert(cts, itmp->x509);
    }
    if(itmp->crl) {
      X509_STORE_add_crl(cts, itmp->crl);
    }
  }

  sk_X509_INFO_pop_free(inf, X509_INFO_free);
  BIO_free(cbio);

  rv = CURLE_OK;
  return rv;
}

int main(void)
{
  CURL *curl;
  CURLcode rv;
  struct File file = {
    // name and path to store the file
    "/usr/local/packages/openssl_example/localdata/https.txt",
    NULL
  };

  openlog(NULL, LOG_PID, LOG_USER);

  // This function sets up the program environment that libcurl needs
  curl_global_init(CURL_GLOBAL_ALL);

   /**
    * This function must be the first function to be called, and it
    * returns a CURL easy handle that you must use as input to other
    * functions in the easy interface. This call MUST have a
    * corresponding call to curl_easy_cleanup when the operation is
    * complete.
    */
  curl = curl_easy_init();

  if (curl) {
    syslog(LOG_INFO,"CURL init succesful - Curl handle is created");

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    // Define our callback to get called when there is data to be written
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

    curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
#ifdef OPENSSL_PROXY
    //Set the proxy for upcoming request, if it is defined.
    curl_easy_setopt(curl, CURLOPT_PROXY, OPENSSL_PROXY);
#endif
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.example.com/");

    /**
     * Turn off the default CA locations, otherwise libcurl will load CA
     * certificates from the locations that were detected/specified at
     * build-time
     */
    curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
    curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);

    /**
     * first try: retrieve page without ca certificates -> should fail
     * unless libcurl was built --with-ca-fallback enabled at build-time
     */
    rv = curl_easy_perform(curl);
    syslog(LOG_INFO,"***Transfer requested without CA-cert***\n");
    if(rv == CURLE_OK)
      syslog(LOG_INFO,"***Transfer succeeded: This condition shouldnot happen***\n");
    else
      syslog(LOG_INFO,"***Transfer failed : Expected result, first transfer must fail ***\n");

    /**
     * Use a fresh connection (optional)
     * this option seriously impacts performance of multiple transfers but
     * it is necessary order to demonstrate this example. recall that the
     * ssl ctx callback is only called _before_ an SSL connection is
     * established, therefore it will not affect existing verified SSL
     * connections already in the connection cache associated with this
     * handle. normally you would set the ssl ctx function before making
     * any transfers, and not use this option.
     */
    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);

    /**
     * Second try: retrieve page using cacerts' certificate -> will succeed
     * load the certificate by installing a function doing the necessary
     * "modifications" to the SSL CONTEXT just before link init
     */
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, sslctx_function);
    rv = curl_easy_perform(curl);

    syslog(LOG_INFO,"*** Re-Transfer requested with CA-cert ***\n");
    if(rv == CURLE_OK)
      syslog(LOG_INFO, "*** Repeat transfer: Transfer Succeeded***\n");
    else
     syslog(LOG_INFO, "*** Repeat transfer: Transfer Failed***\n");

    if(file.stream)
      fclose(file.stream); /* close the local file */

    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }
  return 0;
}
