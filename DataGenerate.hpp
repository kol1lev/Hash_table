#ifndef DataGenerate_H
#define DataGenerate_H

#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#include <cstdlib>

std::string generateName()
{
    //std::vector<std::string> surnames = {"Voloshinskiy", "Ivanov", "Petrov", "Sidorov", "Komarov","Lebedev", "Smirnov", "Morozov", "Novikov", "Popov"};

    std::vector<std::string> surnames = {"Popov"};

    std::vector<std::string> names = {"Vladislav"};

    //std::vector<std::string> names = {"Rostislav", "Artem", "Daniil", "Vadim", "Oleg","Yuri", "Alexey", "Nikita", "Kirill", "Vladislav"};

    std::vector<std::string> patronymics = {"Andreevich"};

    //std::vector<std::string> patronymics = {"Anatolevich", "Ivanovich", "Petrovich", "Nikolaevich","Sergeevich", "Alexeevich", "Yurievich", "Andreevich"};

    return surnames[std::rand() % surnames.size()] + " " +
           names[std::rand() % names.size()] + " " +
           patronymics[std::rand() % patronymics.size()];
}

std::string generateCarModel()
{
    //std::vector<std::string> brands = {"Toyota", "BMW", "Mercedes", "Lada", "Ford","Audi", "Hyundai", "Kia", "Mazda", "Volkswagen"};
    std::vector<std::string> brands = {"Toyota"};

    //std::vector<std::string> models = {"Camry", "X5", "E200", "Granta", "Focus","A4", "Solaris", "Rio", "CX-5", "Polo", "Passat"};
    std::vector<std::string> models = {"Camry"};

    return brands[std::rand() % brands.size()] + " " +
           models[std::rand() % models.size()];
}

std::string generateDate()
{
    //int hour = std::rand() % 24;
    //int minute = std::rand() % 60;
    int hour = 16;
    int minute = 48;
    char buffer[6];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);

    return std::string(buffer);
}


void GenerateData(const std::string& filename, int n)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::ofstream out(filename);
    if (!out.is_open()) return;

    for (int line = 1; line <= n; ++line)
    {
        out << generateName() << " "
            << generateCarModel() << " "
            << generateDate() << " "
            << std::to_string(line) << "\n";
    }

    out.close();
}

#endif
