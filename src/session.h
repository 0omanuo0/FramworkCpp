#pragma once
class Session
{
private:
    /* data */
public:
    std::string id;
    std::map<std::string, std::string> values;
    bool create;
    bool isEmpty(){
        return id.empty() ? true : false;
    }
    void addValue(std::string key, std::string value){
        values.insert(make_pair(key, value));
    }
    void modifyValue(std::string key, std::string value){
        values[key] = value;
    }
    std::string operator[](std::string key){
        return values[key];
    }
    
    Session(std::string id_f){id = id_f;}
    Session(std::string id_f, std::string key, std::string value){
        id = id_f;
        values.insert(make_pair(key, value));
        create = true;
    }
};


