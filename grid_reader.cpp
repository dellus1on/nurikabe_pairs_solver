#include "grid_reader.hpp"
#include <limits>
#include <fstream>

// Зчитування сітки з консолі
grid_data grid_reader::from_console(std::istream &in, std::ostream &out) {
    grid_data data;

    out << "\nВведіть кількість рядків: ";
    if (!(in >> data.rows) || data.rows <= 0) {
        throw std::runtime_error("Невірна кількість рядків");
    }

    out << "Введіть кількість стовпців: ";
    if (!(in >> data.cols) || data.cols <= 0) {
        throw std::runtime_error("Невірна кількість стовпців");
    }

    data.grid.assign(data.rows, std::vector<int>(data.cols, 0));
    out << "\nПримітка:\n"
        "n = 0  — пуста клітинка\n"
        "n > 0  — підказка (ціле число)\n\n";

    // Кількість підказок в рядку
    int hint_count = 0;
    int hint_sum = 0;

    for (int r = 0; r < data.rows; ++r) {
        out << "Введіть " << data.cols << " чисел для рядка " << r + 1 << ": ";
        for (int c = 0; c < data.cols; ++c) {
            if (!(in >> data.grid[r][c])) {
                throw std::runtime_error("Неціле число у введенні");
            }

            int v = data.grid[r][c];
            // Дозволяємо тільки додатні значення
            if (v < 0) {
                throw std::runtime_error("Введене від'ємне значення");
            }
            // Не дозволяємо вводити значення більшим за розміри сітки
            if (v > data.rows * data.cols) {
                throw std::runtime_error("Значення забагато для розміру сітки");
            }
            // Перевірка чисел з підказками на сусідство і що сума не повинна перевищувати розміри сітки
            if (v > 0) {
                if (c > 0 && data.grid[r][c - 1] > 0) {
                    throw std::runtime_error("Підказки не можуть бути сусідніми");
                }
                if (r > 0 && data.grid[r - 1][c] > 0) {
                    throw std::runtime_error("Підказки не можуть бути сусідніми");
                }
                if (hint_sum + v > data.rows * data.cols) {
                    throw std::runtime_error("Сума підказок завелика");
                }
                hint_sum += v;
                ++hint_count;
            }
        }
    }

    // Кількість підказок не може бути непарною
    if (hint_count % 2 != 0) {
        throw std::runtime_error("Кількість підказок має бути парною");
    }

    return data;
}

// Зчитування сітки з файлу
grid_data grid_reader::from_file(const std::string &filepath) {
    std::ifstream fin(filepath);
    if (!fin) {
        throw std::runtime_error("Не вдалося відкрити файл " + filepath);
    }

    // Перші 2 числа - розмірі сітки
    grid_data data;
    if (!(fin >> data.rows >> data.cols) || data.rows <= 0 || data.cols <= 0) {
        throw std::runtime_error("Невірний формат розмірів у файлі");
    }

    // Зчитуєм в матрицю
    data.grid.assign(data.rows, std::vector<int>(data.cols));
    for (int r = 0; r < data.rows; ++r) {
        for (int c = 0; c < data.cols; ++c) {
            if (!(fin >> data.grid[r][c])) {
                throw std::runtime_error("Недостатньо даних у файлі");
            }
        }
    }

    return data;
}
