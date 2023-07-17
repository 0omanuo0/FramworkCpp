#pragma once

#include <string>
#include <random>
#include <ctime>

class idGenerator
{
    private:
        /* data */
    public:
        idGenerator(/* args */);
        static int generateID(){return atoi(generateIDstr().c_str());}
        static std::string generateIDstr(){
            std::srand(std::time(nullptr));
            std::string id;
            for (size_t i = 0; i < 32; i++)
                id.append(std::to_string(abs(rand()+std::rand())%10));
            return id;
        }
};

