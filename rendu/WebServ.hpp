/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rahmed <rahmed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/27 23:27:35 by rahmed            #+#    #+#             */
/*   Updated: 2022/09/11 19:31:53 by rahmed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

# include <exception>
# include <utility>

# include <iostream>
# include <fstream>
# include <cstring>

# include <vector>
# include <map>

# include <sys/signal.h>
# include <sys/socket.h> // For socket functions
# include <sys/types.h>

# ifdef MACOS
	#include <sys/event.h>
# endif
# ifdef LINUX
	#include <sys/epoll.h>
# endif

# include <sys/time.h>
# include <ctime>
# include <sys/un.h>
# include <sys/wait.h>
# include <netinet/in.h> // For sockaddr_in
# include <arpa/inet.h> // for htons
# include <fcntl.h> // for fcntl
# include <unistd.h> // For read
enum e_path_type
{
	TYPE_DIRECTORY = 1,
	TYPE_FILE,
	TYPE_UNKNOWN
};

// PARSING DATA STRUCTURES
struct server_route
{
	std::string					route;
	std::vector<std::string>	accept_methodes; // GET/POST/DELETE
	std::string					redirection;				  // Blanck if not redirect
	std::string					root_directory;				  // if not set, same than root_directory of the server
	std::string					index;
	bool						auto_index;
	bool						is_download_on;
	bool						is_CGI;
	std::string					cgi_ext;	  // extension of CGI-file interpreted
	std::string					cgi_name_cmd; // CMD to launch cgi
	std::string					upload_path;  // "" if no upload accept
};

struct server_config
{
	int									serv_state;
	std::vector<int>					ports;
	int									actual_port;
	std::vector<std::string>			hostname;
	std::string							actual_hostname;
	std::string							name;
	std::map<int, std::string>			errors_pages;
	std::string							root_directory;
	int									limit_size_body;
	std::vector<struct server_route>	routes;
	char**								envp;
};

// TMP VARIABLES
static std::string	dumb_reponse = "HTTP/1.1 200 OK\r\nServer: webServ\r\nContent-type: text/html\r\nContent-Length: 36\r\n\r\n<html><body>SUPER PAGE</body></html>";
static std::string	dumd_request = "GET / HTTP/1.1\r\nHost: localhost:6969\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:102.0) Gecko/20100101 Firefox/102.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nSec-Fetch-Dest: document\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-Site: none\r\nSec-Fetch-User: ?1 ";

// UTILS
extern std::map<int, std::string>			error_pages;
extern std::map<std::string, std::string>	content_file;

void								init_error_pages();
void								init_content_files();
std::string							get_redirection_page(std::string new_url, std::string server_name);
std::vector<std::string>			split(std::string const &str, char c);
std::string							read_file(std::string path);
int									type_path(std::string path);
std::string							get_extension_file(std::string path);

std::string							percent_encode(std::string str);
std::string							percent_decode(std::string str);
std::vector<struct server_config>	parse_file(std::string path);
std::string 						convert_uint_to_hex(unsigned int n);

int create_socket(int port);

std::string	to_string(int n);
int	to_int(std::string str, int base = 10);