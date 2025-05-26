#include "nurikabe_solver.hpp"
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <stdexcept>

// Рекурсивний алгоритм пошуку
std::vector<nurikabe_solver::grid> nurikabe_solver::solve(const grid initial) {
    // Зберігаємо незмінну копію та ініціалізуємо робочу сітку
    initial_grid_ = initial;
    rows_ = static_cast<int>(initial.size());
    cols_ = rows_ ? static_cast<int>(initial[0].size()) : 0;
    grid_ = initial;
    used_.assign(rows_ * cols_, 0);

    // Рекурсивний пошук
    if (!solve_recurse()) {
        // Повертаємо початковий стан глобальних полів
        grid_.clear();
        used_.clear();
        rows_ = 0;
        cols_ = 0;
        initial_grid_.clear();
        throw std::runtime_error("Рішення не знайдено");
    }

    grid_.clear();
    used_.clear();

    // Відновлюємо покроково рішення
    std::vector<nurikabe_solver::grid> res = recover_step_by_step();
    // Повертаємо початковий стан глобальних полів
    rows_ = 0;
    cols_ = 0;
    initial_grid_.clear();

    // Повертаємо покрокове відновлення
    return res;
}

bool nurikabe_solver::solve_recurse() {
    // Скорочена назва для пари двох клітинок
    using anchor = std::pair<cell, cell>;
    // Дуже важлива частина програми - компаратор для пріоритезації пар з меншою кількістю можливих шляхів
    auto cmp = [&](anchor const &p, anchor const &q) {
        int sum_a = grid_[p.first.row][p.first.col] + grid_[p.second.row][p.second.col];
        int sum_b = grid_[q.first.row][q.first.col] + grid_[q.second.row][q.second.col];
        // Знаходиться різниця між загальною кількістю клітинок і мінімальною кількістю для з'єднання початку і кінця
        int free_a = sum_a -  manhattan(p.first,p.second);
        int free_b = sum_b - manhattan(q.first,q.second);
        if (free_a != free_b) return free_a > free_b;
        /*
            Якщо кількість "вільних" клітинок рівна, в пріоритеті пара з більшою сумую загальних клітинок,
            так як вона дозволить в майбутньому значно зменшити зону пошуку
        */
        return sum_a < sum_b;
    };
    // Пріоритетна черга для зберігання "найкращих" пар
    std::priority_queue<anchor, std::vector<anchor>, decltype(cmp)> pq(cmp);

    // Список для зберігання вільних клітинок з числами
    std::vector<cell> nums;
    collect_numbers(nums);

    if (nums.empty()) {
        /*
            Якщо вільних клітинок не залишилося - заповнюємо можливі пропуски в сітці і перевіряємо чи є чорні блоки 2x2,
            якщо ні - рішення знайдено і повертаємо true, якщо так - відновлюєм пропуски і повертаємо false
        */
        std::vector<cell> spaces;
        fill_spaces(spaces);
        if (!has_black_2x2_block()) return true;
        for (auto &cc : spaces) grid_[cc.row][cc.col] = EMPTY;
        return false;
    }
    // Створюємо список для зберігання кількості пар для кожної вершини, якщо для якоїсь вершини перестає існувати можлива пара - повертаємо false
    std::vector<int> degree(rows_*cols_, 0);
    // Заповнюємо список "degree" і пріоритетну чергу
    for (size_t i = 0; i < nums.size(); ++i)
        for (size_t j = i+1; j < nums.size(); ++j) {
            cell a = nums[i], b = nums[j];
            int tot = grid_[a.row][a.col] + grid_[b.row][b.col];
            if (tot > manhattan(a,b)) {
                anchor pr = std::minmax(a,b);
                pq.push(pr);
                degree[a.row*cols_ + a.col]++;
                degree[b.row*cols_ + b.col]++;
            }
        }

    // Перебираємо всі можливі пари, застосовуючи пріоритезацію
    while (!pq.empty()) {
        auto pr = pq.top(); pq.pop();
        cell f = pr.first, c = pr.second;
        int total = grid_[f.row][f.col] + grid_[c.row][c.col];
        // Знаходимо всі шляхи між 2 вершинами
        auto paths = find_all_valid_paths(f, c, total);
        for (auto &path : paths) {
            // Позначаємо поточний шлях як "FILLED", тобто уже сформовану область
            for (int idx = 0; idx < rows_*cols_; ++idx) {
                if (path[idx] && idx != f.row*cols_+f.col && idx != c.row*cols_+c.col)
                    grid_[idx/cols_][idx%cols_] = FILLED;
            }
            /* 
                Позначаємо всі сусідні клітинки області в чорний, попередньо записуючи їх у список для майбутнього можливого відновлення,
                якщо шлях некоректний
            */
            std::vector<char> painted(rows_*cols_, 0);
            paint_adjacent(path, painted);

            // Позначаємо вершини як "використані"
            used_[f.row*cols_+f.col] = used_[c.row*cols_+c.col] = 1;
            /*
                Перевірка чи має поточна область чорні блоки 2 на 2 та чи чорна область зв'язна,
                якщо - так викликаємо рекурсивно метод solve, якщо він повертає true - передаємо цей результат далі по стеку
            */
            if (!has_black_2x2_block() && is_black_area_connected() && solve_recurse()) {
                path_stack_.push(path);
                return true;
            }

            // Шлях був неправильний, повертаємо вершини назад
            used_[f.row*cols_+f.col] = used_[c.row*cols_+c.col] = 0;
            // Шлях був неправильний, прибираємо позначення області як заповненої і перефарбовуємо сусідів в пусті клітинки
            for (int idx = 0; idx < rows_*cols_; ++idx) {
                if (path[idx] && idx != f.row*cols_+f.col && idx != c.row*cols_+c.col)
                    grid_[idx/cols_][idx%cols_] = EMPTY;
                if (painted[idx])
                    grid_[idx/cols_][idx%cols_] = EMPTY;
            }
        }
        // Зменшуємо кількість пар для вершини f і c, якщо для якоїсь вершини не залишається вільної пари - повертаємо false
        if (--degree[f.row*cols_+f.col] == 0 || --degree[c.row*cols_+c.col] == 0)
            return false;
    }
    return false;
}

std::vector<nurikabe_solver::grid> nurikabe_solver::recover_step_by_step() {
    std::vector<grid> steps;
    grid current = initial_grid_;

    // Для кожного шляху послідовно маркуємо клітинки й додаємо новий стан
    while (!path_stack_.empty()) {
        // Заповнюємо FILLED
        Path path = path_stack_.top();
        path_stack_.pop();
        for (int idx = 0; idx < rows_*cols_; ++idx) {
            if (path[idx] && current[idx/cols_][idx%cols_] == EMPTY)
                current[idx/cols_][idx%cols_] = FILLED;
        }
        // Фарбуємо чорним
        for (int idx = 0; idx < rows_*cols_; ++idx) {
            if (!path[idx]) continue;
            int r = idx/cols_, c = idx%cols_;
            for (int d = 0; d < 4; ++d) {
                int nr = r + DX_[d], nc = c + DY_[d];
                int nid = nr*cols_ + nc;
                if (out_of_bounds(nr,nc) || current[nr][nc]==BLACK || path[nid]) continue;
                current[nr][nc] = BLACK;
            }
        }
        steps.push_back(current);
    }

    // Фінальне заповнення решти порожніх
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            if (current[r][c] == EMPTY) {
                current[r][c] = BLACK;
            }
        }
    }
    steps.push_back(current);
    return steps;
}

// Метод для зібрання клітинок з числами в список
void nurikabe_solver::collect_numbers(std::vector<cell> &cells) {
    cells.clear();
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            int id = r*cols_ + c;
            if (grid_[r][c] > 0 && !used_[id]) {
                cells.emplace_back(r,c);
            }
        }
    }
}

// Метод для знаходження всіх можливіх шляхів між 2 вершинами
std::vector<nurikabe_solver::Path>
nurikabe_solver::find_all_valid_paths(cell start, cell end, int totalSize) {
    using Path = std::vector<char>;
    /* 
        Створюємо map для зберігання унікальних шляхів, prev для шляхів k довжини і curr для шляхів k+1 довжини
        Значення для кожного шляху в мепі - це найближча клітинка до кінця, допомагає завчасно обрізати "безнадійні" шляхи
    */
    std::unordered_map<Path,cell,PathHash,PathEq> prev, curr;
    // Ініціалізуємо prev шляхами довжиною 1 - а саме шляхом з однієї стартової клітини
    Path init(rows_*cols_, 0);
    init[start.row*cols_ + start.col] = 1;
    prev.emplace(init, start);

    // Шукаємо всі шляхи довжини [1, totalSize] зберігаючи тільки останні
    for (int len = 1; len < totalSize; ++len) {
        for (auto &kv : prev) {
            const auto &mask = kv.first;
            cell close = kv.second;
            // Перевірка досяжності кінця з найближчої до нього точки
            if (len + manhattan(close,end) > totalSize) continue;
            // Пробуємо розширитися з всіх клітинок шляху
            for (int idx = 0; idx < rows_*cols_; ++idx) {
                if (!mask[idx]) continue;
                int r = idx/cols_, c = idx%cols_;
                for (int d = 0; d < 4; ++d) {
                    int nr = r + DX_[d], nc = c + DY_[d];
                    int nid = nr*cols_ + nc;
                    // Якщо клітинка за межами поля або це чорна клітинка або вона уже належить до нашого шляху - пропускаємо
                    if (out_of_bounds(nr,nc) || grid_[nr][nc]==BLACK || mask[nid]) continue;
                    cell nb(nr,nc);
                    // Перевірка чи немає ця клітинка сусідніх клітинок з числами крім end
                    if (!is_expandable(nb,end,mask)) continue;
                    // Якщо це шлях максимальної (totalSize) довжини і в ньому немає end - пропускаємо
                    if (len==totalSize-1 && !mask[end.row*cols_+end.col] && nid != end.row*cols_+end.col) continue;
                    // Копіюємо шлях (len-1) довжини і додаємо до нього поточну клітинку
                    Path np = mask; np[nid] = 1;
                    // Знаходимо нову найближчу клітинку до кінця
                    cell nclose = (manhattan(nb,end)<manhattan(close,end) ? nb : close);
                    curr.emplace(std::move(np), nclose);
                }
            }
        }
        // Тепер curr зберігатиме шляхи довжиною len + 1, тому свапаємо prev і curr і очищаємо curr
        prev.swap(curr);
        curr.clear();
    }

    // Зберігаємо результат в вектор
    std::vector<Path> result;
    for (auto &kv : prev) {
        result.push_back(kv.first);
    }
    return result;
}

// Метод для знаходження манхетенського (найкоротшого) шляху
int nurikabe_solver::manhattan(cell a, cell b) const {
    return std::abs(a.row - b.row) + std::abs(a.col - b.col);
}

// Метод для перевірки наявності інших клітинок з числами крім end для заданої клітинки
bool nurikabe_solver::is_expandable(cell a, cell end, const Path &path) const {
    for (int d = 0; d < 4; ++d) {
        int nr = a.row + DX_[d], nc = a.col + DY_[d];
        int nid = nr*cols_ + nc;
        if (out_of_bounds(nr,nc) || grid_[nr][nc]==BLACK || path[nid]) continue;
        if (grid_[nr][nc]>0 && !(cell(nr,nc)==end)) return false;
    }
    return true;
}

// Метод для перевірки меж для заданих координат
bool nurikabe_solver::out_of_bounds(int r, int c) const {
    return r<0 || c<0 || r>=rows_ || c>=cols_;
}

// Метод для фарбування сусідніх клітинок області в чорний колір і попереднє збереження їх в список "painted" для майбутнього можливого відновлення
void nurikabe_solver::paint_adjacent(const Path &path, std::vector<char> &painted) {
    for (int idx = 0; idx < rows_*cols_; ++idx) {
        if (!path[idx]) continue;
        int r = idx/cols_, c = idx%cols_;
        for (int d = 0; d < 4; ++d) {
            int nr = r + DX_[d], nc = c + DY_[d];
            int nid = nr*cols_ + nc;
            if (out_of_bounds(nr,nc) || grid_[nr][nc]==BLACK || path[nid]) continue;
            grid_[nr][nc] = BLACK;
            painted[nid] = 1;
        }
    }
}

// Метод для перевірки зв'язності чорної області
bool nurikabe_solver::is_black_area_connected() const {
    // Список visited для зберігання пройдених клітинок
    std::vector<char> vis(rows_*cols_, 0);
    // st - стартова клітинка, found - чи була знайдена стартова клітинка, cnt - кількість чорних клітинок
    int start = -1, cnt = 0;
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            int id = r*cols_ + c;
            /*
                Тут дуже цікавий момент, так як пусті клітинки в майбутньому можуть стати чорними, ми їх також рахуємо за чорні,
                не рахуємо тільки заповнені і клітинки з цифрами
            */
            if (grid_[r][c]==BLACK || grid_[r][c]==EMPTY) {
                if (start<0) start = id;
                ++cnt;
            }
        }
    }
    // Якщо не було знайдено ні одної чорної клітинки, повератаємо true, так як технічно нульова область також зв'язна
    if (start<0) return true;
    /*
        Перевіряємо чи збігається кількість усіх чорних клітинок з кількістю досяжних клітинок починаючи з start
        (якщо область зв'язна, вони повинні співпадати)
    */
    return dfs_count_black(vis, {start/cols_, start%cols_}) == cnt;
}

// Метод для підрахунку всіх досяжних чорних клітинок починаючи з start
int nurikabe_solver::dfs_count_black(std::vector<char> &vis, cell c) const {
    int id = c.row*cols_ + c.col;
    vis[id] = 1;
    int sum = 1;
    for (int d = 0; d < 4; ++d) {
        int nr = c.row + DX_[d], nc = c.col + DY_[d];
        int nid = nr*cols_ + nc;
        if (out_of_bounds(nr,nc) || vis[nid]) continue;
        if (grid_[nr][nc]!=BLACK && grid_[nr][nc]!=EMPTY) continue;
        sum += dfs_count_black(vis, {nr,nc});
    }
    return sum;
}

// Метод для перевірки існування чорних блоків 2x2
bool nurikabe_solver::has_black_2x2_block() const {
    for (int r = 0; r+1 < rows_; ++r) {
        for (int c = 0; c+1 < cols_; ++c) {
            if (grid_[r][c]==BLACK && grid_[r+1][c]==BLACK &&
                grid_[r][c+1]==BLACK && grid_[r+1][c+1]==BLACK) {
                return true;
            }
        }
    }
    return false;
}

// Метод для заповнення можливих пропусків в кінці і попередній запис їх в список spaces для майбутнього можливого відновлення
void nurikabe_solver::fill_spaces(std::vector<cell> &spaces) {
    spaces.clear();
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            if (grid_[r][c]==EMPTY) {
                spaces.emplace_back(r,c);
                grid_[r][c] = BLACK;
            }
        }
    }
}
