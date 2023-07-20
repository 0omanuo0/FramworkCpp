#include <fstream>
#include "server.h"
#include "idGenerator.h"
#include "tinyexpr.h"

namespace fs = std::filesystem;
void ListFilesAndFolders(const fs::path &directory, int level = 0)
{
    for (const auto &entry : fs::directory_iterator(directory))
    {
        const auto filename = entry.path().filename().string();
        const auto indentation = std::string(level * 4, ' ');

        if (fs::is_directory(entry.status()))
        {
            std::cout << indentation << "[Folder] " << filename << std::endl;
            ListFilesAndFolders(entry.path(), level + 1);
        }
        else if (fs::is_regular_file(entry.status()))
        {
            std::cout << indentation << "[File] " << filename << std::endl;
        }
    }
}

double __evaluate_expression(std::string expression, const std::map<std::string, std::string> &data)
{
    // Crear un conjunto de te_variable a partir del mapa
    std::set<te_variable> variablesSet;

    for (const auto &pair : data)
    {
        te_variable var;
        var.m_name = pair.first;

        try
        {
            var.m_value = std::stod(pair.second); // Convertir el valor a double
            var.m_type = TE_DEFAULT;
            variablesSet.insert(var);
        }
        catch (const std::exception &e)
        {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }
    // Store variable names and pointers.
    te_parser tep;
    tep.set_variables_and_functions(variablesSet);

    if (tep.compile(expression))
        return tep.evaluate();
    else
        return std::numeric_limits<double>::quiet_NaN();
}

std::string HttpServer::__find_expressions(std::string line, const std::map<std::string, std::string> &data)
{

    std::string lineF;

    // Definimos una expresión regular para buscar patrones del tipo {{algo}}
    std::smatch matches;
    std::regex pattern(R"(\{\{\s*([^{}]+)\s*\}\})");

    // Creamos iteradores para buscar todas las coincidencias de la expresión regular en la línea de entrada
    auto it = std::sregex_iterator(line.begin(), line.end(), pattern);
    auto end = std::sregex_iterator();
    size_t currentPosition = 0;

    // Iteramos sobre todas las coincidencias encontradas
    for (; it != end; ++it)
    {
        // Obtenemos la coincidencia actual
        std::smatch match = *it;

        // Obtenemos la posición de la coincidencia en la línea de entrada
        size_t matchPosition = match.position();

        // Extraemos el texto antes de la coincidencia y lo agregamos al resultado
        std::string textBeforeMatch = line.substr(currentPosition, matchPosition - currentPosition);
        if (!textBeforeMatch.empty())
            lineF += textBeforeMatch;

        // Extraemos el contenido dentro de las llaves {{...}}
        std::string find = match[1].str();

        double result = __evaluate_expression(find, data);

        if (std::isnan(result))
        {
            // Definimos otra expresión regular para buscar patrones del tipo urlfor(algo)
            std::regex patron_urlfor(R"(\burlfor\(\s*([^\(\)]+)\s*\))");
            std::smatch match_f;

            // Comprobamos si el contenido dentro de las llaves tiene el formato urlfor(algo)
            if (std::regex_search(find, match_f, patron_urlfor))
            {
                // Extraemos el contenido entre los paréntesis de urlfor y lo agregamos al resultado
                std::string path = match_f[1].str().substr(1, match_f[1].str().length() - 2);
                urlfor(path);
                lineF += path;
            }
            else
            {
                // Si no es del formato urlfor(algo), verificamos si el contenido se encuentra en el mapa 'data'
                if (data.find(find) != data.end())
                    lineF += data.find(find)->second; // Agregamos el valor asociado a la clave 'find' al resultado
                else
                    lineF += find; // Si no se encuentra en el mapa 'data', agregamos el contenido original al resultado
            }
        }
        else{
            if(result == static_cast<int>(result))
                lineF += std::to_string(static_cast<int>(result));
        }

        // Actualizamos la posición actual para continuar buscando después de la coincidencia actual
        currentPosition = matchPosition + match.length();
    }

    // Agregamos el texto después de la última coincidencia al resultado final
    std::string textAfterLastMatch = line.substr(currentPosition);
    if (!textAfterLastMatch.empty())
        lineF += textAfterLastMatch;

    // Devolvemos la cadena resultante con todas las sustituciones y modificaciones realizadas
    return lineF;
}

std::string HttpServer::__render_block(const std::string &path, const std::map<std::string, std::string> &data){
    if (!std::filesystem::exists(path))
    {
        std::cout << "file does not exist" << std::endl;
        ListFilesAndFolders(".");
        return "";
    }

    std::ifstream file(path);
    std::string rendered;
    if (file.is_open())
    {
        std::string line;
        std::getline(file, line);

        std::regex block_pattern(R"(\{%\s+block\s+content\s+%\})");
        std::regex endblock_pattern(R"(\{%\s+endblock\s+content\s+%\})");
        while(!std::regex_search(line, block_pattern))
            std::getline(file, line);

        while (std::getline(file, line) && !std::regex_search(line, endblock_pattern))
        {
            line = __find_expressions(line, data);
            rendered += __find_statements(line, data);
        }
        file.close();
    }
    else
        return "";
    return rendered;
}

std::string HttpServer::__render_statements(std::string find, const std::map<std::string, std::string> &data)
{
    std::string lineF;
    // Definimos otra expresión regular para buscar patrones del tipo 'include "data" '
    std::regex patron_include(R"(\binclude\s+"([^"]*)\s*)");

    std::smatch match_f;

    // Comprobamos si el contenido dentro de las llaves tiene el formato 'include "data" '
    if (std::regex_search(find, match_f, patron_include))
    {
        // Extraemos el contenido entre comillas
        std::string path = match_f[1].str();
        lineF = __render_block(path, data);
        if (lineF.empty())
            lineF = "Error al cargar el archivo";
    }
    else
        return find; // Si no se encuentra en el mapa 'data', agregamos el contenido original al resultado
    return lineF;
}

std::string HttpServer::__find_statements(std::string line, const std::map<std::string, std::string> &data)
{
    std::string lineF;

    // Definimos una expresión regular para buscar patrones del tipo {{algo}}
    std::smatch matches;
    std::regex pattern(R"(\{\%\s*([^{}]+)\s*\%\})");

    // Creamos iteradores para buscar todas las coincidencias de la expresión regular en la línea de entrada
    auto it = std::sregex_iterator(line.begin(), line.end(), pattern);
    auto end = std::sregex_iterator();
    size_t currentPosition = 0;

    // Iteramos sobre todas las coincidencias encontradas
    for (; it != end; ++it)
    {
        // Obtenemos la coincidencia actual
        std::smatch match = *it;

        // Obtenemos la posición de la coincidencia en la línea de entrada
        size_t matchPosition = match.position();

        // Extraemos el texto antes de la coincidencia y lo agregamos al resultado
        std::string textBeforeMatch = line.substr(currentPosition, matchPosition - currentPosition);
        if (!textBeforeMatch.empty())
            lineF += textBeforeMatch;

        // Extraemos el contenido dentro de las llaves {% ... %}
        std::string find = match[1].str();

        lineF += __render_statements(find, data);

        // Actualizamos la posición actual para continuar buscando después de la coincidencia actual
        currentPosition = matchPosition + match.length();
    }
    // Agregamos el texto después de la última coincidencia al resultado final
    std::string textAfterLastMatch = line.substr(currentPosition);
    if (!textAfterLastMatch.empty())
        lineF += textAfterLastMatch;

    // Devolvemos la cadena resultante con todas las sustituciones y modificaciones realizadas
    return lineF;
}

// de momento en el redner solo va urlfor
std::string HttpServer::Render(const std::string &route, std::map<std::string, std::string> data)
{
    if (!std::filesystem::exists(route))
    {
        std::cout << "file does not exist" << std::endl;
        ListFilesAndFolders(".");
        return "";
    }

    std::ifstream file(route);
    std::string rendered;
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            line = __find_expressions(line, data);
            rendered += __find_statements(line, data);
        }
        file.close();
    }
    else
        return "";
    return rendered;
}

void HttpServer::urlfor(const std::string &endpoint)
{
    std::size_t index = endpoint.find_last_of(".");
    std::string extension = "txt";
    if (std::string::npos != index)
        extension = endpoint.substr(index + 1);
    addRouteFile(endpoint, extension);
}
