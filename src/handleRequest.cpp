#include "server.h"
#include <fstream>
#include "url_encoding.h"
#include "httpProto.h"

std::vector<std::string> sendFile(std::string path_to_file)
{
    std::vector<std::string> data_to_send = std::vector<std::string>();
    httpProtoResponse response_serv = httpProtoResponse();

    std::ifstream file(path_to_file, std::ios::binary);
    if (!file.is_open() && !file.good())
    {
        std::cerr << "Error al abrir el archivo: " << path_to_file << std::endl;
        return data_to_send;
    }

    // Obtener el tamaño del archivo
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Leer el contenido del archivo en un búfer
    std::vector<char> buffer_f(fileSize);
    if (!file.read(buffer_f.data(), fileSize))
    {
        std::cerr << "Error al leer el archivo: " << path_to_file << std::endl;
        data_to_send.push_back(response_serv.defaultNotFound());
        return data_to_send;
    }

    // Construir la respuesta HTTP con el encabezado adecuado

    response_serv.appendParam("Content-Type", "application/zip");
    response_serv.appendParam("Content-Disposition", "attachment; filename=\"" + path_to_file.substr(path_to_file.find_last_of("/") + 1) + "\"");
    response_serv.length = fileSize;

    data_to_send.push_back(response_serv.defaultOK());
    data_to_send.push_back(std::string(buffer_f.begin(), buffer_f.end()));

    return data_to_send;
}

bool routeContainsVariables(const std::string &routePath)
{
    return (routePath.find('<') != std::string::npos && routePath.find('>') != std::string::npos);
}

bool matchPathWithRoute(const std::string &path, const std::string &routePath, std::vector<std::string> &routeVars)
{
    std::istringstream pathStream(path);
    std::istringstream routePathStream(routePath);

    std::string pathSegment;
    std::string routeSegment;

    while (std::getline(pathStream, pathSegment, '/'))
    {
        if (!std::getline(routePathStream, routeSegment, '/'))
        {
            // La ruta tiene menos segmentos que el path
            return false;
        }

        if (routeSegment != pathSegment)
        {
            if (routeSegment.front() != '<' || routeSegment.back() != '>')
            {
                // Los segmentos no coinciden y no es una variable
                return false;
            }

            // Es una variable, la guardamos
            routeVars.push_back(pathSegment);
        }
    }

    return !std::getline(routePathStream, routeSegment, '/');
}

int waitSocket(int socket, char buffer[], int buffer_len)
{
    // manejo del socket
    shutdown(socket, SHUT_WR);

    while (recv(socket, buffer, buffer_len, 0) > 0)
        continue; // esperar a que acabe el envio

    close(socket);

    return 0;
}

std::string findCookie(HttpServer &server){
    for (auto &session : server.sessions)
    {
        if(session.create){
            session.create = false;
            return session.sessionUser.id;
        }
    }
    return std::string();
    
}

int HttpServer::handleRequest(int socket)
{
    memset(buffer, 0, sizeof(buffer));
    if (read(socket, buffer, sizeof(buffer)) < 0)
    {
        std::cerr << "Error al leer la petición HTTP" << std::endl;
        return -1;
    }

    std::string request(buffer);
    request = decodeURIComponent(request);

    httpMethods http_method;

    http_method.loadParams(request);

    std::cout << http_method.route << std::endl;

    Session session = findMatchSession(http_method.params_get.cookies["SessionID"]);
    // for (const auto& pair : http_method.params_get.cookies) {
    // std::cout << pair.first << ": " << pair.second << std::endl;
    // }

    // Buscar la ruta correspondiente en el vector de rutas
    std::string response;
    for (const auto &route : routes)
    {
        if (routeContainsVariables(route.path))
        {
            std::vector<std::string> routeVars;

            if (matchPathWithRoute(http_method.route, route.path, routeVars))
            {
                Args arg = Args(routeVars, http_method, session);
                arg.socket = socket;
                response = route.handler(arg);
                if (response == REDIRECT)
                    return waitSocket(socket, buffer, sizeof(buffer));
                break;
            }
        }
        else if (route.path == http_method.route)
        {
            std::vector<std::string> routeVars;
            Args arg = Args(routeVars, http_method, session);
            arg.socket = socket;
            response = route.handler(arg);
            if (response == REDIRECT)
                return waitSocket(socket, buffer, sizeof(buffer));
            break;
        }
    }


    // encontrar ruta de archivos (si existiera)
    std::string path_to_file;

    if (response.empty())
    {
        for (const auto &route_file : routes_file)
        {
            std::string::size_type pos = http_method.route.find(route_file.path);
            if (pos != std::string::npos)
            {
                // Coincidencia encontrada, reemplazar la ruta URL con la ruta local
                std::string localPath = http_method.route;
                localPath.replace(pos, route_file.path.length(), route_file.folder_path);
                path_to_file = localPath;
                break;
            }
        }
    }

    // respuestas http
    std::string response_with_header;
    httpProtoResponse response_server = httpProtoResponse();

    if (response.empty() && path_to_file.empty())
        sendResponse(socket, response_server.defaultNotFound() + "<h1>NOT FOUND</h1>");
    else if (!path_to_file.empty())
        sendResponse(socket, sendFile(path_to_file));
    else
    {
        response_server.length = response.length();
        std::string cookie[] = {"SessionID", findCookie(*this)};
        if(cookie[1].empty())
            response_with_header += response_server.defaultOK();
        else
            response_with_header += response_server.defaultOK_cookie(cookie);
        response_with_header += response;
        sendResponse(socket, response_with_header);
    }
    waitSocket(socket, buffer, sizeof(buffer));

    return 0;
}
