#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <iostream>
#include <utility>
#include <random>
#include <chrono>
#include <thread>
#include <functional>
#include <vector>
#include <memory>
#include <sstream>

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

struct GroupStats {
    int num = 0;
    float sum_rating = 0.0;
};

class StudentsDatabaseV1 {
private:
    std::unordered_map<std::string, Student*> email_access_db;
    std::unordered_map<std::string, GroupStats> groups;

public:

    bool load_from_csv(const char* path) {

        std::ifstream input_file{path};
        std::string line;

        if (!input_file.is_open()) {
            std::cerr << "Cannot open file: " << path << std::endl;
            return false;
        }

        email_access_db.clear();
        groups.clear();

        std::getline(input_file, line);

        while (std::getline(input_file, line)) {
            std::stringstream symbols(line);
            auto* stdnt = new Student;


            std::getline(symbols, stdnt->m_name, ',');
            std::getline(symbols, stdnt->m_surname, ',');
            std::getline(symbols, stdnt->m_email, ',');

            std::string val;
            std::getline(symbols, val, ',');
            stdnt->m_birth_year = std::stoi(val);
            std::getline(symbols, val, ',');
            stdnt->m_birth_month = std::stoi(val);
            std::getline(symbols, val, ',');
            stdnt->m_birth_day = std::stoi(val);

            std::getline(symbols, stdnt->m_group, ',');

            std::getline(symbols, val, ',');
            stdnt->m_rating = std::stof(val);

            auto &gs = groups[stdnt->m_group];
            gs.num += 1;
            gs.sum_rating += stdnt->m_rating;

            std::getline(symbols, stdnt->m_phone_number, ',');

            email_access_db[stdnt->m_email] = stdnt;
        }
        return true;
    }

    // (V4) 2.1
    // group with the most students
    std::string group_with_most_students() const {
        if (groups.empty())
            return {};

        std::string biggest_group;
        int biggest_num = 0;

        for (const auto& kv : groups) {
            if (kv.second.num > biggest_num) {
                biggest_num = kv.second.num;
                biggest_group = kv.first;
            }
        }
        return biggest_group;
    }

    /* (V4) 2.2
    if email and new_group are valid, changes the student group to new_group
    true - successfully, false - otherwise
    */
    bool change_student_group(const std::string& email, const std::string& new_group) {
        auto it = email_access_db.find(email);
        if (it == email_access_db.end()) return false;

        Student* student = it->second;
        
        const std::string old_group = student->m_group;
        if (old_group == new_group) return true;

        auto group_stat_it = groups.find(old_group);
        if (group_stat_it != groups.end()) {
            auto& group_stat = group_stat_it->second;
            group_stat.num -= 1;
            group_stat.sum_rating -= student->m_rating;
            if (group_stat.num <= 0) { //if its empty
                groups.erase(group_stat_it); //it erases
            }
        }

        // creates new group
        auto &new_group_stat = groups[new_group];
        new_group_stat.num += 1;
        new_group_stat.sum_rating += student->m_rating;

        student->m_group = new_group;
        return true;
    }

    // (V4) 2.3
    // returns group with the highest average rating
    std::pair<std::string, float> group_with_highest_average() const {
        if (groups.empty()) return {"", 0.0f}; //if it's empty

        std::string best_group = "";
        float best_avg = 0.0f;

        for (const auto& kv : groups) {
            const GroupStats& group_stat = kv.second;

            float avg = group_stat.sum_rating
                    / (float)group_stat.num;
            if (avg > best_avg) {
                best_avg = avg;
                best_group = kv.first;
            }
        }
        return {best_group, best_avg};
    }

    size_t size() const { return email_access_db.size(); }

    ~StudentsDatabaseV1() {
        for (auto& kv : email_access_db) {
            delete kv.second;
        }
    }
};

void run_on_csv_files(const std::vector<std::string>& files) {
    std::mt19937_64 rng((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> op(0,11); // 10:1:1 -> 0..11


    std::cout << "file | operation1_count | operation3_count\n";


    for (const auto &file : files) {
        StudentsDatabaseV1 db;
        db.load_from_csv(file.c_str());

        uint64_t counter1 = 0, counter3 = 0;
        auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(10);

        while (std::chrono::steady_clock::now() < end_time) {
            for (int counter = 0; counter < 12; ++counter) {
                if (counter < 10) {
                    db.group_with_most_students();
                    ++counter1;
                } else {
                    db.group_with_highest_average();
                    ++counter3;
                }
            }
        }

        std::cout << file << " | " << counter1 << " | " << counter3 << "\n";
    }
}

int main()
{
    std::vector<std::string> files = {
        "databases/students_100.csv",
        "databases/students_1000.csv",
        "databases/students_10000.csv",
        "databases/students.csv"
    };

    run_on_csv_files(files);
    return 0;
}