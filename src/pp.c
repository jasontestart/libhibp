#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <openssl/sha.h>
#include <curl/curl.h>
#include "log.h"
#include "pp.h"

#define DEFAULT_API_URL "https://api.pwnedpasswords.com/range/"

/* 
 * In March 2026, with padding, we see as many as 2600 records (small sample).
 * The (6 yr old?) docs say to expect less than half that.
 * Each record should be max approx. 50 bytes.
 */
#define MAX_DOWNLOAD_SIZE 524288

// Helper for hex conversion without sprintf (faster/safer)
static void to_hex(const unsigned char *digest, char *output) {
    static const char hex[] = "0123456789ABCDEF";
    for (int i = 0; i < 20; i++) {
        output[i*2]   = hex[digest[i] >> 4];
        output[i*2+1] = hex[digest[i] & 0x0F];
    }
    output[40] = '\0';
}

int generate_sha1_digest(char *password, size_t size, char *hex_digest) {
    unsigned char message_digest[SHA1_DIGEST_LENGTH_BYTES];
	
    if (!SHA1((unsigned char*)password, size, message_digest)) {
        return -1;
    }

    to_hex(message_digest, hex_digest);
    return 0;
}

struct MemoryStruct {
    char *memory;
    size_t size;
};

/* Callback function for libcurl
 * Stolen from: https://curl.se/libcurl/c/getinmemory.html
*/
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    if (mem->size + realsize > MAX_DOWNLOAD_SIZE) {
        hibp_internal_log("unexpectedly large volume of data downloaded from Pwned Password API");
        return 0;
    }

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        /* out of memory! */
        hibp_internal_log("not enough memory (realloc returned NULL) while downloading from API");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

/* Look for the hash in the pwned password database */
int find_matching_entry(char hash[SHA1_HEX_DIGEST_LENGTH + 1], 
    PwnedResultStruct *pwned_result, char *proxy_server_url, char *api_url) {

    /* Google says URLs should be less that 2000 characters. */
    char url[1792];

    char hash_prefix[API_HASH_PREFIX_LENGTH] = {0};
    memcpy(hash_prefix, hash, API_HASH_PREFIX_LENGTH);
    const char *hash_suffix = hash + API_HASH_PREFIX_LENGTH;

    if (!pwned_result) {
		hibp_internal_log("NULL pointer pwned_result passed to function find_matching_entry.");
		return -1;
    }

    /* Construct the url we are passing to curl */
    snprintf(url,
		sizeof(url),
		"%s%s",
		(api_url == NULL) ? DEFAULT_API_URL : api_url,
	       	hash_prefix);

    CURL *curl;
    CURLcode result = CURLE_FAILED_INIT;

    struct MemoryStruct chunk;    
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if(curl) {
		/* set-up the headers */
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Accept: text/plain");
		headers = curl_slist_append(headers, "Add-Padding: true");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "MyHIBP-Security-Module/1.0");
		
		/* TIMEOUT: 2 seconds for the connection (DNS + TCP Handshake) */
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L);
		/* TIMEOUT: 5 seconds total for the entire transaction */
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

		if (proxy_server_url != NULL)
			curl_easy_setopt(curl, CURLOPT_PROXY, proxy_server_url);

        /* Perform the request, result gets the return code */
        result = curl_easy_perform(curl);

        /* Check for errors */
        if(result != CURLE_OK) {
            hibp_internal_log("curl_easy_perform() failed");
            hibp_internal_log(curl_easy_strerror(result));
        } else {
			pwned_result->occurences = 0; /* initialize */
			char *match;
			memset(pwned_result->hash, 0, sizeof(pwned_result->hash));
			match = strstr(chunk.memory, hash_suffix);
			if (match) {

				/* This block of code is really just a sanity check for the assertion. */
				memcpy(pwned_result->hash, hash, API_HASH_PREFIX_LENGTH);
				int j = 0;
				int i;
				for (i = API_HASH_PREFIX_LENGTH; i < SHA1_HEX_DIGEST_LENGTH; i++) {
					pwned_result->hash[i] = match[j];
					j++;
				}

				/* Validate data in the results struct */
                assert(strcmp(hash, pwned_result->hash) == 0);

				/* Now get the occurences - can't be more than 10 digits, right? */
				char occ_str[10];
				memset(occ_str, 0, sizeof(occ_str));
				j++;
				i = 0;
				while (match[j] != '\r' && match[j] != '\0' && match[j] != '\n') {
					occ_str[i] = match[j];
					i++;
					j++;
					if (i > 10)
						return -1;
				}

				pwned_result->occurences = atoll(occ_str);
				
				/* if we get a zero, then this probably a false positive from padding. */
				if (pwned_result->occurences == 0) {
					memset(pwned_result->hash, 0, sizeof(pwned_result->hash));
				}
			}
		}

		/* always cleanup */
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
    }
    free(chunk.memory);
    return (int)result;
}
