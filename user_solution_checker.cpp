#include "user_solution_checker.hpp"
#include <limits>
#include <queue>
#include <iomanip>

user_solution_checker::user_solution_checker(const std::vector<std::vector<int>>& initialGrid) : initial_(initialGrid),
    rows_(static_cast<int>(initialGrid.size())),
    cols_(rows_ ? static_cast<int>(initialGrid[0].size()) : 0),
    user_grid_(rows_, std::vector<int>(cols_, 0))
{}

// Метод для перевірки правильності вводу і коректності користувацького рішеня
bool user_solution_checker::input_solution_interactive(std::istream& in, std::ostream& out) {
    out << "\nПримітка для рішення:\n"
        "n = -1 — чорна клітинка\n"
        "n = -2 — заповнена клітинка\n"
        "n >  0 — підказка (тільки у клітинках з початковими підказками)\n\n";

    // Звичитуємо користувацьку сітку, якщо вона має неправильний формат - повертаєм false
    if (!input_phase(in, out))
        return false;

    // Повертаємо результат перевірки користувацього рішення на коректність
    return validation_phase(out);
}

// Метод для перевірки правильності вводу користувацього рішення
bool user_solution_checker::input_phase(std::istream& in, std::ostream& out) {
    // Порядково зчитуєм сітку рішення від користувача
    for (int r = 0; r < rows_; ++r) {
        while (true) {
            out << "Введіть " << cols_ << " значень для рядка " << (r+1)
    << " через пробіл: " << std::flush;
            std::vector<int> row_vals(cols_);
            bool format_error = false;
            // Зчитуємо весь рядок користувача в список для подальшої перевірки
            for (int c = 0; c < cols_; ++c) {
                if (!(in >> row_vals[c])) {
                    out << "Помилка: введіть ціле число.\n";
                    in.clear();
                    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    format_error = true;
                    break;
                }

                int v = row_vals[c];
                // Допускаються тільки заповнені, чорні, і клітинки з підказками
                if (v < -2 || v == 0) {
                    out << "Помилка: недопустиме значення "<<v
                        <<" ("<<r+1<<","<<c+1<<").\n";
                    format_error = true;
                    break;
                }
                /*
                    Якщо користувач ввів число більше нуля (клітинка з підказкою), перевіряєм чи є вона на цьому місці
                    в початковій сітці і чи значення збігаються
                */
                if (v > 0 && initial_[r][c] != v) {
                    out << "Помилка: підказка "<<v
                        <<" не була в початковій сітці ("<<r+1<<","<<c+1<<").\n";
                    format_error = true;
                    break;
                }
                // Перевірки чи немає двох сусідніх клітинок з підказками
                if (v > 0) {
                    if (c>0 && row_vals[c-1]>0) {
                        out << "Помилка: дві підказки підряд у рядку "<<(r+1)
                            <<" між стовпцями "<<c<<" та "<<c+1<<".\n";
                        format_error = true;
                        break;
                    }
                    if (r>0 && user_grid_[r-1][c]>0) {
                        out << "Помилка: дві підказки підряд у стовпці "<<(c+1)
                            <<" між рядками "<<r<<" та "<<r+1<<".\n";
                        format_error = true;
                        break;
                    }
                }
            }

            if (!format_error) {
                // Якщо рядок коректний - записуєм його користвацьку сітку для майбутньої перевірки
                user_grid_[r] = std::move(row_vals);
                break;
            }
        }
    }
    return true;
}

bool user_solution_checker::validation_phase(std::ostream& out) {
    // Координати верхнього лівого кута чорного блока 2x2, якщо він буде
    cell lt;
    if (check_black_2x2_block(lt)) {
        // Якщо чорний блок було знайдено, виводим помилку і координати лівого верхнього кута
        out << "Помилка: знайдено 2×2 чорний блок у клітині ("
            << lt.row+1 << "," << lt.col+1 << ")\n";
        return false;
    }
    // Перевірка чорної області на зв'язність
    if (!check_black_connectivity()) {
        out << "Помилка: чорна область незв'язна\n";
        return false;
    }
    // Причина помилки
    std::string reason;
    // Клітинка де знайдена помилка
    cell loc;
    if (!check_islands(reason, loc)) {
        out << "Помилка: " << reason;
        if (loc.row >= 0)
            out << " (рядок " << loc.row+1 << ", стовпець " << loc.col+1 << ")";
        out << "\n";
        return false;
    }

    out << "\nРішення правильне!\n";
    return true;
}

// Метод для перевірки наявності чорних блоків 2x2 і повернення лівої верхньої клітинки проблемного блоку
bool user_solution_checker::check_black_2x2_block(cell &lt) const {
    for (int r = 0; r+1 < rows_; ++r) {
        for (int c = 0; c+1 < cols_; ++c) {
            if (user_grid_[r][c]==-1 &&
                user_grid_[r+1][c]==-1 &&
                user_grid_[r][c+1]==-1 &&
                user_grid_[r+1][c+1]==-1)
            {
                lt.row = r; lt.col = c;
                return true;
            }
        }
    }
    return false;
}

// Метод для перевірки зв'язності чорної області
bool user_solution_checker::check_black_connectivity() const {
    std::vector<std::vector<bool>> vis(rows_, std::vector<bool>(cols_,false));
    cell start(-1,-1);
    int total_black = 0;
    for (int r = 0; r < rows_; ++r)
        for (int c = 0; c < cols_; ++c)
            if (user_grid_[r][c] == -1) {
                if (start.row < 0) start = cell(r,c);
                ++total_black;
            }
    if (start.row < 0) return true;
    std::queue<cell> q;
    q.push(start);
    vis[start.row][start.col] = true;
    int count = 0;
    while (!q.empty()) {
        cell u = q.front(); q.pop();
        ++count;
        for (int d = 0; d < 4; ++d) {
            int nr = u.row + DR_[d], nc = u.col + DC_[d];
            if (out_of_bounds(nr,nc) || vis[nr][nc] || user_grid_[nr][nc] != -1)
                continue;
            vis[nr][nc] = true;
            q.push(cell(nr,nc));
        }
    }
    return count == total_black;
}

// Метод для перевірки площі островів (повинна мати площу сумі двух клітинок з цифрами)
bool user_solution_checker::check_islands(std::string &reason, cell &loc) const {
    // Список для зберігання пройдених клітинок
    std::vector<std::vector<bool>> vis(rows_, std::vector<bool>(cols_,false));

    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            // Якщо клітинка не пройдена і належить білій області - перевіряємо її
            if (!vis[r][c] && (user_grid_[r][c] > 0 || user_grid_[r][c] == -2)) {
                // Черга для проходу в ширину по білій області
                std::queue<cell> q;
                cell start(-1, -1), end(-1, -1);
                // Змінна для підрахнку розміру білої області
                int size = 0;
                // Додаємо в чергу стартову клітинку і помічаємо її як пройдену
                q.push(cell(r,c));
                vis[r][c] = true;

                // Проходимося по всім клітинкам і рахуєм розмір області, також паралельно записуємо в start і end клітинки з підказками
                while (!q.empty()) {
                    cell u = q.front(); q.pop();
                    size++;
                    if (user_grid_[u.row][u.col] > 0) {
                        if (start.row == -1) {
                            start = u;
                        } else if (end.row == -1) {
                            end = u;
                        } else {
                            reason = "острів має більше 2 підказки";
                            loc = u;
                            return false;
                        }
                    }
                    for (int d = 0; d < 4; ++d) {
                        int nr = u.row + DR_[d], nc = u.col + DC_[d];
                        if (out_of_bounds(nr,nc) || vis[nr][nc]) continue;
                        if (user_grid_[nr][nc] > 0 || user_grid_[nr][nc] == -2) {
                            vis[nr][nc] = true;
                            q.push(cell(nr,nc));
                        }
                    }
                }

                // Якщо клітинок з підказками менше 2 - повертаємо false
                if (start.row == -1 || end.row == -1) {
                    reason = "острів має менше ніж 2 підказки";
                    loc = (start.row != -1 ? start : (end.row != -1 ? end : cell(r, c)));
                    return false;
                }

                // Якщо площа області не збігається з сумою клітинок з підказками - повертаємо false
                int sum = user_grid_[start.row][start.col] + user_grid_[end.row][end.col];
                if (sum != size) {
                    reason = "площа острова (" + std::to_string(size) + ") не дорівнює сумі підказок: " + std::to_string(sum);
                    loc = start;
                    return false;
                }
            }
        }
    }
    return true;
}
