/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: me <me@student.42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/08 13:06:35 by abittel           #+#    #+#             */
/*   Updated: 2022/09/10 16:51:17 by me               ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include <cstring>
#include <stdlib.h>

CGI::CGI(Response *rp, struct server_config *cfg, struct server_route *rt) :  res(""), _response(rp), _cfg(cfg), _route(rt)
{
}
CGI::CGI(CGI const&  rhs)
{
	*this = rhs;
}
CGI	const& CGI::operator=(CGI const&  rhs)
{
	this->_cfg = rhs._cfg;
	this->_route = rhs._route;
	this->_response = rhs._response;
	return *this;
}
CGI::~CGI(){}

std::map<std::string, std::string>	CGI::get_env_cgi()
{
	std::string	inter;
	std::map<std::string, std::string>	map_env;

	//CREATE GOOD ENVIRONMENT VARIABLES
	for (std::map<std::string, std::string>::iterator it = _response->_req._req_var.begin(); ; it++)
	{
		if (it != _response->_req._req_var.end() && it != _response->_req._req_var.begin())
			inter += "&";
		else if (it == _response->_req._req_var.end())
			break;
		inter += it->first + "=" + it->second;
	}
	//VARIABLES
	if (_response->_req._req_meth == "GET")
		map_env["QUERY_STRING"] = inter;
	else if (_response->_req._req_meth == "POST")
		post_variables = inter;
	for (int i = 0; _cfg->envp[i] != NULL; i++)
		map_env[std::string(_cfg->envp[i]).substr(0, inter.find('='))] = std::string(_cfg->envp[i]).substr(std::string(_cfg->envp[i]).find('=') + 1);
	map_env["SERBER_SOFTWARE"] = "WebServ/0.1";
	map_env["SERVER_NAME"] = _cfg->hostname[0];
	map_env["GATWAY_INTERFACE"] = "CGI/1.1";
	map_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	map_env["SERVER_PORT"] = to_string(_cfg->ports[0]);
	map_env["REQUEST_METHOD"] = _response->_req._req_meth;
	map_env["PATH_TRANSLATED"] = _route->root_directory + (*(_route->root_directory.end() - 1) == '/' ? "" : "/") + (*(_response->_req._req_path.begin()) == '/' ? _response->_req._req_path.substr(1) : _response->_req._req_path);
	map_env["PATH_INFO"] = "";
	map_env["SCRIPT_NAME"] = _response->_req._req_path;
	map_env["REMOTE_Host"] = "";
	map_env["REMOTE_ADDR"] = "";
	map_env["REMOTE_USER"] = "";
	map_env["REMOTE_IDENT"] = "";
	map_env["AUTH_TYPE"] = "";
	//map_env["CONTENT_TYPE"] = _response->_req_content_type;
	map_env["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
	map_env["CONTENT_LENGTH"] = to_string(_response->_req._req_content_length);
	map_env["REDIRECT_STATUS"] = "200";
	//map_env["DOCUMENT_ROOT"] = _cfg->root_directory;
	return (map_env);
}

char**	convert_map_to_env(std::map<std::string, std::string> const& map_env)
{
	char**	env;
	int		i = 0;
	std::string	inter;
	std::map<std::string, std::string>::const_iterator	it;
	std::map<std::string, std::string>::const_iterator	it_end = map_env.end();
	
	env = (char**)malloc(sizeof(char*) * (map_env.size() + 1));
	for (it = map_env.begin(); it != it_end; ++it)
	{
		inter = it->first + "=" + it->second;
		env[i] = (char*)malloc(sizeof(char) * (inter.size() + 1));
		strcpy(env[i], inter.c_str());
		++i;
	}
	env[i] = NULL;
	return (env);
}

void	CGI::exec_cgi(int *pip, int *pip2)
{
	close(pip[1]); close(pip2[0]);
	dup2(pip[0], STDIN_FILENO);
	dup2(pip2[1], STDOUT_FILENO);
	//dup2(STDERR_FILENO, STDOUT_FILENO);
	char **inter = (char**)malloc(sizeof(const char*) * 3);
	inter[0] = (char *)malloc(sizeof(char) * (_route->cgi_name_cmd.size() + 1));
	std::strcpy(inter[0], _route->cgi_name_cmd.c_str()); 
	std::string path_file = _route->root_directory + (*(_route->root_directory.end() - 1) == '/' ? "" : "/") + (*(_response->_req._req_path.begin()) == '/' ? _response->_req._req_path.substr(1) : _response->_req._req_path);
	inter[1] = (char *)malloc(sizeof(char) * (path_file.size() + 1));
	std::strcpy(inter[1], path_file.c_str());
	inter[2] = NULL;
	//std::cerr << inter[0] << " " << inter[1] << "|" << _route->cgi_name_cmd.c_str() << std::endl;
	execve(_route->cgi_name_cmd.c_str(), inter , convert_map_to_env(map_env));
	Response inter_rep(NULL, 0, "", "");
	inter_rep.get_error_page_route(500);
	write (1, inter_rep.get_rep().c_str(), inter_rep.get_rep().size());
	exit(1);
}

int	CGI::read_CGI(int fd)
{
	ssize_t nb_rd = 0;
	std::vector<char> buf(4096);
	do
	{
		nb_rd = (read(fd, &buf[0], 4096));
		if (nb_rd == -1)
		{
			return (-1);
		}
		this->res.append(buf.begin(), buf.begin() + nb_rd);
	} while (nb_rd > 0);
	close (fd);
	//std::cerr << this->res << std::endl;
	return (1);
}

int CGI::get_body()
{
	int pip[2];
	int pip2[2];

	pipe(pip);
	pipe(pip2);
	map_env = get_env_cgi();
	int pid = fork();
	if (pid == 0)
		exec_cgi(pip, pip2);
	else
	{
		close(pip2[1]);
		close(pip[0]);
		if (_response->_req._req_meth == "POST")
			write(pip[1], post_variables.c_str(), post_variables.size());
		close(pip[1]);
		fd_pipe_rd = pip2[0];
		//fcntl(fd_pipe_rd, F_SETFL, O_NONBLOCK);
		return (read_CGI(pip2[0]));
	}
	return (0);
}
