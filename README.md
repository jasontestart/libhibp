# libhibp

A lightweight C library for interacting with the **Have I Been Pwned** API using k-Anonymity.

## Dependencies

To build this library on a **Debian**-based distribution (e.g., Ubuntu, Mint, Kali) , you will need to install the following development packages:

```bash
sudo apt update
sudo apt install build-essential pkg-config libcurl4-openssl-dev libssl-dev
```

## Building & Installing
```bash
make
sudo make install
```

## Running the Example

To try the interactive password checker:

```bash
make examples
LD_LIBRARY_PATH=. ./examples/pwnedpassword
```

## Usage

Currently, the library implements one function:

```c
long long is_pwned_password(char *password, char *proxy_server_url, char *api_url);
```

The value of `proxy_server_url` is passed to `libcurl` as [CURLOPT_PROXY][https://curl.se/libcurl/c/CURLOPT_PROXY.html]. 
Set this to `NULL` if you aren't using a proxy.

The value of `api_url` can also be NULL, in which case it will use the default base URL `https://api.pwnedpasswords.com/range/`. The
URL at `api_url` is expected to behave like the default base URL. See the documentation for the [Pwned Password API][https://haveibeenpwned.com/API/v3#PwnedPasswords].

The function returns the number of occurences where `password` is breached, or a `-1` if there was an error.

Link against the library in your own projects using pkg-config:

```bash
gcc main.c $(pkg-config --cflags --libs libhibp) -o my_app
```

See also `examples/pwnedpassword.c`.
