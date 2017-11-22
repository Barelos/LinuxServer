#include <set>
#include <map>
#include <string>
#include <vector>
#include <regex>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct user_t{
public:
	std::string name;
	std::string password;
} user_t;
typedef std::vector<std::string> string_v;
typedef std::vector<user_t> user_v;

enum REPLY {USERNAME_IN, BAD_PASSWORD, OK};
std::string delimiter = "-";


class Server{
public:
	Server(int portnum);
	void work();
private:
	void add_to_data(std::string, std::string);
	void update_database();
	REPLY log_user(std::string, std::string);
	REPLY sign_user(std::string, std::string, bool);
	string_v parse_string(std::string);
	int welcome_socket;
	struct sockaddr_in server_addr, client_addr;
	user_v clients;
	char buffer[1024]; 
	socklen_t clilen;
	std::vector<std::string> parse_input(std::string);
};