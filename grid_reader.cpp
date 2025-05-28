#include "grid_reader.hpp"
#include <limits>
#include <fstream>

// Зчитування сітки з консолі
grid_data grid_reader::from_console(std::istream &in, std::ostream &out) {
    grid_data data;

    // Зчитуємо рядки
    while (true) {
        out << "\nВведіть кількість рядків: ";
        if ((in >> data.rows) && data.rows > 0) break;
        out << "Помилка: введіть додатнє ціле число\n";
        in.clear();
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Зчитуємо стовпці
    while (true) {
        out << "Введіть кількість стовпців: ";
        if ((in >> data.cols) && data.cols > 0) break;
        out << "Помилка: введіть додатнє ціле число\n";
        in.clear();
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    data.grid.assign(data.rows, std::vector<int>(data.cols, 0));
    out << "\nПримітка:\n"
        "n = 0  — пуста клітинка\n"
        "n > 0  — підказка (ціле число)\n\n";

    // Кількість підказок в рядку
    int hint_count = 0;
    int hint_sum = 0;

    for (int r = 0; r < data.rows; ++r) {
        while (true) {
            out << "Введіть " << data.cols << " чисел для рядка " << r + 1 << ": ";
            int temp_count = 0;
            int temp_hint_sum = 0;
            bool format_error = false;
            for (int c = 0; c < data.cols; ++c) {
                if (!(in >> data.grid[r][c])) {
                    out << "Помилка: введіть ціле число.\n";
                    format_error = true;
                    break;
                }

                int v = data.grid[r][c];
                // Дозволяємо тільки додатні значення
                if (v < 0) {
                    out << "Помилка: від’ємне значення.\n";
                    format_error = true;
                    break;
                }
                // Не дозволяємо вводити значення більшим за розміри сітки
                if (v > data.rows * data.cols) {
                    out << "Помилка: значення завелике для сітки.\n";
                    format_error = true;
                    break;
                }
                // Перевірка сусідніх підказок і суми
                if (v > 0) {
                    if (c > 0 && data.grid[r][c - 1] > 0) {
                        out << "Помилка: сусідні підказки.\n";
                        format_error = true;
                        break;
                    }
                    if (r > 0 && data.grid[r - 1][c] > 0) {
                        out << "Помилка: сусідні підказки.\n";
                        format_error = true;
                        break;
                    }
                    if (hint_sum + temp_hint_sum + v > data.rows * data.cols) {
                        out << "Помилка: перевищено суму підказок.\n";
                        format_error = true;
                        break;
                    }
                    temp_count++;
                    temp_hint_sum += v;
                }
            }

            // Якщо помилка - очищаємо стан потоку, відкидаємо залишок рядка і заново читаємо рядок
            if (format_error) {
                in.clear();
                in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } else {
                hint_count += temp_count;
                hint_sum += temp_hint_sum;
                break;
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
