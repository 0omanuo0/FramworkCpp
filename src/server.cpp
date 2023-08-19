#include "server.h"
#include "templating.h"

HttpServer::HttpServer()
{
    template_render->server = this;
}

HttpServer::HttpServer(int port_server, int max_connections)
    : port(port_server), MAX_CONNECTIONS(max_connections)
{
    template_render->server = this;
}
HttpServer::HttpServer(int port_server, const std::string SSLcontext_server[], int max_connections)
    : port(port_server), MAX_CONNECTIONS(max_connections)
{
    context.certificate = SSLcontext_server[0];
    context.private_key = SSLcontext_server[1];
    HTTPS = true;

    template_render = new Templating();
    template_render->server = this;
}

std::string HttpServer::Render(const std::string &route, std::map<std::string, std::string> data)
{
    return template_render->Render(route, data);
};

void HttpServer::__startListenerSSL()
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

        // Crear el objeto SSL
        SSL *ssl = SSL_new(ssl_ctx);
        // Asociar el socket con el objeto SSL
        SSL_set_fd(ssl, clientSocket);
        // Establecer la conexión SSL
        if (SSL_accept(ssl) <= 0)
        {
            std::cerr << "Error al establecer la conexión SSL." << std::endl;
            SSL_free(ssl);
            close(clientSocket);
            continue;
        }

        // Crear un hilo para manejar la comunicación con el cliente
        std::thread thread(&HttpServer::__handle_request, this, std::move(clientSocket), std::move(ssl));
        threads.push_back(std::move(thread));
        // Eliminar hilos terminados
        threads.erase(std::remove_if(threads.begin(), threads.end(), [](const std::thread &t)
                                     { return !t.joinable(); }),
                      threads.end());
    }
}

void HttpServer::startListener()
{
    if (HTTPS)
        __startListenerSSL();
    else
        return;
    // __startListener(port);
}

int HttpServer::setup()
{
    if (HTTPS)
    {
        // Inicializar OpenSSL
        SSL_library_init();
        ssl_ctx = SSL_CTX_new(TLS_server_method());

        // Cargar certificado y clave privada
        if (SSL_CTX_use_certificate_file(ssl_ctx, context.certificate.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            cerr << "Error al cargar el certificado." << endl;
            return 1;
        }
        if (SSL_CTX_use_PrivateKey_file(ssl_ctx, context.private_key.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            cerr << "Error al cargar la clave privada." << endl;
            return 1;
        }
    }

    // Crear el socket del servidor
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        cerr << "Error al crear el socket" << endl;
        return 1;
    }

    // Configurar la estructura de la dirección del servidor
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    // Enlazar el socket a la dirección y el puerto
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        cerr << "Error al enlazar el socket" << endl;
        return 1;
    }

    // Escuchar las conexiones entrantes
    if (listen(serverSocket, MAX_CONNECTIONS) < 0)
    {
        cerr << "Error al escuchar las conexiones" << endl;
        return 1;
    }

    cout << "Servidor HTTP en ejecución en el puerto " << port << endl;
    return 0;
}

void HttpServer::addRoute(const std::string &path,
                          std::function<std::string(Args &)> handler,
                          std::vector<std::string> methods)
{
    routes.push_back({path, methods, [handler](Args &args)
                      {
                          return handler(args); // Call the original handler with the query and method
                      }});
}
void HttpServer::addRouteFile(const std::string &endpoint, const std::string &extension)
{
    routesFile.push_back({endpoint, content_type.find(extension)->second});
}
void HttpServer::urlfor(const std::string &endpoint)
{
    std::size_t index = endpoint.find_last_of(".");
    std::string extension = "txt";
    if (std::string::npos != index)
        extension = endpoint.substr(index + 1);
    addRouteFile(endpoint, extension);
}
