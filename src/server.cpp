#include "server.h"
#include "jinjaTemplating/templating.h"

HttpServer::HttpServer()
{
    template_render->server = this;
}

struct in_addr hostnameToIp(const char *hostname)
{
    struct hostent *hostInfo = gethostbyname(hostname);
    struct in_addr addr;

    if (hostInfo == nullptr)
    {
        throw runtime_error("Error: Unable to resolve hostname.");
        // You might want to handle the error more gracefully
        exit(1);
    }

    addr.s_addr = *(reinterpret_cast<unsigned long *>(hostInfo->h_addr));
    return addr;
}

HttpServer::HttpServer(int port_server, char *hostname)
    : port(port_server)
{
    host = hostname;
    // Convierte la dirección IP en formato de cadena a representación binaria
    ip_host_struct = hostnameToIp(host);

    template_render = new Templating();
    template_render->server = this;
}
HttpServer::HttpServer(int port_server, const string SSLcontext_server[], char *hostname)
    : port(port_server)
{
    host = hostname;
    // Convierte la dirección IP en formato de cadena a representación binaria
    ip_host_struct = hostnameToIp(host);
    context.certificate = SSLcontext_server[0];
    context.private_key = SSLcontext_server[1];
    HTTPS = true;

    template_render = new Templating();
    template_render->server = this;
}

// string HttpServer::Render(const string &route, map<string, string> data)
// {
//     return template_render->Render(route, data);
// };

string HttpServer::Render(const string &route, nlohmann::json data)
{
    return template_render->Render(route, data);
};

string HttpServer::Render(const string &route, const string &data)
{
    return template_render->Render(route, data);
};

string HttpServer::RenderString(const string &route, nlohmann::json data)
{
    return template_render->RenderString(route, data);
};

void HttpServer::__startListenerSSL()
{
    vector<thread> threads;

    while (true)
    {
        // Aceptar una conexión entrante
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressSize);
        if (clientSocket == -1)
        {
            cerr << "Error al aceptar la conexión entrante" << endl;
            continue;
        }

        // Crear el objeto SSL
        SSL *ssl = SSL_new(ssl_ctx);
        // Asociar el socket con el objeto SSL
        SSL_set_fd(ssl, clientSocket);
        // Establecer la conexión SSL
        if (SSL_accept(ssl) <= 0)
        {
            cerr << "Error al establecer la conexión SSL." << endl;
            SSL_free(ssl);
            close(clientSocket);
            continue;
        }

        // Crear un hilo para manejar la comunicación con el cliente
        thread thread(&HttpServer::__handle_request, this, move(clientSocket), move(ssl));
        threads.push_back(move(thread));
        // Eliminar hilos terminados
        threads.erase(remove_if(threads.begin(), threads.end(), [](const std::thread &t)
                                { return !t.joinable(); }),
                      threads.end());
    }
}

void HttpServer::__startListener()
{
    vector<thread> threads;

    while (true)
    {
        // Aceptar una conexión entrante
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressSize);
        if (clientSocket == -1)
        {
            cerr << "Error al aceptar la conexión entrante" << endl;
            continue;
        }

        // Crear un hilo para manejar la comunicación con el cliente
        SSL *ssl = NULL;
        thread thread(&HttpServer::__handle_request, this, move(clientSocket), move(ssl));
        threads.push_back(move(thread));
        // Eliminar hilos terminados
        threads.erase(remove_if(threads.begin(), threads.end(), [](const std::thread &t)
                                { return !t.joinable(); }),
                      threads.end());
    }
}

void HttpServer::startListener()
{
    try
    {
        if (this->__setup() != 0)
            throw std::runtime_error("Error: setup failed");
        if (HTTPS)
            this->__startListenerSSL();
        else
            this->__startListener();
    }
    catch (const std::exception &e)
    {
        close(this->serverSocket);
        this->logger.error(e.what());
    }
}

void HttpServer::setEnv(const std::string &envPath)
{
    this->envPath = envPath;
}

int HttpServer::__loadEnv()
{
    // check if exists envPath
    if (!filesystem::exists(envPath))
    {
        // display that .env not exists so is using default values
        logger.warning("The .env file does not exist, using default values");
        return 1;
    }
    logger.log("Loading environment variables from .env file");

    std::ifstream file(envPath);
    if (!file.is_open())
        return 2;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream is_line(line);
        std::string key;
        if (line[0] == '#')
            continue;

        if (std::getline(is_line, key, '='))
        {
            std::string value;
            if (std::getline(is_line, value))
                this->data_[key] = value;
        }
    }
    return 0;
}

int HttpServer::__setup()
{
    if (this->__loadEnv() > 1)
        throw std::runtime_error("Error: loadEnv failed");
    // Configurar el servidor
    for (auto &data : data_)
    {
        if (data.first == "max_connections")
        {
            if (std::holds_alternative<int>(data.second))
                this->MAX_CONNECTIONS = std::get<int>(data.second);
            else if (std::holds_alternative<std::string>(data.second))
                this->MAX_CONNECTIONS = std::stoi(std::get<std::string>(data.second));
            else
                throw std::runtime_error("Error: max_connections must be an integer");
        }
        else if (data.first == "secret_key")
        {
            if (std::holds_alternative<std::string>(data.second))
                this->idGeneratorJWT = idGenerator(std::get<std::string>(data.second));
            else
                throw std::runtime_error("Error: secret_key must be a string");
        }
        else if (data.first == "default_session_name")
        {
            if (std::holds_alternative<std::string>(data.second))
                this->default_session_name = std::get<std::string>(data.second);
            else
                throw std::runtime_error("Error: default_session_name must be a string");
        }
        else if (data.first == "not_found")
        {
            if (std::holds_alternative<std::string>(data.second))
                this->__default_not_found = std::get<std::string>(data.second);
            else
                throw std::runtime_error("Error: not_found must be a string");
        }
        else if (data.first == "internal_server_error")
        {
            if (std::holds_alternative<std::string>(data.second))
                this->__default_internal_server_error = std::get<std::string>(data.second);
            else
                throw std::runtime_error("Error: internal_server_error must be a string");
        }
        else if (data.first == "unauthorized")
        {
            if (std::holds_alternative<std::string>(data.second))
                this->__default_unauthorized = std::get<std::string>(data.second);
            else
                throw std::runtime_error("Error: unauthorized must be a string");
        }
        else if (data.first == "bad_request")
        {
            if (std::holds_alternative<std::string>(data.second))
                this->__default_bad_request = std::get<std::string>(data.second);
            else
                throw std::runtime_error("Error: bad_request must be a string");
        }
    }

    if (HTTPS)
    {
        // Inicializar OpenSSL
        SSL_library_init();
        ssl_ctx = SSL_CTX_new(TLS_server_method());

        // Cargar certificado y clave privada
        if (SSL_CTX_use_certificate_file(ssl_ctx, context.certificate.c_str(), SSL_FILETYPE_PEM) <= 0 || !filesystem::exists(context.certificate))
        {
            cerr << "Error al cargar el certificado." << endl;
            return 1;
        }
        if (SSL_CTX_use_PrivateKey_file(ssl_ctx, context.private_key.c_str(), SSL_FILETYPE_PEM) <= 0 || !filesystem::exists(context.private_key))
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
    serverAddress.sin_addr.s_addr = INADDR_ANY; //////////////////////////////////////ARREGLAR
    serverAddress.sin_port = htons(port);

    // Enlazar el socket a la dirección y el puerto
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) != 0)
    {
        logger.error("Error while binding the socket");
        return 1;
    }

    // Escuchar las conexiones entrantes
    if (listen(serverSocket, MAX_CONNECTIONS) < 0)
    {
        logger.error("Error while listening for incoming connections");
        return 1;
    }
    if (HTTPS)
        cout << "Servidor https://" << host << ":" << port << " en ejecución " << endl;
    else
        cout << "Servidor http://" << host << ":" << port << " en ejecución " << endl;
    return 0;
}

std::variant<std::string, int> &HttpServer::operator[](const std::string &key)
{
    return data_[key];
}

void HttpServer::addRoute(const string &path, types::FunctionHandler handler, vector<string> methods)
{
    this->routes.push_back({path, methods, [handler](Request &args)
                            {
                                return handler(args); // Call the original handler with the query and method
                            }});
}
void HttpServer::addRouteFile(const string &endpoint, const string &extension)
{
    for (auto &route : routesFile)
        if (route.path == endpoint)
            return;
    try
    {
        this->routesFile.push_back({endpoint, content_type.find(extension)->second});
    }
    catch (const std::exception &e)
    {
        this->routesFile.push_back({endpoint, "application/force-download"});
    }
}
void HttpServer::urlfor(const string &endpoint)
{
    size_t index = endpoint.find_last_of(".");
    string extension = "txt";
    if (string::npos != index)
        extension = endpoint.substr(index + 1);
    addRouteFile(endpoint, extension);
}

void HttpServer::setNotFound(const types::defaultFunctionHandler &handler)
{
    this->__not_found_handler = handler;
}

void HttpServer::setUnauthorized(const types::defaultFunctionHandler &handler)
{
    this->__unauthorized_handler = handler;
}

void HttpServer::setInternalServerError(const types::defaultFunctionHandler &handler)
{
    this->__internal_server_error_handler = handler;
}
