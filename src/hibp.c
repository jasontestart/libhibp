#include "log.h"
#include "pp.h"
#include <string.h>
#include <stdio.h>
#include "hibp.h"

/* Public API */
__attribute__((visibility("default")))
long long is_pwned_password(char *password, char *proxy_server_url, char *api_url) {
    int result = 0;
    char mypassword[PASSWORD_MAX_LENGTH];
    char hex_digest[SHA1_HEX_DIGEST_LENGTH + 1];

    memset(mypassword, 0, sizeof(mypassword));
    memset(hex_digest, 0, sizeof(hex_digest));

    if (strlen(password) >= PASSWORD_MAX_LENGTH) {
        hibp_internal_log("Input password exceeds max length buffer");
        return -1;
    }
    
    memcpy(mypassword, password, strlen(password));

    result = generate_sha1_digest(mypassword, strlen(mypassword), hex_digest);
    if (result != 0) {
        hibp_internal_log("Crypto error generating SHA1 digest");
        memset(mypassword, 0, sizeof(mypassword));
        return -1;
    }

    PwnedResultStruct pwned_match;
    pwned_match.occurences = 0;

    result = find_matching_entry(hex_digest, &pwned_match, proxy_server_url, api_url);
    if (result != 0) {
        hibp_internal_log("Network/API error searching for password hash");
        memset(mypassword, 0, sizeof(mypassword));
        return -1;
    }

    memset(mypassword, 0, sizeof(mypassword));

    return pwned_match.occurences;
}
