
default: testclient testserver

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o )

CXXFLAGS=-std=c++17
LDFLAGS=-lopentracing -lopentracing_mocktracer

%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

testclient: testclient.o simplehttpclient.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

testserver: testserver.o simplehttpserver.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o testclient testserver

