#pragma once

// Структура для зберігання координат клітинки сітки
struct cell {
    int row;
    int col;

    // Конструктор за замовчуванням
    cell() = default;

    // Конструктор з параметрами
    cell(int r, int c) {
        row = r;
        col = c;
    }

    // Оператор порівняння на рівність
    bool operator==(const cell &other) const {
        return row == other.row && col == other.col;
    }

    // Оператор для впорядкування (наприклад, для set/map)
    bool operator<(const cell &other) const {
        return row < other.row || (row == other.row && col < other.col);
    }
};
