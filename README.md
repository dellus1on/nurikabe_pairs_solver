# Nurikabe Pairs Solver

Це консольна C++ програма для розв’язання модифікованої версії японської головоломки **«Нурікабе»**, де в одній білій області може бути **дві підказки**, а розмір області дорівнює **сумі цих чисел**.

## 🔧 Можливості
- Введення головоломки вручну або з файлу `input.txt`
- Перевірка користувацького рішення
- Автоматичне пошукове розв’язання з виводом усіх кроків
- Оптимізація з використанням **евристики Манхеттенської відстані**

## 📁 Структура проєкту
- `cell.hpp` — структура координат
- `grid_reader.cpp/.hpp` — зчитування вхідних даних
- `nurikabe_solver.cpp/.hpp` — основна логіка розв’язання
- `user_solution_checker.cpp/.hpp` — перевірка рішень користувача
- `nurikabe_pairs.cpp` — головний файл `main()`

## 🚀 Запуск

### 1. Звичайна компіляція:
```bash
g++ -std=c++17 -o solve nurikabe_pairs.cpp nurikabe_solver.cpp grid_reader.cpp user_solution_checker.cpp
```

### Компіляція з оптимізацією:
```bash
g++ -std=c++17 -O3 -march=native -flto -DNDEBUG -o solve nurikabe_pairs.cpp nurikabe_solver.cpp grid_reader.cpp user_solution_checker.cpp
```

### Запуск програми:
```bash
./solve
```
