/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: me <me@student.42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/24 20:42:05 by abittel           #+#    #+#             */
/*   Updated: 2022/09/09 21:31:21 by me               ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include <unistd.h>

void init_config(struct server_config &val)
{
	char local_path[256];
	val.ports = std::vector<int>();
	val.name = "";
	val.errors_pages = std::map<int, std::string>();
	getcwd(local_path, 256);
	val.root_directory = std::string(local_path);
	val.limit_size_body = 4096;
	val.routes = std::vector<struct server_route>();
	val.hostname = std::vector<std::string>();
}

void init_route(struct server_route &val)
{
	val.route = "";
	val.accept_methodes = std::vector<std::string>();
	val.redirection = "";
	val.auto_index = false;
	val.is_CGI = false;
	val.cgi_ext = "";
	val.cgi_name_cmd = "";
	val.root_directory = "";
	val.is_download_on = false;
	val.upload_path = "";
}

int get_of_tabs(std::string &str)
{
	unsigned long i;
	for (i = 0; i < str.size() && str[i] == '\t'; i++)
	{
	}
	str = str.substr(i, str.size() - (i));
	return (static_cast<int>(i));
}

int is_blanck_str(std::string &str)
{
	for (unsigned long i = 0; i < str.size(); i++)
		if (str[i] != ' ' && str[i] != '\n' && str[i] != '\n')
			return (false);
	return (true);
}

void process_line(std::string &_line, struct server_config &cfg,
				  std::vector<struct server_config> &res,
				  bool &is_in_server, bool &is_in_location)
{
	std::string line = _line;
	int nb_tabs = get_of_tabs(line);
	std::vector<std::string> splt = split(line, ' ');
	if (!splt.size() || is_blanck_str(line))
		return;
	if (splt[0] == "listen")
	{
		if (splt.size() == 1 || !is_in_server || nb_tabs != 1)
			throw std::runtime_error("ERROR PARSING SERVER");
		for (unsigned long i = 1; i < splt.size(); i++)
		{
			if (splt[i].find(':') == std::string::npos)
			{
				cfg.hostname.push_back("127.0.0.1");
				cfg.ports.push_back(to_int(splt[i]));
				std::cout << "\033[1;33mListening on port \033[0m" + to_string(cfg.ports.back()) + "\n";
			}
			else
			{
				cfg.hostname.push_back(splt[i].substr(0, splt[i].find(':')));
				std::string port;
				port = splt[i].substr(splt[i].find(':') + 1, splt[i].size() - splt[i].find(':') - 1);
				std::cout << "\033[1;33mListening on port \033[0m" + port + "\n";
				cfg.ports.push_back(to_int(port));
			}
		}
		is_in_location = false;
	}
	else if (splt[0] == "server")
	{
		if (splt.size() > 1)
			throw std::runtime_error("ERROR PARSING SERVER");
		if (is_in_server)
			res.push_back(cfg);
		init_config(cfg);
		is_in_server = true;
		is_in_location = false;
	}
	else if (splt[0] == "server_name")
	{
		if (splt.size() != 2 || !is_in_server || nb_tabs != 1)
			throw std::runtime_error("ERROR PARSING SERVER_NAME");
		cfg.name = splt[1];
		is_in_location = false;
	}
	else if (splt[0] == "error_page")
	{
		if (splt.size() != 3 || nb_tabs != 1 || !is_in_server)
			throw std::runtime_error("ERROR PARSING ERROR_PAGE");
		cfg.errors_pages.insert(std::pair<int, std::string>(to_int(splt[1]), splt[2]));
		is_in_location = false;
	}
	else if (splt[0] == "location")
	{
		struct server_route inter;
		init_route(inter);
		if (splt.size() != 2 || nb_tabs != 1 || !is_in_server)
			throw std::runtime_error("ERROR PARSING LOCATION");
		inter.route = splt[1];
		cfg.routes.push_back(inter);
		is_in_location = true;
	}
	else if (splt[0] == "root")
	{
		if ((!is_in_location && nb_tabs == 2) || splt.size() != 2)
			throw std::runtime_error("ERROR PARSING ROOT");
		if (is_in_location)
			cfg.routes.back().root_directory = splt[1];
		if (!is_in_location)
			cfg.root_directory = splt[1];
	}
	else if (splt[0] == "methode")
	{
		if (!is_in_location || splt.size() == 1 || nb_tabs != 2)
			throw std::runtime_error("ERROR PARSING METHOD");
		for (unsigned long i = 1; i < splt.size(); i++)
			cfg.routes.back().accept_methodes.push_back(splt[i]);
	}
	else if (splt[0] == "autoindex")
	{
		if (!is_in_location || splt.size() == 1 || nb_tabs != 2 ||
			(splt[1] != "on" && splt[1] != "off"))
			throw std::runtime_error("ERROR PARSING METHOD");
		if (splt[1] == "on")
			cfg.routes.back().auto_index = true;
		else
			cfg.routes.back().auto_index = false;
	}
	else if (splt[0] == "redirect")
	{
		if (!is_in_location || splt.size() != 2 || nb_tabs != 2)
			throw std::runtime_error("ERROR PARSING METHOD");
		cfg.routes.back().redirection = splt[1];
	}
	else if (splt[0] == "cgi")
	{
		if (!is_in_location || splt.size() != 3 || nb_tabs != 2)
			throw std::runtime_error("ERROR PARSING METHOD");
		cfg.routes.back().is_CGI = true;
		cfg.routes.back().cgi_ext = splt[1];
		cfg.routes.back().cgi_name_cmd = splt[2];
	}
	else if (splt[0] == "index")
	{
		if (!is_in_location || splt.size() != 2 || nb_tabs != 2)
			throw std::runtime_error("ERROR PARSING METHOD");
		cfg.routes.back().index = splt[1];
	}
	else if (splt[0] == "download")
	{
		if (!is_in_location || splt.size() == 1 || nb_tabs != 2 ||
			(splt[1] != "on" && splt[1] != "off"))
			throw std::runtime_error("ERROR PARSING METHOD");
		if (splt[1] == "on")
			cfg.routes.back().is_download_on = true;
		else
			cfg.routes.back().is_download_on = false;
	}
	else if (splt[0] == "client_max_body_size")
	{
		if (!is_in_server || splt.size() != 2 || nb_tabs != 1)
			throw std::runtime_error("ERROR PARSING LIMIT SIZE BODY");
		cfg.limit_size_body = to_int(splt[1]);
	}
	else if (splt[0] == "upload")
	{
		if (!is_in_location || splt.size() != 2 || nb_tabs != 2)
			throw std::runtime_error("ERROR PARSING UPLOAD");
		cfg.routes.back().upload_path = splt[1];
		cfg.routes.back().accept_methodes.push_back("PUT");
	}
	else
		throw std::runtime_error("ERROR PARSING");
}

std::vector<struct server_config> parse_file(std::string path)
{
	std::string cfg;
	struct server_config inter;
	std::vector<struct server_config> res;

	bool is_in_server(false), is_in_location(false);
	try
	{
		cfg = read_file(path);
	}
	catch (const std::exception &e)
	{
		std::cout << "webserv: CANNOT READ CONFIG FILE" << std::endl;
		throw std::runtime_error("CANNOT READ FILE");
	}
	std::vector<std::string> splt;
	splt = split(cfg, '\n');
	int nb_line = 0;
	for (std::vector<std::string>::iterator it = splt.begin(); it != splt.end(); it++, nb_line++)
	{
		try
		{
			process_line(*it, inter, res, is_in_server, is_in_location);
		}
		catch (std::exception &e)
		{
			std::cout << "webserv: ERROR CONFIG FILE: line " << to_string(nb_line + 1) << std::endl;
			throw std::runtime_error("FATAL ERROR : parsing");
		}
	}
	if (is_in_server)
		res.push_back(inter);
	for (std::vector<struct server_config>::iterator it = res.begin(); it != res.end(); it++)
	{
		for (std::vector<struct server_route>::iterator it_r = it->routes.begin(); it_r != it->routes.end(); it_r++)
		{
			if (it_r->root_directory == "")
				it_r->root_directory = it->root_directory;
			if (it_r->index == "")
				it_r->index = "index.html";
			if (it_r->accept_methodes.size() == 0
			|| (it_r->accept_methodes.size() == 1 && it_r->accept_methodes[0] == "PUT"))
			{
				it_r->accept_methodes.push_back("GET");
				it_r->accept_methodes.push_back("POST");
				it_r->accept_methodes.push_back("DELETE");
			}
		}
	}
	return (res);
}
