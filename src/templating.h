#ifndef TEMPLATING_H
#define TEMPLATING_H

//#include "server.h"

#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <filesystem>
#include <map>
#include <regex>
#include <string>

#include "idGenerator.h"
#include "tinyexpr.h"
#include "server.h"
//#include "server.h"

class HttpServer;

class Templating
{
private:
    std::string __find_expressions(std::string line, const std::map<std::string, std::string> &data);

    std::string __find_statements(std::string line, std::ifstream &file, const std::map<std::string, std::string> &data);
    std::string __render_statements(std::string find, const std::map<std::string, std::string> &data);
    std::string __render_block(const std::string &path, const std::map<std::string, std::string> &data);
    std::string __render_for(std::string find, std::ifstream &file, const std::map<std::string, std::string> &data);
    std::string __render_if(std::string find, std::ifstream &file, const std::map<std::string, std::string> &data);
        
    const std::regex if_pattern = std::regex(R"(\bif\s+([^{}]+)\s*)");
    const std::regex else_pattern = std::regex(R"(\{\%\s+else\s+\%\})");
    const std::regex endif_pattern = std::regex(R"(\{\%\s+endif\s+\%\})");
    
    const std::regex for_pattern = std::regex(R"(\bfor\s+([^{}]+)\s+in\s+([^{ }]+)\s*)");
    const std::regex endfor_pattern = std::regex(R"(\{\%\s+endfor\s+\%\})");

    const std::regex block_pattern = std::regex(R"(\{%\s+block\s+content\s+%\})");
    const std::regex endblock_pattern = std::regex(R"(\{%\s+endblock\s+content\s+%\})");

    const std::regex patron_include = std::regex(R"(\binclude\s+"([^"]*)\s*)");

public:
    HttpServer *server;
    Templating(){};

    std::string Render(const std::string &route, const std::map<std::string, std::string> &data);
};

#endif // TEMPLATING_H
