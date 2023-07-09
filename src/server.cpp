#include "server.h"
#include <chrono>

void HttpServer::__startListener(int port){
    std::vector<std::thread> threads;
    while (true)
    {
        // Aceptar una conexión entrante
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressSize);
        if (clientSocket < 0)
        {
            std::cerr << "Error al aceptar la conexión entrante" << std::endl;
            continue;
        }

        SSL *ssl = NULL;
        // Crear un hilo para manejar la comunicación con el cliente
        std::thread thread(&HttpServer::handleRequest, this, std::move(clientSocket), std::move(ssl));
        threads.push_back(std::move(thread));

        // Eliminar hilos terminados
        threads.erase(std::remove_if(threads.begin(), threads.end(), [](const std::thread &t)
                                     { return !t.joinable(); }),
                      threads.end());
    }
}

void HttpServer::__startListenerSSL(int port){
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

        // Crear el objeto SSL
        SSL *ssl = SSL_new(ssl_ctx);
        // Asociar el socket con el objeto SSL
        SSL_set_fd(ssl, clientSocket);
        // Establecer la conexión SSL
        if (SSL_accept(ssl) <= 0) {
            std::cerr << "Error al establecer la conexión SSL." << std::endl;
            SSL_free(ssl);
            close(clientSocket);
            continue;
        }

        std::thread thread;
        // Crear un hilo para manejar la comunicación con el cliente
        thread = std::thread(&HttpServer::handleRequest, this, std::move(clientSocket), std::move(ssl));
        threads.push_back(std::move(thread));

        // Eliminar hilos terminados
        threads.erase(std::remove_if(threads.begin(), threads.end(), [](const std::thread &t)
                                     { return !t.joinable(); }),
                      threads.end());
    }
}


int HttpServer::setup()
{
    if(HTTPS){
        // Inicializar OpenSSL
        SSL_library_init();
        ssl_ctx = SSL_CTX_new(TLS_server_method());

        // Cargar certificado y clave privada
        if (SSL_CTX_use_certificate_file(ssl_ctx, context.certificate.c_str(), SSL_FILETYPE_PEM) <= 0) {
            std::cerr << "Error al cargar el certificado." << std::endl;
            return 1;
        }
        if (SSL_CTX_use_PrivateKey_file(ssl_ctx, context.private_key.c_str(), SSL_FILETYPE_PEM) <= 0) {
            std::cerr << "Error al cargar la clave privada." << std::endl;
            return 1;
        }
    }


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
    if(HTTPS)
        __startListenerSSL(port);
    else
        __startListener(port);
}

Session HttpServer::findMatchSession(std::string id)
{
    for (auto &i : sessions)
        if (i.id == id)
            return i;
    return Session("");
}

void HttpServer::createSession()
{
}

#pragma region addRoutes
void HttpServer::addRoute(const std::string &path,
                          const std::function<std::string(Args &)> handler,
                          std::vector<std::string> methods)
{
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
    routes_folder.push_back({path, folder_path});
}

void HttpServer::addrouteFile(const std::string &path, std::string type)
{
    type = content_type.find(type)->second;
    routes_files.push_back({path, type});
}

#pragma endregion