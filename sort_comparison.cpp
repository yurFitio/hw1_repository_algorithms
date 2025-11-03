#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>

struct Student {
    std::string m_name; // ім'я українською
    std::string m_surname; // прізвище українською
    std::string m_email; // [a-z.]*@student.org
    int m_birth_year; // 1950..2010
    int m_birth_month; // 1..12
    int m_birth_day; // 1..Number of days in m_birth_month
    std::string m_group; // [A-Z][A-Z][A-Z]-[0-9][0-9]
    float m_rating; // 0..100
    std::string m_phone_number; // 38(0xx)xx-xx-xxx
};

class StudentsDatabaseV1 {
private:
    std::vector<Student> students;

public:
    bool load_from_csv(const char* path) {
        std::ifstream input_file{path};
        std::string line;

        if (!input_file.is_open()) {
            std::cerr << "Cannot open file: " << path << std::endl;
            return false;
        }

        students.clear();

        std::getline(input_file, line);

        while (std::getline(input_file, line)) {
            std::stringstream symbols(line);
            Student stdnt;

            std::getline(symbols, stdnt.m_name, ',');
            std::getline(symbols, stdnt.m_surname, ',');
            std::getline(symbols, stdnt.m_email, ',');

            std::string val;
            std::getline(symbols, val, ',');
            stdnt.m_birth_year = std::stoi(val);
            std::getline(symbols, val, ',');
            stdnt.m_birth_month = std::stoi(val);
            std::getline(symbols, val, ',');
            stdnt.m_birth_day = std::stoi(val);

            std::getline(symbols, stdnt.m_group, ',');

            std::getline(symbols, val, ',');
            stdnt.m_rating = std::stof(val);


            std::getline(symbols, stdnt.m_phone_number, ',');

            students.push_back(stdnt);
        }
        return true;
    }

    std::vector<Student> take_students() { return std::move(students); }
};

bool student_cmp(const Student& a, const Student& b) {
    if (a.m_surname < b.m_surname) return true;
    if (a.m_surname > b.m_surname) return false;
    return a.m_name < b.m_name;
}

void quicksort_students(std::vector<Student> &arr, int l, int r) {
    while (l < r) {
        Student pivot = arr[l];
        int i = l, j = r;
        while (i <= j) {
            while (student_cmp(arr[i], pivot)) ++i;
            while (student_cmp(pivot, arr[j])) --j;
            if (i <= j) { std::swap(arr[i], arr[j]); ++i; --j; }
        }
        if (j - l < r - i) {
            if (l < j) quicksort_students(arr, l, j);
            l = i;
        } else {
            if (i < r) quicksort_students(arr, i, r);
            r = j;
        }
    }
}

void write_csv(const char* out, const std::vector<Student>& v) {
    std::ofstream ofs(out);
    ofs << "m_name,m_surname,m_email,m_birth_year,m_birth_month,m_birth_day,m_group,m_rating,m_phone_number\n";
    for (const auto &s : v) {
        ofs << s.m_name << "," << s.m_surname << "," << s.m_email << ","
            << s.m_birth_year << "," << s.m_birth_month << "," << s.m_birth_day << ","
            << s.m_group << "," << s.m_rating << "," << s.m_phone_number << "\n";
    }
}

int main() {
    const char* in = "databases/students.csv";
    StudentsDatabaseV1 db;
    if (!db.load_from_csv(in)) return 1;

    std::vector<Student> v1 = db.take_students();
    std::vector<Student> v2 = v1;

    auto t1 = std::chrono::steady_clock::now();
    std::sort(v1.begin(), v1.end(), student_cmp);
    auto t2 = std::chrono::steady_clock::now();
    auto dur_std = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    auto t3 = std::chrono::steady_clock::now();
    if (!v2.empty()) quicksort_students(v2, 0, (int)v2.size() - 1);
    auto t4 = std::chrono::steady_clock::now();
    auto dur_qs = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

    write_csv("students_sorted_std.csv", v1);
    write_csv("students_sorted_qs.csv", v2);

    std::cout << "std::sort: " << dur_std << " us\n";
    std::cout << "quick sort: " << dur_qs << " us\n";
    return 0;
}