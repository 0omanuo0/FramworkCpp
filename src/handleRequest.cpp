#include "server.h"
#include "url_encoding.h"

std::string __find_cookie(std::vector<Session> &sessions, std::string id)
{
    for (auto &session : sessions)
        if (session.id == id)
            return session.id;
    return std::string();
}

bool __match_path_with_route(const std::string &path, const std::string &routePath, std::vector<std::string> &routeVars)
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

bool __route_contains_variables(const std::string &routePath)
{
    return (routePath.find('<') != std::string::npos && routePath.find('>') != std::string::npos);
}

std::string __recv(SSL *ssl)
{
    char buffer[BUFFER_SIZE] = {0};
    memset(buffer, 0, sizeof(buffer));
    if (SSL_read(ssl, buffer, sizeof(buffer) - 1) < 0)
    {
        std::cerr << "Error al leer la petición HTTPS" << std::endl;
        return std::string();
    }
    return std::string(buffer);
}

int HttpServer::__create_response(SSL *ssl, const std::string &response, Session &session)
{
    // respuestas https
    std::string response_with_header;
    httpProtoResponse response_server;
    if (response.empty()){
        std::string respNOTFOUND = "<h1>NOT FOUND</h1>";
        response_server.length = respNOTFOUND.length();
        __send_response(ssl, response_server.defaultNotFound() + respNOTFOUND);
    }
    else
    {
        if (response.substr(0, REDIRECT.length()) == REDIRECT) ////////////////esto no se puede quedar asi
        {
            std::string cookie[] = {default_session_name, __find_cookie(sessions, session.id)};
            if (cookie[1].empty())
                response_with_header = response_server.defaultRedirect(response.substr(REDIRECT.length()));
            else
            {
                response_with_header = response_server.defaultRedirect_cookie(response.substr(REDIRECT.length()), cookie);
                session.create = false;
            }
        }
        else
        {
            response_server.length = response.length();
            std::string cookie[] = {default_session_name, __find_cookie(sessions, session.id)};
            if (cookie[1].empty())
                response_with_header += response_server.defaultOK();
            else
            {
                response_with_header += response_server.defaultOK_cookie(cookie);
                session.create = false;
            }
            response_with_header += response;
        }
    }
    __send_response(ssl, response_with_header);
    return 0;
}

int HttpServer::__send_response(SSL *ssl, const std::string &response)
{
    const char *httpResponse = response.c_str();
    if (SSL_write(ssl, httpResponse, strlen(httpResponse)) < 0)
    {
        std::cerr << "Error al enviar la respuesta HTTPS" << std::endl;
        return -1;
    }
    return 0;
}

int __wait_socket(int socket, SSL *ssl)
{

    if (ssl != NULL)
    {
        // Cerrar la conexión SSL y el socket
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(socket);
        // SSL_clear(ssl);
        socket = -1; // Establecer el socket en un valor no válido
        return 0;
    }

    char buffer[BUFFER_SIZE];
    int recvResult = 0;
    while ((recvResult = recv(socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        // Esperar a que acabe el envío
        continue;
    }

    if (recvResult < 0)
    {
        std::cerr << "Error al recibir datos del socket" << std::endl;
        return -1;
    }

    // Manejo del socket
    if (socket >= 0)
    {
        shutdown(socket, SHUT_WR);
        close(socket);
        socket = -1; // Establecer el socket en un valor no válido
    }

    return 0;
}

int HttpServer::__handle_request(int socket, SSL *ssl)
{
    std::string request(decodeURIComponent(__recv(ssl)));

    httpMethods http_method(request);
    std::cout << http_method.route << std::endl;

    int session_index = __find_match_session(http_method.params.cookies["SessionID"]);
    Session session = __get_session(session_index);

    // Buscar la ruta correspondiente en el std::vector de rutas
    std::string response;
    std::vector<std::string> routeVars;
    for (const auto &route : routes)
    {
        if (__route_contains_variables(route.path))
        {
            if (__match_path_with_route(http_method.route, route.path, routeVars))
            {
                Args arg = Args(routeVars, socket, ssl, http_method, session);
                response = route.handler(arg);
                if (!session.isEmpty() && session_index == -1)
                    setNewSession(session);
                else if (!session.isEmpty() && session_index != -1)
                    sessions[session_index] = session;
                break;
            }
        }
        else if (route.path == http_method.route)
        {
            Args arg = Args(routeVars, socket, ssl, http_method, session);
            response = route.handler(arg);
            if (!session.isEmpty() && session_index == -1)
                setNewSession(session);
            else if (!session.isEmpty() && session_index != -1)
                sessions[session_index] = session;
            break;
        }
    }

    __create_response(ssl, response, session);
    __wait_socket(socket, ssl);

    return 0;
}
