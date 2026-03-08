#include <unistd.h>
#include <syslog.h>
#include <stdio.h>
#include <errno.h>
#include <curl/curl.h>
#include <time.h>
#include "log.h"

static int log_to_syslog = 0;

__attribute__((constructor))
static void libhibp_init(void) {

    if (getuid() == 0 || geteuid() == 0) {
        log_to_syslog = 1;
        openlog("libhibp", LOG_PID | LOG_NDELAY, LOG_AUTHPRIV);
    } else {
        log_to_syslog = 0;
    }

    curl_global_init(CURL_GLOBAL_ALL);
}

__attribute__((destructor))
static void libhibp_cleanup(void) {

    if (log_to_syslog)
        closelog();
}

void hibp_internal_log(const char *msg) {
    int saved_errno = errno;
    static int tokens = 3;
    static time_t last_check = 0;
    time_t now = time(NULL);

    /* Refill tokens every 5 seconds */
    if (now - last_check > 5) {
        if (tokens < 3) tokens++;
        last_check = now;
    }

    if (tokens > 0) {
	if (log_to_syslog) {
            /* Log to secure system auth logs */
            syslog(LOG_AUTHPRIV | LOG_ERR, "[libhibp] %s (errno: %d)", msg, saved_errno);
	} else {
            fprintf(stderr, "libhibp error: %s (errno: %d)\n", msg, saved_errno);
	}
        tokens--;
    }
}
