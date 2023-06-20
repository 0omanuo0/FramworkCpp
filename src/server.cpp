#include "server.h"
#include <chrono>

int HttpServer::setup()
{

    // Crear el socket del servidor
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }

    // Configurar la estructura de la dirección del servidor
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    // Enlazar el socket a la dirección y el puerto
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        std::cerr << "Error al enlazar el socket" << std::endl;
        return 1;
    }

    // Escuchar las conexiones entrantes
    if (listen(serverSocket, MAX_CONNECTIONS) < 0)
    {
        std::cerr << "Error al escuchar las conexiones" << std::endl;
        return 1;
    }

    std::cout << "Servidor HTTP en ejecución en el puerto " << port << std::endl;
    return 0;
}

void HttpServer::startListener(int port)
{
    std::vector<std::thread> threads;
    while (true)
    {
        // Aceptar una conexión entrante
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressSize);
        if (clientSocket == -1)
        {
            std::cerr << "Error al aceptar la conexión entrante" << std::endl;
            continue;
        }

        // Crear un hilo para manejar la comunicación con el cliente
        std::thread thread(&HttpServer::handleRequest, this, clientSocket);
        threads.push_back(std::move(thread));

        // Eliminar hilos terminados
        threads.erase(std::remove_if(threads.begin(), threads.end(), [](const std::thread &t)
                                     { return !t.joinable(); }),
                      threads.end());
    }
}

Session HttpServer::findMatchSession(std::string id)
{
    for (auto &i : sessions)
        if (i.sessionUser.id == id)
            return i;
    return Session("");
}

void HttpServer::createSession()
{
}

#pragma region addRoutes
void HttpServer::addRoute(const std::string &path,
                          const std::function<std::string(Args &)> handler,
                          std::vector<std::string> methods){
    routes.push_back({path, methods, [handler](Args &args)
                      {
                          return handler(args); // Call the original handler with the query and method
                      }});
}

void HttpServer::addRoute(const std::string &path,
                          std::function<std::string(Args &)> handler,
                          std::vector<std::string> methods,
                          std::vector<std::string> vars)
{
    routes.push_back({path, methods, [vars, handler](Args &args)
                      {
                          return handler(args); // Call the original handler with the query and method
                      }});
}
void HttpServer::addRoute(const std::string &path,
                          std::function<std::string(Args &)> handler,
                          std::vector<std::string> methods,
                          std::vector<std::string> vars,
                          std::string query)
{
    routes.push_back({path, methods, [vars, query, handler](Args &args)
                      {
                          return handler(args); // Call the original handler with the query and method
                      }});
}

void HttpServer::addFilesHandler(const std::string &path, const std::string &folder_path)
{
    routes_file.push_back({path, folder_path});
}

#pragma endregion