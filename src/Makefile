
LIBS=-lopentracing -lopentracing_mocktracer

default: testclient testserver

testclient: testclient.cpp simplehttpclient.cpp
	g++ -std=c++17 ${LIBS} testclient.cpp simplehttpclient.cpp -o testclient

testserver: testserver.cpp simplehttpserver.cpp
	g++ -std=c++17 ${LIBS} testserver.cpp simplehttpserver.cpp -o testserver

clean:
	rm testclient testserver
