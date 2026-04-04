# libhibp

A lightweight C library for interacting with the [Have I Been Pwned](https://haveibeenpwned.com/) **Pwned Password**
API using k-Anonymity.

## Dependencies

To build this library, you will need to install the development packages for cURL and OpenSSL.

### Debian-based installation
```bash
sudo apt update
sudo apt install build-essential pkg-config libcurl4-openssl-dev libssl-dev
```

### RHEL-based installation (tested with Rocky Linux)
```bash
sudo dnf groupinstall 'Development Tools'
sudo dnf install libcurl-devel
```

## Building & Installing
```bash
make
sudo make install
```

## Running the Example

To try the interactive password checker:

```bash
cd examples
make
LD_LIBRARY_PATH=.. ./pwnedpassword
```

## Usage

Currently, the library implements one function:

```c
#include <hibp.h>

long long is_pwned_password(char *password, char *proxy_server_url, char *api_url);
```

The value of `proxy_server_url` is passed to `libcurl` as [CURLOPT_PROXY](https://curl.se/libcurl/c/CURLOPT_PROXY.html). 
Set this to `NULL` if you aren't using a proxy.

The value of `api_url` can also be `NULL`, in which case it will use the default base URL `https://api.pwnedpasswords.com/range/`. The
URL at `api_url` is expected to behave like the default base URL. See the documentation for the [Pwned Password API](https://haveibeenpwned.com/API/v3#PwnedPasswords).

The function returns the number of occurences where `password` is breached, or a `-1` if there was an error.

Link against the library in your own projects using pkg-config:

```bash
gcc main.c $(pkg-config --cflags --libs libhibp) -o my_app
```

See [examples](examples) directory for example code and Makefile.

## History and Acknowledgements
While not a professional developer, the author wrote this as part of a "larger" personal project for an "ecosystem" of Linux system modules to interface with the Pwned Password API. Thanks to examples from the cURL team and all the body of knowledge that Google AI currated from StackOverlow providing the author with a good refresher after 25 years away from C programming.
