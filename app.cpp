#include <iostream>
#include "src/server.h"
#include "users.hpp"
#include "src/usersdb.h"
#include "src/idGenerator.h"


using namespace std;

const int PORT = 8444;


string HTTPScontext[] = {"secrets/cert.pem", "secrets/key.pem"};

HttpServer server(PORT, HTTPScontext, "ubuntu-manu.local");
UsersDB DATABASE("secrets/users.db");

types::HttpResponse home(Request &req)
{
    if (req.method == POST)
    {
        if (req.session["logged"] == "true")
        {
            DATABASE["POSTS"].insertRow({{"autor", req.session["user"]}, {"contenido", req.content["post"]}});
            vector<post> POSTS;
            try
            {
                for (int i = 1; i <= DATABASE["POSTS"].countRows(); i++)
                {
                    unordered_map post1 = DATABASE["POSTS"].getByID(to_string(i));
                    post p = {{post1["autor"]}, post1["contenido"]};
                    POSTS.push_back(p);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }

            return server.Render("templates/home_logged.html", {{"user", req.session["user"]}, {"posts", posts_to_string(POSTS)}});
        }
        else
            return server.Render("templates/home.html");
        return server.Redirect("/");
    }
    else if (req.method == GET)
    {
        if (req.session["logged"] == "true")
        {
            vector<post> POSTS;
            try
            {
                for (size_t i = 1; i <= DATABASE["POSTS"].countRows(); i++)
                {
                    unordered_map post1 = DATABASE["POSTS"].getByID(to_string(i));
                    post p = {{post1["autor"]}, post1["contenido"]};
                    POSTS.push_back(p);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }

            return server.Render("templates/home_logged.html", {{"user", req.session["user"]}, {"posts", posts_to_string(POSTS)}});
        }
        else
            return server.Render("templates/home.html");
    }
}

types::HttpResponse user_account(Request &req)
{
    if (req.session["logged"] == "true" && req.parameters["iuserid"] == req.session["user"])
        return server.Render("templates/user.html", {{"user", req.parameters["iuserid"]}, {"pass", DATABASE.getUser(req.parameters["iuserid"])[2]}});
    else
        return server.Redirect("/login");
}

types::HttpResponse images(Request &req)
{
    return server.Render("templates/image.html", {{"id", req.parameters["iuserid"]}});
}

types::HttpResponse login(Request &req)
{
    if (req.method == GET)
    {
        if (req.session["logged"] == "true" )
            return server.Redirect("/");
        return server.Render("templates/login.html");
    }
    else if (req.method == POST)
    {
        // vector<vector<string>> USERS;
        try
        {
            vector<string> user = DATABASE.getUser(req.content["fname"]);

            if (crypto_lib::calculateSHA512(req.content["fpass"]) == user[2] && req.content["fname"] == user[1])
            {
                req.session["logged"] = "true";
                req.session["user"] = user[1];
                return server.Redirect("/");
            }
            return server.Render("templates/login.html", {{"error", "true"}});
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            server.InternalServerError();
        }
    }
    return server.NotFound();
}

types::HttpResponse logout(Request &req)
{
    if (req.session["logged"] == "true" )
        req.session.destroySession();
    return server.Redirect("/login");
}

int main(int argc, char **argv)
{
    try
    {

        // DATABASE["POSTS"].insertRow({{"autor", "manu"}, {"contenido", "HOLA :D"}, {"FECHA", "2022-01-01"}});
        // DATABASE["POSTS"].insertRow({{"autor", "juan"}, {"contenido", "AAAAAAA"}, {"FECHA", "2022-01-01"}});
        // DATABASE["POSTS"].insertRow({{"autor", "manu"}, {"contenido", "asdf"}, {"FECHA", "2022-01-01"}});
        // DATABASE["POSTS"].insertRow({{"autor", "rich"}, {"contenido", "adios"}, {"FECHA", "2022-01-01"}});

        DATABASE.insertUser("manu", {{"pass", crypto_lib::calculateSHA512("asdf1234")}});
        DATABASE.insertUser("juan", {{"pass", crypto_lib::calculateSHA512("123")}});
        DATABASE.insertUser("rich", {{"pass", crypto_lib::calculateSHA512("456")}});
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    server["secret_key"] = uuid::generate_uuid_v4();
    server["max_connections"] = 10;

    // Ruta sin variables
    server.addRoute("/home", home, {GET, POST});
    server.addRoute("/", home, {GET, POST});
    server.addRoute("/user/<iuserid>", user_account, {GET, POST});

    server.addRoute("/login", login, {GET, POST});
    server.addRoute("/logout", logout, {GET});
    server.addRoute("/image/<id>", images, {GET});

    server.startListener();
}
