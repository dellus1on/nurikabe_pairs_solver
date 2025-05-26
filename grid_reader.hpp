#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

// Структура для зберігання розмірів і значень сітки
struct grid_data {
    int rows;
    int cols;
    std::vector<std::vector<int>> grid;
};

class grid_reader {
public:
    /*
        Зчитування сітки з консолі (std::cin / std::cout)
        У разі помилки форматування кидає виняток std::runtime_error
    */
    static grid_data from_console(std::istream &in, std::ostream &out);

    /*
        Зчитування сітки з файлу за вказаним шляхом
        У разі помилки відкриття або формату кидає std::runtime_error
    */
    static grid_data from_file(const std::string &filepath);
};
