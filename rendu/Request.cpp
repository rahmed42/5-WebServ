/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: me <me@student.42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/14 00:13:59 by abittel           #+#    #+#             */
/*   Updated: 2022/09/10 16:20:42 by me               ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request(std::string req) : \
_req(req), _req_meth(""), _req_http_type(""), _req_path(""), _req_accept_content(""), 	\
_req_content_length(0), _req_content_type(""), _req_var(), _is_upload_req(false),  	 	 \
_req_upload_token(""), _download_file(false), _is_chunked_req(false), _finish_read(false), \
gnl_line(""), nb_rd(0)
{
	this->parse_request();
}
Request::~Request()
{}
Request::Request(Request const &src)
{
	*this = src;
}
Request const& Request::operator=(Request const &rhs)
{
	this->_req = rhs._req;
	this->_req_meth = rhs._req_meth;
	this->_req_http_type = rhs._req_http_type;
	this->_req_path = rhs._req_path;
	this->_req_accept_content = rhs._req_accept_content;
	this->_req_content_length = rhs._req_content_length;
	this->_req_content_type = rhs._req_content_type;
	this->_req_var = rhs._req_var;
	this->_is_upload_req = rhs._is_upload_req;
	this->_req_upload_token = rhs._req_upload_token;
	this->_download_file = rhs._download_file;
	this->_is_chunked_req = rhs._is_chunked_req;
	this->_finish_read = rhs._finish_read;
	this->gnl_line = rhs.gnl_line;
	this->nb_rd = rhs.nb_rd;
	return (*this);
}

void Request::set_request(std::string val)
{
	_req = val;
}

std::string	Request::get_next_line(int fd_client, bool get_buffer)
{
	char buff[4096 + 1];
	std::string res;
	if (get_buffer)
	{
		res = gnl_line;
		gnl_line = "";
		return (res);
	}
	if (gnl_line.find("\r\n", 0) != std::string::npos)
	{
		res = gnl_line.substr(0, gnl_line.find("\r\n", 0));
		gnl_line.erase(0, gnl_line.find("\r\n", 0) + 2);
		return (res);
	}
	while (gnl_line.find("\r\n", 0) == std::string::npos)
	{
		int ret = (int)read(fd_client, buff, 4096);
		if (ret == 0)
			return ("");
		else if (ret == -1)
			throw std::runtime_error("Cannot read");
		buff[ret] = '\0';
		gnl_line += buff;
	}
	if (gnl_line.find("\r\n", 0) == std::string::npos)
	{
		res = gnl_line;
		gnl_line = "";
		return (res);
	}
	res = gnl_line.substr(0, gnl_line.find("\r\n", 0));
	gnl_line.erase(0, gnl_line.find("\r\n") + 2);
	return (res);
}

int Request::read_request(int fd_client)
{
	//READ HEADER
	if (!_req.size())
	{
		std::string	inter = "";
		do
		{
			try{
				inter = get_next_line(fd_client);
			}	catch (std::runtime_error *e)
			{
				continue;
			}
			_req += inter + "\r\n";
			std::vector<std::string> splt = split(inter, ' ');
			if (splt.size() && splt[0] == "Content-Length:")
			{
				if (splt.size() != 2)
					return (400);
				this->_req_content_length = to_int(splt[1]);
			}
			else if (splt.size() == 2 && splt[0] == "Transfer-Encoding:" && splt[1] == "chunked")
				this->_is_chunked_req = true;
		}while (inter != "");
		fcntl(fd_client, F_SETFL, O_NONBLOCK);
	}
	//READ BODY : Content-Length
	//fcntl(fd_client, F_SETFL, O_NONBLOCK);
	std::vector<char> buff(4096);
	if (!this->_is_chunked_req)
	{
		std::string inter =  get_next_line(fd_client, true);
		nb_rd += inter.size();
		_req += inter;
	}
	while (nb_rd < this->_req_content_length)
	{
		ssize_t	nb_bytes = read(fd_client, &buff[0], 4096);
		if (nb_bytes == -1)
			return (-1);
		nb_rd += nb_bytes;
		_req += std::string(&buff[0], nb_bytes);
		//std::cout << "Read :" <<  nb_rd << "/" << this->_req_content_length << std::endl;
	}
	//READ BODY : Transfer-Encoding chunked
	size_t	nb_to_rd = 0;
	if (this->_is_chunked_req)
	{
		do {
			try{
				std::string res = get_next_line(fd_client);
				if (res == "")
					break ;
				nb_to_rd = to_int(res, 16);
				_req += get_next_line(fd_client) + "\n";
			}catch (std::runtime_error& e){	
				return (-1);
			}
		}while (nb_to_rd);
	}
	_finish_read = true;
	return (200);
}

void Request::parse_variables()
{
	std::string line_variables;

	if (_req_meth == "GET")
	{
		if (_req_path.find('?') == std::string::npos)
		{
			_req_path = percent_decode(_req_path);
			return;
		}
		line_variables = std::string(_req_path.begin() + (int)_req_path.find('?') + 1, _req_path.end());
		_req_path = percent_decode(_req_path.substr(0, _req_path.size() - line_variables.size() - 1));
	}
	else if (_req_meth == "POST")
	{
		_req_path = percent_decode(_req_path);
		std::vector<std::string> req_splt = split(_req, '\n');
		if (req_splt[req_splt.size() - 2] == "\r")
			line_variables = req_splt[req_splt.size() - 1];
	}
	std::vector<std::string> variables;
	variables = split(line_variables, '&');
	for (std::vector<std::string>::iterator it = variables.begin(); it != variables.end(); it++)
	{
		std::vector<std::string> vals = split(*it, '=');
		_req_var.insert(std::pair<std::string, std::string>(vals[0], percent_decode(vals[1])));
	}
}

int Request::parse_request()
{
	std::vector<std::string> splt;
	std::istringstream is(_req);
	std::string line;

	if (_req == "")
		return (400);
	// Parse First LINE : GET / HTTP/1.1
	getline(is, line); // read first line
	splt = split(line, ' ');
	try
	{
		_req_meth = splt[0];
		_req_path = splt[1];
		_req_http_type = splt[2];
	}
	catch (const std::exception &e)
	{
		return (400);
	}
	_req_path = percent_decode(_req_path);
	//IS SUPPORT HTTP REQUEST ?
	if (!(_req_meth == "GET" || _req_meth == "POST" || _req_meth == "DELETE" || _req_meth == "PUT" \
|| _req_meth == "HEAD") || _req_http_type != "HTTP/1.1\r")
		return (403);
	parse_variables();
	while (getline(is, line))
	{
		splt = split(line, ' ');
		if (!splt.size()) // LINE BLANK
			break;
		if (splt[0] == "Accept:")
		{
			try
			{	_req_accept_content = splt[1];	}
			catch (const std::exception &e)
			{	return (400);	}
		}
		if (splt[0] == "Content-Type:" && splt.size() == 2)
			_req_content_type = splt[1];
		if (splt.size() > 1 && splt[0] == "Content-Type:" && splt[1] == "multipart/form-data;" && _req_meth == "POST")
		{
			if (splt.size() != 3)
				return (400);
			_is_upload_req = true;
			_req_upload_token = splt[2].substr(10, splt[2].size() - 10);
			while (_req_upload_token.find('-') != std::string::npos)
				_req_upload_token.erase(_req_upload_token.find('-'), 1);
		}
		if (splt[0] == "Transfer-Encoding:" && splt.size() == 2 && splt[1] == "chunked\r")
			_is_chunked_req = true;
	}
	return (200);
}
