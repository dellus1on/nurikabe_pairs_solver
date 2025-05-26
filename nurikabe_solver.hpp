#pragma once

#include <vector>
#include <iostream>
#include <stack>
#include <set>
#include "Cell.hpp"

// Клас, що реалізує розв'язувач головоломки Nurikabe (пари чисел)
class nurikabe_solver {
public:
    // Позначення типів клітинок
    enum cell_state : int {
        EMPTY = 0,     // порожня клітинка
        BLACK = -1,    // чорна (вода)
        FILLED = -2    // заповнена клітинка острова (не число)
    };

    // Скорочена назва для сітки
    using grid = std::vector<std::vector<int>>;

    nurikabe_solver() = default;

    // Запускає розв'язання задачі та повертає послідовність проміжних станів
    std::vector<grid> solve(const grid initial);

private:
    int rows_ = 0;
    int cols_ = 0;

    grid grid_; // робоча копія сітки
    grid initial_grid_; // початкова сітка
    std::vector<std::vector<bool>> used_; // позначення використаних чисел
    std::stack<std::vector<std::vector<bool>>> path_stack_; // стек для відновлення покрокового рішення

    // Зміщення по 4 напрямках: вверх, вниз, вліво, вправо
    static constexpr int DX_[4] = { -1, 1, 0, 0 };
    static constexpr int DY_[4] = { 0, 0, -1, 1 };

    // Основний рекурсивний метод для пошуку рішення
    bool solve_recurse();

    // Відновлення кроків розв'язку за шляхами зі стеку
    std::vector<grid> recover_step_by_step();

    // Збирає координати ще не використаних чисел на сітці
    void collect_numbers(std::vector<cell> &cells);

    // Повертає всі допустимі шляхи між двома числами, які можна з'єднати
    std::vector<std::vector<std::vector<bool>>> find_all_valid_paths(cell start, cell end, int total_size);

    // Обчислює мангетенську (найкоротшу) відстань між двома клітинками
    int manhattan(cell a, cell b) const;

    // Перевіряє, чи клітинка може бути додана до поточного шляху
    bool is_expandable(cell a, cell end, const std::vector<std::vector<bool>> &path) const;

    // Перевіряє, чи координати виходять за межі поля
    bool out_of_bounds(int r, int c) const;

    // Фарбує сусідні клітинки шляху в чорний колір
    void paint_adjacent(const std::vector<std::vector<bool>> &path, std::vector<std::vector<bool>> &painted);

    // Перевіряє, чи всі чорні клітинки утворюють єдину зв'язну область
    bool is_black_area_connected() const;

    // DFS-обхід, який рахує кількість зв'язаних чорних клітинок
    int dfs_count_black(std::vector<std::vector<bool>> &visited, cell c) const;

    // Перевіряє, чи є в сітці чорний квадрат 2×2
    bool has_black_2x2_block() const;

    // Тимчасово заповнює всі порожні клітинки чорним і зберігає список для відновлення
    void fill_spaces(std::vector<cell> &spaces);
};
