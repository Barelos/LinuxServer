FLAGS = -Wextra -Wall -Wvla -pthread -std=c++11

defult: Server.h Server.cpp
	$(CXX) -std=c++11 $^ -o Server

client: Client.cpp
	$(CXX) $(FLAGS) $^ -o Client	