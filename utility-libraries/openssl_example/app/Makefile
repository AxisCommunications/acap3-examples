PROG1	= openssl_example
OBJS1	= $(PROG1).c
PROGS	= $(PROG1)

LDLIBS += -Llib
CFLAGS += -I/opt/build/openssl-1.1.1l/build/include/curl
CFLAGS += -I/opt/build/curl/build/include/curl
CFLAGS += -Wall \
	  -Wformat=2 \
	  -Wpointer-arith \
	  -Wbad-function-cast \
	  -Wstrict-prototypes \
	  -Wmissing-prototypes \
	  -Winline \
	  -Wdisabled-optimization \
	  -Wfloat-equal \
	  -W \
	  -Werror

# Pre-processing proxy setting
ifneq ($(strip $(OPENSSL_PROXY)),)
CFLAGS += -DOPENSSL_PROXY="\"$(OPENSSL_PROXY)\""
endif

SHLIB_DIR = ./lib
LDFLAGS = -L$(SHLIB_DIR) -Wl,--no-as-needed,-rpath,'$$ORIGIN/lib'
SHLIBS += -lm -lssl -lcrypto -lcurl

all:	$(PROGS)

$(PROG1): $(OBJS1)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SHLIBS) $(LDLIBS)  $^ $(LIBS) $(LDLIBS) -o $@

clean:
	rm -f $(PROGS) *.o *.eap *_LICENSE.txt package.conf* param.conf
