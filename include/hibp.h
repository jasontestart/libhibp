#ifndef HIBP_H
#define HIBP_H

// Export this symbol so the linker can find it in the .so
__attribute__((visibility("default")))
long long is_pwned_password(char *password, char *proxy_server_url, char *api_url);

#endif
