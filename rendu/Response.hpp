/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: me <me@student.42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/21 13:38:01 by abittel           #+#    #+#             */
/*   Updated: 2022/09/09 23:01:09 by me               ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "WebServ.hpp"
#include <dirent.h> //READ DIRECTORY
#include "Request.hpp"
#include "Cgi.hpp"

#define STATE_NULL 0
#define STATE_READ_REQUEST 1
#define STATE_READ_CGI 6
#define STATE_READ_FILE 2
#define STATE_PROCESS 5
#define STATE_WRITE_FILE 3
#define STATE_WRITE_RESPONSE 4

class Response
{
private:
	friend class CGI;
	
	struct server_config* _cfg;
	//Request
	Request	_req;
	// Response prop
	std::string							_rep_hd;
	std::string							_rep_bd;
	std::string							_rep_content_file;
	std::string							_rep;
	int									_server_port;
	std::string							_server_hostname;
	CGI									_cgi;


	int				get_body();
	int				get_header();
	int				get_body_no_route();
	int				get_body_route(struct server_route *route);
	std::string		get_error_page(int error);
	void			get_body_auto_index(std::string root_directory, std::string route_str);
	std::string		get_redirection_page(std::string new_url);
	void			create_files(struct server_route *route);
	int				readCGI(struct server_config* _cfg, struct server_route* route);
	int 			put_request(server_route* route);

public:
	// CANONICS FUNCTIONS
	Response(struct server_config *cfg = NULL, int server_port = 80, std::string server_hostname = "", std::string req = "");
	Response(Response const &cp);
	~Response();
	Response &operator=(Response const &cp);

	// Accessors
	void				set_request(std::string val);
	std::string const	&get_request() const;

	int					read_request(int fd_client);
	int					process(std::string req);
	std::string			get_rep();
	void			set_rep(std::string rep);
	
	//PUBLIC ATTRIBUTES
	int									_state_code;
	std::string		get_error_page_route(int error);
};
