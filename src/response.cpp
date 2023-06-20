#include "server.h"


int HttpServer::sendResponse(int socket, std::string response){
    // Enviar la respuesta HTTP al cliente
    const char *httpResponse = response.c_str();
    if (send(socket, httpResponse, strlen(httpResponse), 0) < 0)
    {
        std::cerr << "Error al enviar la respuesta HTTP" << std::endl;
        return -1;
    }

    //std::cout << "send to: " << socket << std::endl
    //            << response << std::endl;
    return 0;
}

int HttpServer::sendResponse(int socket, std::vector<std::string> response){
    // Enviar el encabezado y el contenido del archivo al cliente
    if (send(socket, response[0].c_str(), response[0].size(), 0) < 0)
    {
        std::cerr << "Error al enviar el encabezado HTTP" << std::endl;
        return -1;
    }

    if (send(socket, response[1].c_str(), response[1].size(), 0) < 0)
    {
        std::cerr << "Error al enviar el contenido del archivo" << std::endl;
        return -1;
    }

    //std::cout << "send to: " << socket << std::endl
    //            << response[0].c_str() << std::endl
    //            << response[1].size()  << std::endl;
    return 0;
}