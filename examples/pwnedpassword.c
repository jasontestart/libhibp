#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h> // Required for tcgetattr, tcsetattr, read, STDIN_FILENO
#include <string.h>

#include "hibp.h"

/* Prompt for a password and do not echo back. */
void getPassword(char *password, size_t size) {
    struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ECHO);
    newt.c_lflag &= ~(ICANON);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    printf("Enter password: ");
    fflush(stdout);

    int i = 0;
    int c;
    while (i < size - 1 && (c = getchar()) != '\n' && c != EOF) {
        password[i++] = c;
    }
    password[i] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    printf("\n");
}

int main() {
    int occurences;
    char password[512];
    memset(password, 0, sizeof(password));

    getPassword(password, sizeof(password));

    /* occurences = is_pwned_password(password, "http://mysquid.internal:3128", "https://myapi.internal/range/"); */
    occurences = is_pwned_password(password, NULL, NULL);

    /* Erase the plain text password from memory - clean the whole buffer */
    memset(password, 0, sizeof(password));

    if (occurences < 0)
		exit(occurences);

    if (occurences > 0)
        printf("There are %d occurences of that password in the Pwned Password database.\n", occurences);
    else
        printf("That password was not found in the Pwned Password database.\n");
}
