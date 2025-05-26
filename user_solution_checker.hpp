#pragma once

#include <vector>
#include <iostream>
#include <string>
#include "cell.hpp"

class user_solution_checker {
public:
    // Ініціалізація перевірки з початковою сіткою
    user_solution_checker(const std::vector<std::vector<int>> &initial_grid);

    /*
        Інтерактивне зчитування рішення та його перевірка
        Повертає true, якщо рішення коректне
        Під час зчитування одразу виявляє помилки формату (пусті клітинки, невірні підказки, сусідні підказки тощо)
    */
    bool input_solution_interactive(std::istream &in, std::ostream &out);

private:
    const std::vector<std::vector<int>> &initial_;
    int rows_, cols_;
    std::vector<std::vector<int>> user_grid_;

    // Фаза зчитування даних від користувача
    bool input_phase(std::istream &in, std::ostream &out);

    // Фаза перевірки правильності введеного розв’язку
    bool validation_phase(std::ostream &out);

    // Перевірка наявності чорного блока 2x2. Якщо знайдено — повертає координати верхнього лівого кута.
    bool check_black_2x2_block(cell &lt) const;

    // Перевірка зв’язності чорної області
    bool check_black_connectivity() const;

    // Перевірка коректності островів. У разі помилки повертає причину та місце.
    bool check_islands(std::string &reason, cell &loc) const;

    static constexpr int DR_[4] = {-1, 1, 0, 0};
    static constexpr int DC_[4] = {0, 0, -1, 1};

    // Перевірка виходу за межі сітки
    bool out_of_bounds(int r, int c) const {
        return r < 0 || c < 0 || r >= rows_ || c >= cols_;
    }
};
