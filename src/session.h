#pragma once
class Session
{
private:
    /* data */
public:
    struct session
    {
        std::string id;
        std::map<std::string, std::string> values;
    };
    session sessionUser;
    bool isEmpty(){
        return sessionUser.id.empty() ? true : false;
    }
    void addValue(std::string key, std::string value){
        sessionUser.values.insert(make_pair(key, value));
    }
    void modifyValue(std::string key, std::string value){
        sessionUser.values[key] = value;
    }
    std::string obtainValue(std::string key){
        return sessionUser.values[key];
    }
    
    Session(std::string id_f){sessionUser.id = id_f;}
};


