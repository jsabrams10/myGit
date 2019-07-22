all: WTF WTFserver

WTF: client/WTF.c
	gcc -g -Wall -o client/WTF client/WTF.c -lssl -lcrypto

WTFserver: server/WTFserver.c
	gcc -g -Wall -o server/WTFserver server/WTFserver.c -lpthread -lssl -lcrypto

test: WTFtest.c
	gcc -g -Wall -o WTFtest WTFtest.c

clean:
	rm -r client/WTF server/WTFserver WTFtest testProj .configure
