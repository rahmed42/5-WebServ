/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abittel <abittel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/08 13:03:55 by abittel           #+#    #+#             */
/*   Updated: 2022/08/27 22:36:53 by abittel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "WebServ.hpp"

class Response;

class CGI
{
	public:
		CGI(Response *rep, struct server_config *cfg, struct server_route *rt);
		CGI const& operator=(CGI const&  rhs);
		~CGI(void);
		CGI(CGI const&  cp);
		int get_body();
		int read_CGI(int fd);
		
		int									fd_pipe_rd;
		std::string							res;
	private:
		std::map<std::string, std::string>	get_env_cgi();
		void								exec_cgi(int *pip, int* pip2);

		Response							*_response;
		struct server_config				*_cfg;
		struct server_route					*_route;
		std::map<std::string, std::string>	map_env;
		std::string							post_variables;
};

#include "Response.hpp"