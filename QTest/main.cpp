#include <QtCore/QCoreApplication>
#include <QDebug>
#include <format>
#include <iostream>
#include <locale.h>
#include <chrono>

class Person
{
public:
    Person() = delete;
    Person(unsigned long long id, const std::string& firstName, const std::string& lastName) noexcept
        : _id(id), _firstName(firstName), _lastName(lastName) {}

    auto getId() const noexcept -> unsigned long long
    {
        return _id;
    }
    auto getFirstName() const noexcept -> const std::string&
    {
        return _firstName;
    }
    auto getLastName() const noexcept -> const std::string&
    {
        return _lastName;
    }

private:
    unsigned long long _id;
    std::string _firstName;
    std::string _lastName;
};

template<>
class std::formatter<Person>
{
public:
    constexpr auto parse(auto& context)
    {

    }

    auto format(const Person& person, auto& context)
    {

    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout << std::format("{:*<20}\n", "lefttt");
    std::cout << std::format("{:*>20}\n", "rightt");
    std::cout << std::format("{:*^20}\n", "center");
    std::cout << std::format("{:*^20}\n", "centered");

    std::cout << std::format("{:<5}\n",   42);
    std::cout << std::format("{:<+5}\n",  42);
    std::cout << std::format("{:< 5}\n",  42);
    std::cout << std::format("{:< 5}\n",  -42);

    std::cout << std::format("{:15d}\n", 42);
    std::cout << std::format("{:15b}\n", 42);
    std::cout << std::format("{:#15b}\n", 42);
    std::cout << std::format("{:15X}\n", 42);
    std::cout << std::format("{:#15X}\n", 42);

    const double pi{ 3.1415 };
    const int precision{ 5 };
    const int width{ 15 };
    std::cout << std::format("{:15.2f}\n", pi);
    std::cout << std::format("{:15.{}f}\n", pi, precision);
    std::cout << std::format("{:{}.{}f}\n", pi, width, precision);
    std::cout << std::format("{0:{1}.{2}f}\n", pi, width, precision);

    std::cout << std::format(std::locale("en_US.UTF-8"), "{:L}\n", 1024);
    std::cout << std::format(std::locale("zh_CN.UTF-8"), "{:L}\n", 1024);
    std::cout << std::format(std::locale("de_DE.UTF-8"), "{:L}\n", 1024);

    std::cout << std::format("{:%d.%m.%Y}.", std::chrono::system_clock::now()) << "\n";

    Person p(11, "Hello", "World");
    std::cout << std::format("{}", p);
    return a.exec();
}
