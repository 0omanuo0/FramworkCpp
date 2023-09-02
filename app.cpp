#include <iostream>
#include "src/server.h"
#include "users.hpp"

using namespace std;

const int PORT = 8443;
const int MAX_CONNECTIONS = 500;

string HTTPScontext[] = {"secrets/cert.pem", "secrets/key.pem"};

vector<user> USERS = {{"manu", "asdf"}, {"user2", "pass2"}};
vector<post> POSTS = {{USERS[0], "post1"}, {USERS[1], "post2"}};

HttpServer server(PORT, HTTPScontext, "server-manu.local", MAX_CONNECTIONS);

string home(Args &args)
{
    if(args.request.method == POST){
        if(args.session["logged"] == "true" && !args.session.isEmpty()){
            POSTS.push_back({getUser(args.session["user"], USERS), args.request.content["post"]});
            return server.Render("templates/home_logged.html", {{"user", args.session["user"]}, {"posts", posts_to_string(POSTS)}});
        }
        else
            return server.Render("templates/home.html");
        return Redirect(args.ssl, "/");
    }
    else if(args.request.method == GET){
        if (args.session["logged"] == "true" && !args.session.isEmpty())
            return server.Render("templates/home_logged.html", {{"user", args.session["user"]}, {"posts", posts_to_string(POSTS)}});
        else 
            return server.Render("templates/home.html");
    }
}

string user_account(Args &args){
    if (args.session["logged"] == "true" && args.vars[0] == args.session["user"])
        return server.Render("templates/user.html", {{"user", args.vars[0]}, {"pass", getUser(args.vars[0], USERS).pass}} );
    else 
        return Redirect(args.ssl, "/login");
}

string images(Args &args){
    return server.Render("templates/image.html", {{"id", args.vars[0]}});
}

string login(Args &args)
{
    if (args.request.method == GET)
    {
        if (args.session["logged"] == "true" && !args.session.isEmpty())
            return Redirect(args.ssl, "/");
        return server.Render("templates/login.html");
    }
    else if (args.request.method == POST)
    {
        for(auto &user : USERS){
            if(args.request.content["fpass"] == user.pass && args.request.content["fname"] == user.name){
                args.session.createSession();
                args.session["logged"] = "true";
                args.session["user"] = user.name;
                return Redirect(args.ssl, "/");
            }
        }
        return server.Render("templates/login.html", {{"error", "true"}});

    }
    return "error";
}

string logout(Args &args)
{
    if (args.session["logged"] == "true" && !args.session.isEmpty())
        args.session.destroySession();
    return Redirect(args.ssl, "/login");
}

int main(int argc, char **argv)
{
    // Ruta sin variables
    server.addRoute("/home", home, {GET, POST});
    server.addRoute("/", home, {GET, POST});
    server.addRoute("/user/<iuserid>", user_account, {GET, POST});

    server.addRoute("/login", login, {GET, POST});
    server.addRoute("/logout", logout, {GET});
    server.addRoute("/image/<id>", images, {GET});

    server.setup();
    server.startListener();
}
