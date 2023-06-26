#include <fstream>
#include "server.h"
#include "idGenerator.h"

std::string HttpServer::__render_line(std::string line, std::map<std::string, std::string> data)
{

    std::string lineF;

    std::smatch matches;
    std::regex pattern(R"(\{\{\s*([^{}]+)\s*\}\})");

    auto it = std::sregex_iterator(line.begin(), line.end(), pattern);
    auto end = std::sregex_iterator();
    size_t currentPosition = 0;

    for (; it != end; ++it)
    {
        std::smatch match = *it;
        size_t matchPosition = match.position();
        std::string textBeforeMatch = line.substr(currentPosition, matchPosition - currentPosition);

        if (!textBeforeMatch.empty())
            lineF += textBeforeMatch;

        std::regex patron(R"(\burlfor\(\s*([^\(\)]+)\s*\))");
        std::smatch urlfor_match;
        std::string find = match[1].str();

        if (std::regex_search(find, urlfor_match, patron))
        {
            std::string path = urlfor_match[1].str().substr(1, urlfor_match[1].str().length() - 2);
            urlfor(path);
            lineF += path;
        }
        else
        {
            if (data.find(find) != data.end())
                lineF += data.find(find)->second;
            else
                lineF += find;
        }

        currentPosition = matchPosition + match.length();
    }

    std::string textAfterLastMatch = line.substr(currentPosition);

    if (!textAfterLastMatch.empty())
        lineF += textAfterLastMatch;

    return lineF;  
}

// de momento en el redner solo va urlfor
std::string HttpServer::render(const std::string &route, std::map<std::string, std::string> data)
{
    std::ifstream file(route); // Reemplaza "archivo.html" con la ruta y el nombre de tu archivo HTML
    std::string rendered;
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            rendered += __render_line(line, data);
        }
        file.close();
    }
    else
    {
        return "";
    }
    return rendered;
}

void HttpServer::urlfor(const std::string &endpoint)
{
    std::size_t index = endpoint.find_last_of(".");
    std::string extension = "txt";
    if (std::string::npos != index)
        extension = endpoint.substr(index + 1);
    addrouteFile(endpoint, extension);
}
