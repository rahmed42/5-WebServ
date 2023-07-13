/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: me <me@student.42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/21 13:38:25 by abittel           #+#    #+#             */
/*   Updated: 2022/09/09 21:52:02 by me               ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

void init_content_files()
{
	std::ifstream f;
	f.open("conf/mime.type");

	for (std::string line = ""; getline(f, line);)
	{
		std::vector<std::string> splt = split(line, ' ');
		for (unsigned int i = 1; i < splt.size(); i++)
			if (splt[i] != "")
				content_file.insert(std::pair<std::string, std::string>(splt[i], splt[0]));
	}
	return;
}

void init_error_pages()
{
	error_pages[204] = "No Content";
	error_pages[400] = "Bad Request";
	error_pages[403] = "Forbidden";
	error_pages[404] = "Not Found";
	error_pages[405] = "Not Allowed";
	error_pages[413] = "Request Entity Too Large";
	error_pages[502] = "Bad Gateway";
	error_pages[504] = "Gatway Timeout";
}

std::vector<std::string> split(std::string const &str, char c)
{
	std::vector<std::string> res;
	std::istringstream is(str);
	std::string inter;

	while (getline(is, inter, c))
		res.push_back(inter);
	return (res);
}

std::string read_file2(std::string path)
{
	std::ifstream f;
	std::string res;

	f.open(path.c_str(), std::ifstream::out);
	if (!f.is_open())
		throw std::runtime_error("CANNOT OPEN FILE");
	for (std::string line = ""; getline(f, line);)
	{
		res += line + "\n";
	}
	return (res);
}

std::string read_file(std::string path)
{
	std::ifstream f;
	std::stringstream buffer;

	f.open(path.c_str(), std::ifstream::out);
	if (!f.is_open())
		throw std::runtime_error("CANNOT OPEN FILE");
	buffer << f.rdbuf();
	return (buffer.str());
}

int type_path(std::string path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return (TYPE_DIRECTORY);
		else if (s.st_mode & S_IFREG)
			return (TYPE_FILE);
		else
			return (TYPE_UNKNOWN);
	}
	else
	{
		throw std::runtime_error("File not found");
	}
}

std::string get_extension_file(std::string path)
{
	if (path.find_last_of(".") != std::string::npos)
	{
		std::string res = path.substr(path.find_last_of(".") + 1);
		return res;
	}
	return ("");
}

std::string percent_decode(std::string str)
{
	std::string res;
	for (unsigned int i = 0; i < str.size(); i++)
	{
		if (str[i] == '+')
			res += ' ';
		else if (str[i] == '%')
		{
			res += static_cast<char>(to_int(str.substr(i + 1, 2), 16));
			i += 2;
		}
		else
			res += str[i];
	}
	return (res);
}

std::string percent_encode(std::string str)
{
	std::map<char, std::string> chr;
	chr[' '] =  "+"; chr['!'] =  "%21"; chr['#'] =  "%23";
	chr['$'] =  "%24"; chr['%'] =  "%25"; chr['&'] =  "%26";
	chr['\''] =  "%27"; chr['('] =  "%28"; chr[')'] =  "%29";
	chr['*'] =  "%2A"; chr['+'] =  "%2B"; chr[','] =  "%2C";
	chr[':'] =  "%3A"; chr[';'] =  "%3B"; chr['='] =  "%3D";
	chr['?'] =  "%3F"; chr['@'] =  "%40"; chr['['] =  "%5B";
	chr[']'] =  "%5D";
	std::string res;
	for (unsigned int i = 0; i < str.size(); i++)
	{
		if (chr.count(str[i]))
			res += chr.find(str[i])->second;
		else
			res += str[i];
	}
	return (res);
}

std::string	convert_uint_to_hex(unsigned int n)
{
	std::string res;
	std::string hex = "0123456789ABCDEF";
	if (n == 0)
		return ("0");
	while (n > 0)
	{
		res = hex[n % 16] + res;
		n /= 16;
	}
	return (res);
}

std::string	to_string(int n)
{
	std::string s;
	std::stringstream out;
	out << n;
	s = out.str();
	return (s);
}

int	to_int(std::string str, int base)
{
	long int res = std::strtol(str.c_str(), NULL, base);
	return static_cast<int>(res);
}