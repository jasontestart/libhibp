CFLAGS += -I../include
LDFLAGS += -L..
LIBS += -lhibp

pwnedpassword: pwnedpassword.c
	$(CC) $(CFLAGS) pwnedpassword.c -o pwnedpassword $(LDFLAGS) $(LIBS)
	@echo ""
	@echo "Build complete. To run the example:"
	@echo "LD_LIBRARY_PATH=.. ./pwnedpassword"
	@echo ""

clean:
	rm -f pwnedpassword
