#include <iostream>
#include "src/server.h"
#include "src/idGenerator.h"
// #include "httpProto.h"

const int PORT = 9443;
const int MAX_CONNECTIONS = 5;

SSLcontext context = {"prueba/cert.pem", "prueba/key.pem"};

HttpServer server(PORT, context);


std::string home(Args &args)
{
    if (!args.vars.empty())
    {
        std::string admin = args.vars[0];
        std::string id = args.vars[1];
        if (admin == "1234")
            return Redirect(args.ssl, "/dashboard");

        return server.render("templates/index.html", {{"id", id}, {"nombre", admin}});
    }
    else
    {
        // Ruta sin variables
        if (args.session["logged"] == "true" && args.session.id != "")
            return "Bienvenido al dashboard: " + args.session.id;
        return Redirect(args.ssl, "/login");
        ;
    }
}

std::string login(Args &args)
{
    if (args.request.method == GET)
    {
        if (args.session["logged"] == "true" && args.session.id != "")
            return Redirect(args.socket, "/dashboard");
        return server.render("templates/login.html");
    }
    else if (args.request.method == POST)
    {
        if (args.request.content["fpass"] == "123" && args.request.content["fname"] == "manu")
        {
            Session s1 = Session(idGenerator::generateIDstr(), "logged", "true");
            server.sessions.push_back(s1);
            return Redirect(args.socket, std::string("/dashboard"), {"SessionID", findCookie(server)});
        }
    }
    return "error";
}


int main(int argc, char **argv)
{
    // Ruta sin variables
    server.addRoute("/dashboard", home, {GET});

    server.addRoute("/login", login, {GET, POST});
    server.addRoute("/dashboard/<admin>/<id>",
                    home, {GET},
                    std::vector<std::string>(),
                    std::string());

    server.addFilesHandler("/files/", "./files/");

    server.setup();
    server.startListener(server.serverSocket);
}
