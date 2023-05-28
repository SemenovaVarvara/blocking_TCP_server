all: server client

server: server.o diary1.o
	g++ server.o diary1.o -o server

server.o: server.cpp diary1.h
	g++ -fno-elide-constructors -std=c++17 -pedantic -Wall -Wextra -Wfloat-equal -Werror server.cpp -c

client: client.o diary1.o
	g++ client.o diary1.o -o client
	
client.o: client.cpp diary1.h
	g++ -fno-elide-constructors -std=c++17 -pedantic -Wall -Wextra -Wfloat-equal -Werror client.cpp -c

diary1.o: diary1.cpp diary1.h
	g++ -fno-elide-constructors -std=c++17 -pedantic -Wall -Wextra -Wfloat-equal -Werror diary1.cpp -c

clean: 
	rm -f *.o
	rm -f client
	rm -f server
