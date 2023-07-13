/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: me <me@student.42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/25 18:00:03 by abittel           #+#    #+#             */
/*   Updated: 2022/09/10 15:00:58 by me               ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Cgi.hpp"
#include <algorithm>

std::map<std::string, std::string> content_file;
std::map<int, std::string> error_pages;

Response::Response(struct server_config *cfg, int server_port, std::string server_hostname, std::string req) :
_cfg(cfg),_req(req), _rep_hd(""), _rep_bd(""), _server_port(server_port), _server_hostname(server_hostname), _cgi(NULL,NULL,NULL), _state_code(0)
{
}
Response::Response(Response const &cp) :  _cfg(cp._cfg), _req(cp._req), _rep_hd(cp._rep_hd), _rep_bd(cp._rep_bd), _server_port(cp._server_port), _server_hostname(cp._server_hostname), _cgi(cp._cgi), _state_code(cp._state_code)
{
}
Response &Response::operator=(Response const &cp)
{
	_rep_hd = cp._rep_hd;
	_rep_bd = cp._rep_bd;
	_cfg = cp._cfg;
	_req = cp._req;
	_server_hostname = cp._server_hostname;
	_server_port = cp._server_port;
	_state_code = cp._state_code;
	return *this;
}

Response::~Response()
{
}

void Response::set_request(std::string val)
{
	_req = val;
}

std::string const &Response::get_request() const
{
	return (_req._req);
}

void Response::create_files(struct server_route *route)
{
	std::string name_file;
	std::vector<std::string> lines;

	lines = split(_req._req, '\n');
	for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
	{
		std::string inter = *it;
		while (inter.find('-') != std::string::npos)
			inter.erase(inter.find('-'), 1);
		if (inter == _req._req_upload_token)
		{
			++it;
			if (it == lines.end())
				break;
			if (split(*it, ';').size() < 3 || !split(split(*it, ';')[2], '=').size() || split(split(*it, ';')[2], '=')[0] != " filename")
				continue;
			name_file = split(split(*it, ';')[2], '=')[1];
			name_file = name_file.substr(1, name_file.size() - 3);
			if (name_file == "" || name_file == "\"")
				continue;
			std::ofstream file;
			file.open((route->upload_path + (*(route->upload_path.end() - 1) != '/' ? "/" : "") + name_file).c_str(), std::ios::trunc);
			if (!file.is_open())
				throw std::runtime_error("CANNOT CREATE FILE TO UPLOAD");
			it+=3;
			for (; it + 1 != lines.end(); it++)
			{
				std::string inter = *(it);
				while (inter.find('-') != std::string::npos)
					inter.erase(inter.find('-'), 1);
				if (inter == _req._req_upload_token)
					break;
				file << *it << "\n";
			}
			file.close();
			--it;
		}
	}
}

int Response::get_body_no_route()
{
	if (_req._req_meth != "GET" && _req._req_meth != "POST")
		return (403);
	try
	{
		int type = type_path(_cfg->root_directory + "/" + _req._req_path);
		if (_req._req_path == "/" || type == TYPE_DIRECTORY)
		{
			try
			{
				_rep_bd = read_file(_cfg->root_directory + "/index.html");
				if (content_file.count(get_extension_file(_cfg->root_directory + "/index.html")))
					_rep_content_file = content_file.find(get_extension_file(_cfg->root_directory + "/index.html"))->second;
				else
					_rep_content_file = "text/plain";
			}
			catch (std::exception &e)
			{
				return (403);
			}
		}
		else
		{
			try
			{
				if (type == TYPE_FILE)
				{
					_rep_bd = read_file(_cfg->root_directory + _req._req_path);
					if (content_file.count(get_extension_file(_cfg->root_directory + _req._req_path)))
						_rep_content_file = content_file.find(get_extension_file(_cfg->root_directory + _req._req_path))->second;
					else
						_rep_content_file = "text/plain";
				}
				else
					return (404);
			}
			catch (std::exception &e)
			{
				return (404);
			}
		}
	}
	catch (std::exception &e)
	{
		return (404);
	}
	return (200);
}

int	Response::readCGI(struct server_config* _cfg, struct server_route* route)
{
	if (this->_state_code != STATE_READ_CGI)
		_cgi = CGI(this, _cfg, route);
	if (_state_code != STATE_READ_CGI && _cgi.get_body() == -1)
	{
		_state_code = STATE_READ_CGI;
		return (-1);
	}
	else if (_state_code == STATE_READ_CGI && _cgi.read_CGI(_cgi.fd_pipe_rd) == -1)
		return (-1);
	else
	{
		_rep_bd = _cgi.res;
		_rep_content_file = "text/plain";
		if (_rep_bd == "")
			return (1);
		{
			std::vector<std::string>	splt = split(_rep_bd, '\n');
			for (int i = 0; i < (int)splt.size() && splt[i] != "\r"; i++)
			{
				std::vector<std::string>	splt_space = split(splt[i], ' ');
				_rep_content_file = splt_space[1].substr(0, splt_space[1].size() - 1);
			}
		}
		//_rep_content_file = split(_rep_bd.substr(0, _rep_bd.find("\r\n")), ' ')[1];
		//_rep_content_file = _rep_content_file.substr(0, _rep_content_file.size() - 1);
		if (_rep_bd.find("\r\n\r\n") == std::string::npos)
			return (1);
		_rep_bd =  _rep_bd.substr(_rep_bd.find("\r\n\r\n"));
		_rep = _rep_bd + _rep_hd;
	}
	return (1);
}

int Response::put_request(server_route* route)
{
	int type = type_path(route->upload_path);
	if (type == TYPE_DIRECTORY)
	{
		std::ofstream file;
		file.open((route->upload_path + _req._req_path).c_str(), std::ios::trunc);
		if (!file.is_open())
			throw std::runtime_error("CANNOT CREATE FILE TO UPLOAD");
		std::vector<std::string> lines = split(_req._req, '\n');
		bool is_in_body = false;
		for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
		{
			if (*it == "\r" && !is_in_body)
				is_in_body = true;
			else if (is_in_body)
				file << *it << "\n";
		}
		file.close();
		if (content_file.count(get_extension_file(route->upload_path + _req._req_path)))
			_rep_content_file = content_file[get_extension_file(route->upload_path + _req._req_path)];
		else
			_rep_content_file = content_file[""];
		return (200);
	}
	else
		throw std::runtime_error("PUT FILE NOT GOOD PATH");
}

int Response::get_body_route(struct server_route *route)
{
	// ACCEPT METHOD
	bool is_meth_ok = false;
	for (std::vector<std::string>::iterator it = route->accept_methodes.begin(); it != route->accept_methodes.end(); it++)
		if (*it == _req._req_meth)
			is_meth_ok = true;
	if ((_req._req_meth == "GET" || _req._req_meth == "POST") && (route->is_CGI && route->cgi_ext == get_extension_file(_req._req_path)))
		is_meth_ok = true;
	if (!is_meth_ok)
		return (405);
	// UPLOAD FILES POST REQUEST
	if (_req._is_upload_req && route->upload_path != "")
	{
		try
		{
			int type = type_path(route->upload_path);
			if (type == TYPE_DIRECTORY)
				create_files(route);
			else
				throw std::runtime_error("UPLOAD FILE NOT GOOD PATH");
		}
		catch (const std::exception &e)
		{	std::cout << "Cannot upload file." << e.what() << std::endl;	}
	}
	//UPLOAD FILES PUT REQUEST
	if (_req._req_meth == "PUT")
	{
		try
		{	put_request(route);	}
		catch (const std::exception &e)
		{	std::cout << "Cannot put file." << e.what() << std::endl;	}
	}
	// REDIRECTION
	if (route->redirection != "")
	{
		get_redirection_page(route->redirection);
		return (200);
	}
	// TRY TO FIND PATH
	try
	{
		int type = type_path(route->root_directory + _req._req_path);
		if (type == TYPE_DIRECTORY) // IS_DIRECTORY
		{
			try
			{
				if (_req._req_meth == "DELETE")
				{
					std::remove((route->root_directory + _req._req_path + "/" + route->index).c_str());
					return (200);
				}
				_rep_bd = read_file(route->root_directory + _req._req_path + "/" + route->index);
				if (content_file.count(get_extension_file(route->root_directory + _req._req_path + "/" + route->index)))
				    _rep_content_file = content_file.find(get_extension_file(route->root_directory + _req._req_path + "/" + route->index))->second;
                else
                    _rep_content_file = "text/plain";
				return (200);
			}
			catch (std::exception &e)
			{
				if (!route->auto_index)
					return (404);
			}
		}
		else if (type == TYPE_FILE) //IS_FILE
		{
			try
			{
				if (_req._req_meth == "DELETE")
				{
					std::remove((route->root_directory + _req._req_path).c_str());
					return (200);
				}
				if (content_file.count(get_extension_file(route->root_directory + "/" + _req._req_path)))
					_rep_content_file = content_file[get_extension_file(route->root_directory + "/" + _req._req_path)];
				else
					_rep_content_file = "text/plain";
				if (route->is_download_on)
					_req._download_file = true;
				if (route->is_CGI && route->cgi_ext == get_extension_file(_req._req_path)) // CGI
					readCGI(_cfg, route);
				else
					_rep_bd = read_file(route->root_directory + _req._req_path);
				return (200);
			} catch (std::exception &e)	{	return (404);	}
		}
		else
			return (404); // NOT FOUND
	}
	catch (std::exception &e)
	{
		return (404);
	}
	// IF NOT FOUND IS AUTO_INDEX
	get_body_auto_index(route->root_directory + _req._req_path, route->route);
	return (200);
}

int Response::get_body()
{
	struct server_route *route = NULL;
	for (std::vector<struct server_route>::iterator it = _cfg->routes.begin(); it != _cfg->routes.end(); it++)
		if (_req._req_path.substr(0, it->route.size()) == it->route &&
			(route == NULL || it->route.size() > route->route.size()))
			route = it.base();
	if (route == NULL)
		return (get_body_no_route());
	else
	{
		_req._req_path = _req._req_path.substr(route->route.size(), _req._req_path.size() - route->route.size());
		return (get_body_route(route));
	}
}

void Response::get_body_auto_index(std::string root_directory, std::string route_str)
{
	DIR *dir;
	struct dirent *read_dir;
	std::vector<std::string> files;

	dir = opendir(root_directory.c_str());
	while ((read_dir = readdir(dir)) != NULL)
	{
		if (read_dir->d_name[0] != '.' || std::string(read_dir->d_name) == "..")
			files.push_back(std::string(read_dir->d_name) + (read_dir->d_type == DT_DIR ? "/" : ""));
	}
	closedir(dir);
	std::sort(files.begin(), files.end());
	_rep_bd = "<html> \
                <body> \
                    <h1>Index files</h1> \
                    <ul>";
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
		_rep_bd += "<li><a href=\"" + percent_encode(route_str + _req._req_path + (*(_req._req_path.end() - 1) != '/' ? "/" : "") + \
		(*((*it).end() - 1) == '/' ? (*it).substr(0, (*it).size() - 1) : *it)) + "\">" + *it + "</a></li>";
	_rep_bd += "</body></html>";
	_rep_content_file = content_file["html"];
}

std::string Response::get_error_page(int error)
{
	std::string res_hd = "";
	std::string res_bd = "";
	std::string error_content = error_pages[error];

	_rep_bd = "<html><body><h1>Error " + to_string(error) + " :</h1> <br><br><p> " + error_content + "</p></body></html>";
	if (_req._is_chunked_req)
		_rep_bd = (_rep_bd.size() > 0 ? convert_uint_to_hex((unsigned int)_rep_bd.size()) + "\r\n" : "") + _rep_bd + "\r\n0\r\n\r\n";
	_rep_hd += "HTTP/1.1 " + to_string(error) + " " + error_content + "\r\n";
	_rep_hd += "Access-Control-Allow-Origin: *\r\n";
	_rep_hd += "Server: " + _cfg->name + "\r\nContent-Type: text/html\r\n";
	if (!_req._is_chunked_req)
		_rep_hd += "Content-size: " + to_string(_rep_bd.size()) + "\r\n";
	else
		_rep_hd += "Transfer-Encoding: chunked\r\n";
	_rep_hd += "\r\n";
	_rep = _rep_bd + _rep_hd;
	return (_rep_hd + _rep_bd);
}

std::string Response::get_redirection_page(std::string new_url)
{
	this->_rep_bd = "<head><meta http-equiv=\"refresh\" content=\"0;URL=" + new_url + "\" /></head>";
	return (_rep_bd);
}

std::string Response::get_error_page_route(int error)
{
	std::string error_content = error_pages[error];
	_req._req_accept_content = content_file.find("html")->second;
	if (!_cfg->errors_pages.count(error))
		return (get_error_page(error));
	else
	{
		try
		{
			int type = type_path(_cfg->root_directory + "/" + _cfg->errors_pages.find(error)->second);
			if (type == TYPE_DIRECTORY || type == TYPE_FILE)
			{
				try
				{
					if (type == TYPE_DIRECTORY)
						_rep_bd = read_file(_cfg->root_directory + "/index.html");
					else
						_rep_bd = read_file(_cfg->root_directory + "/" + _cfg->errors_pages.find(error)->second);
					if (_req._is_chunked_req && _rep_bd.size())
						_rep_bd = (_rep_bd.size() ? convert_uint_to_hex((unsigned int)_rep_bd.size()) + "\r\n" : "") + _rep_bd + "\r\n0\r\n\r\n";
					_rep_hd += "HTTP/1.1 " + to_string(error) + " " + error_content + "\r\n";
					_rep_hd += "Access-Control-Allow-Origin: *\r\n";
					_rep_hd += "Server: " + _cfg->name + "\r\n";
					if (_rep_bd.size())
						_rep_hd += "Content-Type: text/html\r\n";
					if (!_req._is_chunked_req)
						_rep_hd += "Content-size: " + to_string(_rep_bd.size()) + "\r\n";
					else
						_rep_hd += "Transfer-Encoding: chunked\r\n";
					_rep_hd += "\r\n";
					_rep = _rep_bd + _rep_hd;
					return (_rep_hd + _rep_bd);
				}
				catch (std::exception &e)
				{
					return (get_error_page(error));
				}
			}
			else
				return (get_error_page(error));
		}
		catch (std::exception &e)
		{
			return (get_error_page(error));
		}
	}
	return (get_error_page(error));
}

int Response::get_header()
{
	if (_req._req_meth == "GET" || _req._req_meth == "POST" || _req._req_meth == "DELETE")
		_rep_hd += "HTTP/1.1 200 OK\r\n";
	else if (_req._req_meth == "PUT")
	{
		_rep_hd += "HTTP/1.1 201 Created\r\nServer: " + _cfg->name + "\r\n\r\n";
		return (200);
	}
	if (_req._download_file)
		_rep_hd += "Content-Disposition: attachment\r\n";
	_rep_hd += "Access-Control-Allow-Origin: *\r\n";
	_rep_hd += "Server: " + _cfg->name + "\r\nContent-Type: " + _rep_content_file + "\r\n";
	if (!_req._is_chunked_req && _rep_bd.size())
		_rep_hd += "Content-size: " + to_string(_rep_bd.size()) + "\r\n";
	else if (_rep_bd.size())
		_rep_hd += "Transfer-Encoding: chunked\r\n";
	_rep_hd += "\r\n";
	return (200);
}

int	Response::read_request(int fd_client)
{
	int	res = 0;
	try
	{
		if (_req._finish_read == false && (res = _req.read_request(fd_client)) != 200)
		{
			_state_code = STATE_READ_REQUEST;
			return (-1);
		}
		if (_req._req.size() > static_cast<unsigned int>(_cfg->limit_size_body))
		{
			_rep = get_error_page_route(413);
			_state_code = STATE_PROCESS;
			return (413);
		}
	} catch(std::exception& e)
	{
		_state_code = STATE_PROCESS;
	}
	_state_code = STATE_PROCESS;
	return (1);
}

int	Response::process(std::string req)
{
	(void)(req);
	int	res = 0;
	if ((_state_code == STATE_PROCESS) && (res = _req.parse_request()) != 200) // CHECK IF PARSE IS OK (code 200)
	{
		_rep = get_error_page_route(res);
		_state_code = STATE_WRITE_RESPONSE;
		return (1);
	}
	if (_state_code != STATE_READ_CGI && (res = get_body()) != 200) // CHECK IF PARSE IS OK (code 200)
	{
		if (res < 0)
		{
			//_state_code = STATE_READ_FILE;
			return (res);
		}
		_rep = get_error_page_route(res);
		_state_code = STATE_WRITE_RESPONSE;
		return (1);
	}
	else if (_state_code == STATE_READ_CGI)
	{
		if (readCGI(0, 0) == -1)
			return (-1);
	}
	if (_req._is_chunked_req && _rep_bd.size()) //ADD CHUNCKS SIZE
		_rep_bd = (_rep_bd.size() ? convert_uint_to_hex((unsigned int)_rep_bd.size()) + "\r\n" : "") + _rep_bd + "\r\n0\r\n\r\n";
	get_header();
	_rep = _rep_hd + _rep_bd;
	_state_code = STATE_WRITE_RESPONSE;
	// std::cout << _req._req << std::endl; // !test
	return (1);
}

std::string Response::get_rep()
{
	return (_rep);
}

void	Response::set_rep(std::string rep)
{
	this->_rep = rep;
}
