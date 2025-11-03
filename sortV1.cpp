#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
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

    const std::vector<Student>& get_students() const { return students; }
    std::vector<Student> take_students() { return std::move(students); }
};

// comparator by (m_surname, m_name)
static inline bool student_cmp(const Student& a, const Student& b) {
    if (a.m_surname < b.m_surname) return true;
    if (a.m_surname > b.m_surname) return false;
    return a.m_name < b.m_name;
}

// quicksort implementation (in-place)
static void quicksort_students(std::vector<Student> &arr, int l, int r, std::mt19937 &rng) {
    while (l < r) {
        std::uniform_int_distribution<int> d(l, r);
        Student pivot = arr[d(rng)]; // copy pivot
        int i = l, j = r;
        while (i <= j) {
            while (student_cmp(arr[i], pivot)) ++i;
            while (student_cmp(pivot, arr[j])) --j;
            if (i <= j) { std::swap(arr[i], arr[j]); ++i; --j; }
        }
        // recurse smaller part first to limit stack
        if (j - l < r - i) {
            if (l < j) quicksort_students(arr, l, j, rng);
            l = i;
        } else {
            if (i < r) quicksort_students(arr, i, r, rng);
            r = j;
        }
    }
}

static void write_csv(const char* out, const std::vector<Student>& v) {
    std::ofstream ofs(out);
    ofs << "name,surname,email,year,month,day,group,rating,phone\n";
    for (const auto &s : v) {
        ofs << s.m_name << "," << s.m_surname << "," << s.m_email << ","
            << s.m_birth_year << "," << s.m_birth_month << "," << s.m_birth_day << ","
            << s.m_group << "," << s.m_rating << "," << s.m_phone_number << "\n";
    }
}

int main(int argc, char** argv) {
    const char* in = (argc > 1) ? argv[1] : "students.csv";
    StudentsDatabaseV1 db;
    if (!db.load_from_csv(in)) return 1;

    // prepare two copies
    std::vector<Student> v1 = db.take_students(); // move out original
    std::vector<Student> v2 = v1; // copy for second algorithm

    // std::sort timing
    auto t1 = std::chrono::steady_clock::now();
    std::sort(v1.begin(), v1.end(), student_cmp);
    auto t2 = std::chrono::steady_clock::now();
    auto dur_std = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    // quicksort timing
    std::mt19937 rng((unsigned)std::chrono::steady_clock::now().time_since_epoch().count());
    auto t3 = std::chrono::steady_clock::now();
    if (!v2.empty()) quicksort_students(v2, 0, (int)v2.size() - 1, rng);
    auto t4 = std::chrono::steady_clock::now();
    auto dur_qs = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

    // write outputs (not counted in timings)
    write_csv("students_sorted_std.csv", v1);
    write_csv("students_sorted_qs.csv", v2);

    // minimal report
    std::cout << "std::sort: " << dur_std << " us\n";
    std::cout << "quick sort: " << dur_qs << " us\n";
    return 0;
}