#ifndef HIBP_PP_H
#define HIBP_PP_H
#include <stddef.h>

#define SHA1_DIGEST_LENGTH_BYTES 20
#define SHA1_HEX_DIGEST_LENGTH 40

/* For k-anonymity. Pardon the use of "prefix" and "suffix". */
#define API_HASH_PREFIX_LENGTH 5
/* Total hex digest length less prefix length */
#define API_SHA1_HASH_SUFFIX_LENGTH 35

/*
 * Passwords are unlikely to get this long.
 * Setting this to be consistent with Linux PAM.
 * Active Directory's limit is 256.
 */ 
#define PASSWORD_MAX_LENGTH 512


typedef struct {
    char hash[SHA1_HEX_DIGEST_LENGTH + 1];
    long long occurences;
} PwnedResultStruct;

/* Use the OpenSSL SHA1() function to generate a HEX digest of the SHA1 hash */
__attribute__((visibility("hidden")))
int generate_sha1_digest(char *password, size_t size, char *hex_digest);

__attribute__((visibility("hidden")))
/* Leverage the CURL API to cheked the pwned passwords database */
int find_matching_entry(char hash[SHA1_HEX_DIGEST_LENGTH + 1],
	       	PwnedResultStruct *pwned_result, char *proxy_server_url, char *api_url);

#endif
