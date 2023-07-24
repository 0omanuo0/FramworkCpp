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

public:
    HttpServer *server;
    Templating(){};

    std::string Render(const std::string &route, const std::map<std::string, std::string> &data);
};

#endif // TEMPLATING_H
