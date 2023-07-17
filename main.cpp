#include <iostream>
#include "src/server.h"
// #include "httpProto.h"

const int PORT = 8443;
const int MAX_CONNECTIONS = 5;

std::string HTTPScontext[] = {"prueba/cert.pem", "prueba/key.pem"};

HttpServer server(PORT, HTTPScontext);


std::string home(Args &args)
{
    if (!args.vars.empty())
    {
        std::string admin = args.vars[0];
        std::string id = args.vars[1];
        if (admin == "1234")
            return Redirect(args.ssl, "/dashboard");

        return server.Render("templates/index.html", {{"id", id}, {"nombre", admin}});
    }
    else
    {
        // Ruta sin variables
        if (args.session["logged"] == "true" && args.session.id != "")
            return "Bienvenido al dashboard: " + args.session.id;
            
        return Redirect(args.ssl, "/login");
    }
}

std::string login(Args &args)
{
    if (args.request.method == GET)
    {
        if (args.session["logged"] == "true" && args.session.id != "")
            return Redirect(args.ssl, "/dashboard");
        return server.Render("templates/login.html");
    }
    else if (args.request.method == POST)
    {
        if (args.request.content["fpass"] == "123" && args.request.content["fname"] == "manu")
        {
            args.session.createSession();
            args.session["logged"] = "true";
            return Redirect(args.ssl, std::string("/dashboard"));
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
                    home, {GET});

    server.setup();
    server.startListener();
}
