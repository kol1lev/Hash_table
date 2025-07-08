#include <string>
#include <vector>
#include <algorithm>
#include<sstream>
#include<iostream>
#include <iomanip>
#include <cmath>
#include "DataGenerate.hpp"

std::pair<std::string, unsigned int> parse_line(const std::string& line) {
    std::istringstream iss(line);
    std::string word;
    std::string FIO;
    std::string last;

    int index = 0;
    while (iss >> word) {
        if (index < 3) {
            if (!FIO.empty()) FIO += ' ';
            FIO += word;
        }
        last = word;
        ++index;
    }
    return {FIO, static_cast<unsigned int>(std::stoul(last))};
}


struct hastable_item
{
    std::string fio;

    unsigned int id;

    std::string value;

    int status = 0;

    unsigned int hash = 0;

    int line_number = -1;

    hastable_item() = default;

    hastable_item(std::string f, unsigned int id, std::string val, int hash, int lineno)
    : fio(std::move(f)), id(id), value(std::move(val)), status(1),hash(hash), line_number(lineno) {}
};

class hastable
{
    private:
    hastable_item* table;
    int capacity;
    int size = 0;

unsigned int compute_hash_key(const std::string& FIO, unsigned int id) {
    unsigned int key = 0;
    for (char c : FIO) {
        key = key * 31 + static_cast<unsigned int>(c);
    }
    key = key * 31 + id;

    const double A = 0.6180339887;
    double fractional = fmod(key * A, 1.0);
    return static_cast<unsigned int>(fractional * capacity);
}


public:
    hastable(int capacity) : capacity(capacity){
        table = new hastable_item[capacity];
    }

    ~hastable(){
        delete[] table;
    }

void insert(const std::string& line, int lineno) {
    if ((size + 1.0) / capacity >= 0.7) {
        resize(capacity * 2);
    }

    auto [fio, id] = parse_line(line);

    unsigned int full_hash = compute_hash_key(fio, id);
    int base_index = full_hash % capacity;

    for (int i = 0; i < capacity; i++) {
        int index = (base_index + i) % capacity;

        if (table[index].status == 0) break;

        if (table[index].status == 1 &&
            table[index].fio == fio &&
            table[index].id == id) {
            table[index].value = line;
            table[index].line_number = lineno;
            table[index].hash = full_hash;
            return;
        }
    }

    int first_deleted = -1;

    for (int i = 0; i < capacity; i++) {
        int index = (base_index + i) % capacity;

        if (table[index].status == 0) {
            if (first_deleted != -1) {
                table[first_deleted] = hastable_item(fio, id, line, full_hash, lineno);
            } else {
                table[index] = hastable_item(fio, id, line, full_hash, lineno);
            }
            size++;
            return;
        } else if (table[index].status == 2 && first_deleted == -1) {
            first_deleted = index;
        }
    }

    if (first_deleted != -1) {
        table[first_deleted] = hastable_item(fio, id, line, full_hash, lineno);
        size++;
    }
}

void insert_without_resize(const std::string& line, int lineno) {
    auto [fio, id] = parse_line(line);
    unsigned int full_hash = compute_hash_key(fio, id);
    int base_index = full_hash % capacity;


    for (int i = 0; i < capacity; i++) {
        int index = (base_index + i) % capacity;

        if (table[index].status == 0 || table[index].status == 2) {
            table[index] = hastable_item(fio, id, line, full_hash, lineno);
            size++;
            return;
        }
    }
}


std::pair<int, int> search(const std::string& line) {
    auto [fio, id] = parse_line(line);

    unsigned int full_hash = 0;
    for (char c : fio) {
        full_hash = full_hash * 31 + c;
    }
    full_hash = full_hash * 31 + id;

    unsigned int base_index = full_hash % capacity;

    int steps = 0;

    for (int i = 0; i < capacity; i++) {
        int index = (base_index + i) % capacity;
        steps++;

        if (table[index].status == 0) break;

        if (table[index].status == 1 &&
            table[index].fio == fio &&
            table[index].id == id) {
            return {table[index].line_number, steps};
        }
    }

    return {-1, steps};
}



void remove(const std::string& line) {
    auto [fio, id] = parse_line(line);

    unsigned int full_hash = compute_hash_key(fio, id);
    unsigned int base_index = full_hash % capacity;

    for (int i = 0; i < capacity; i++) {
        int index = (base_index + i) % capacity;

        if (table[index].status == 0) break;

        if (table[index].status == 1 &&
            table[index].fio == fio &&
            table[index].id == id &&
            table[index].value == line) {
            table[index].status = 2;
            size--;
            return;
        }
    }

    if (capacity > 10 && (size * 1.0 / capacity) <= 0.3) {
        resize(capacity / 2);
    }
}



void resize(int new_capacity) {
    if (new_capacity < 10) {
        new_capacity = 10;
    }

    auto* old_table = table;
    int old_capacity = capacity;

    table = new hastable_item[new_capacity];
    capacity = new_capacity;
    size = 0;

    for (int i = 0; i < old_capacity; i++) {
        if (old_table[i].status == 1) {
            insert_without_resize(old_table[i].value, old_table[i].line_number);
        }
    }

    delete[] old_table;
}


void print() const {
    const int FIO_WIDTH = 20;
    const int VALUE_WIDTH = 25;
    const int ID_WIDTH = 8;
    const int HASH_WIDTH = 12;

    int active_count = 0;
    int deleted_count = 0;
    int empty_count = 0;
    int logical_index = 1;

    std::cout << "\n┌─────────┬──────────────┬──────────┬──────────────────────┬──────────┬─────────────────────────┐\n";
    std::cout << "│ Индекс  │ Хэш          │ Статус   │ ФИО                  │ ID       │ Значение                │\n";
    std::cout << "├─────────┼──────────────┼──────────┼──────────────────────┼──────────┼─────────────────────────┤\n";

    for (int i = 0; i < capacity; i++) {
        std::string status_str;

        switch (table[i].status) {
            case 0: status_str = "Пусто"; empty_count++; break;
            case 1: status_str = "\033[32mЗанято\033[0m"; active_count++; break;
            case 2: status_str = "\033[31mУдалено\033[0m"; deleted_count++; break;
            default: status_str = "Неизвестно";
        }

        if (table[i].status == 0) continue;

        std::string fio_display = table[i].fio;
        if (fio_display.length() > FIO_WIDTH) {
            fio_display = fio_display.substr(0, FIO_WIDTH - 3) + "...";
        }

        std::string value_display = table[i].value;
        if (value_display.length() > VALUE_WIDTH) {
            value_display = value_display.substr(0, VALUE_WIDTH - 3) + "...";
        }

        std::cout << "│ " << std::setw(7) << logical_index++ << " │ "
                  << std::setw(HASH_WIDTH) << table[i].hash << " │ "
                  << std::setw(8) << status_str << " │ "
                  << std::setw(FIO_WIDTH) << std::left << fio_display << std::right << " │ "
                  << std::setw(ID_WIDTH) << table[i].id << " │ "
                  << std::setw(VALUE_WIDTH) << std::left << value_display << std::right << " │\n";
    }

    std::cout << "└─────────┴──────────────┴──────────┴──────────────────────┴──────────┴─────────────────────────┘\n";

    std::cout << "\n\033[1mСтатистика:\033[0m\n";
    std::cout << "┌──────────────────────┬──────────────┐\n";
    std::cout << "│ Всего ячеек          │ " << std::setw(12) << capacity << " │\n";
    std::cout << "├──────────────────────┼──────────────┤\n";
    std::cout << "│ Занято               │ " << std::setw(12) << active_count << " │\n";
    std::cout << "├──────────────────────┼──────────────┤\n";
    std::cout << "│ Удалено              │ " << std::setw(12) << deleted_count << " │\n";
    std::cout << "├──────────────────────┼──────────────┤\n";
    std::cout << "│ Свободно             │ " << std::setw(12) << empty_count << " │\n";
    std::cout << "├──────────────────────┼──────────────┤\n";
    std::cout << "│ Заполнение           │ " << std::setw(11) << std::fixed << std::setprecision(1)
              << (active_count * 100.0 / capacity) << "% │\n";
    std::cout << "└──────────────────────┴──────────────┘\n";
}


void save_to_file(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Ошибка открытия файла для записи\n";
        return;
    }

    for (int i = 0; i < capacity; i++) {
        if (table[i].status == 1) {
            out << table[i].value << "\n";
        }
    }

    out.close();
}
void save_report(const std::string& filename, std::pair<int, int> search_info) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Ошибка открытия файла отчёта\n";
        return;
    }

    out << "┌─────────┬────────┬──────────┬──────────────────────┬──────────┬─────────────────────────┐\n";
    out << "│ Индекс  │ Хэш    │ Статус   │ ФИО                  │ ID       │ Значение                │\n";
    out << "├─────────┼────────┼──────────┼──────────────────────┼──────────┼─────────────────────────┤\n";

    for (int i = 0; i < capacity; i++) {
        if (table[i].status == 0) continue;

        std::string status_str;
        switch (table[i].status) {
            case 1: status_str = "Занято  "; break;
            case 2: status_str = "Удалено"; break;
            default: status_str = "Пусто  ";
        }

        std::string fio_display = table[i].fio;
        if (fio_display.length() > 20) fio_display = fio_display.substr(0, 17) + "...";

        std::string value_display = table[i].value;
        if (value_display.length() > 25) value_display = value_display.substr(0, 22) + "...";

        out << "│ " << std::setw(7) << i << " │ "
            << std::setw(6) << table[i].hash << " │ "
            << std::setw(8) << status_str << " │ "
            << std::setw(20) << std::left << fio_display << std::right << " │ "
            << std::setw(8) << table[i].id << " │ "
            << std::setw(25) << std::left << value_display << std::right << " │\n";
    }

    out << "└─────────┴────────┴──────────┴──────────────────────┴──────────┴─────────────────────────┘\n";

    out << "\nРезультаты поиска:\n";
    if (search_info.first != -1) {
        out << "Найдено: номер строки = " << search_info.first
            << ", шагов = " << search_info.second << "\n";
    } else {
        out << "Ключ не найден. Шагов = " << search_info.second << "\n";
    }

    out.close();
}



};
int main() {
    int size_of_table;
    std::cout << "Введите размер таблицы: ";
    std::cin >> size_of_table;

    hastable ht(size_of_table);

    GenerateData("Data.txt", size_of_table);
    std::ifstream in("Data.txt");

    if (!in.is_open()) {
        std::cout << "Не удалось открыть файл\n";
        return 1;
    }

    std::string line;
    int lineno = 1;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            ht.insert(line, lineno);
        }
        lineno++;
    }

    ht.save_to_file("Output.txt");

    in.close();
    int n;
    std::cout << "Сколько строк удалить? Введите n: ";
    std::cin >> n;
    std::cin.ignore();

    for (int i = 0; i < n; ++i) {
        std::string key_line;
        std::cout << "Введите строку #" << (i + 1) << " для удаления:\n";
        std::getline(std::cin, key_line);
        ht.remove(key_line);
    }

    std::string search_key;
    std::cout << "Введите строку для поиска:\n";
    std::getline(std::cin, search_key);

    auto [found_line, steps] = ht.search(search_key);
    if (found_line != -1) {
        std::cout << "Найдено! Номер строки: " << found_line << ", шагов: " << steps << "\n";
    } else {
        std::cout << "Ключ не найден. Шагов: " << steps << "\n";
    }

    ht.print();
    ht.save_report("Report.txt", {found_line, steps});

    return 0;
}
