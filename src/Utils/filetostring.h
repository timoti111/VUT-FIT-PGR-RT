#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include "Utils/Shadinclude.hpp"

static class Utils {// 1. retrieve the vertex/fragment source code from filePath
public:
    static std::string readFileToString(std::string path) {
        std::string text;
        std::ifstream file;
        // ensure ifstream objects can throw exceptions:
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            // open files
            file.open(path);
            std::stringstream shaderStream;
            // read file's buffer contents into streams
            shaderStream << file.rdbuf();
            // close file handlers
            file.close();
            // convert stream into string
            text = shaderStream.str();
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }

        return text;
    }
};

