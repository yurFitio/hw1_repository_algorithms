#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <utility>
#include <random>
#include <chrono>
#include <thread>
#include <vector>

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

class StudentsDatabaseV2 {
public:
    std::vector<Student> students;
    std::unordered_map<std::string, size_t> email_to_index;
    std::unordered_map<std::string, std::vector<size_t>> group_to_students;

    std::string cached_max_group;
    std::pair<std::string, float> cached_best_rating_group;

    StudentsDatabaseV2() = default;

    bool load_from_csv(const char* path) {
        std::ifstream f(path);
        if (!f.is_open()) return false;

        students.clear();
        email_to_index.clear();
        group_to_students.clear();
        cached_max_group.clear();
        cached_best_rating_group = {"", 0.0f};
        index_pos_in_group.clear();

        std::string line;
        std::getline(f, line);

        while (std::getline(f, line)) {
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

            size_t idx = students.size();
            students.push_back(std::move(stdnt));
            email_to_index[students[idx].m_email] = idx;
            auto &vec = group_to_students[students[idx].m_group];
            vec.push_back(idx);
            index_pos_in_group[idx] = vec.size() - 1;
        }

        recompute_caches();
        return true;
    }

    // (V4) 2.1
    // group with the most students
    std::string group_with_most_students() const {
        return cached_max_group;
    }

    /* (V4) 2.2
    if email and new_group are valid, changes the student group to new_group
    true - successfully, false - otherwise
    */
    bool change_student_group(const std::string& email, const std::string& new_group) {
        auto it = email_to_index.find(email);
        if (it == email_to_index.end()) return false;
        size_t idx = it->second;
        if (idx >= students.size()) return false;

        const std::string old_group = students[idx].m_group;
        if (old_group == new_group) return true;

        // remove idx from old_group vector using stored position
        auto git = group_to_students.find(old_group);
        if (git != group_to_students.end()) {
            auto &vec = git->second;
            auto pos_it = index_pos_in_group.find(idx);
            if (pos_it != index_pos_in_group.end()) {
                size_t pos = pos_it->second;
                size_t last = vec.back();
                vec[pos] = last;
                index_pos_in_group[last] = pos;
                vec.pop_back();
                index_pos_in_group.erase(pos_it);
            } else {
                // fallback linear removal (should not happen)
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (vec[i] == idx) {
                        size_t last = vec.back();
                        vec[i] = last;
                        index_pos_in_group[last] = i;
                        vec.pop_back();
                        break;
                    }
                }
            }
            if (vec.empty()) group_to_students.erase(git);
        }

        // add idx to new_group
        auto &nvec = group_to_students[new_group];
        nvec.push_back(idx);
        index_pos_in_group[idx] = nvec.size() - 1;

        students[idx].m_group = new_group;

        // update caches (recompute simple and safe)
        recompute_caches();
        return true;
    }

    // (V4) 2.3
    // returns group with the highest average rating
    std::pair<std::string, float> group_with_highest_average() const {
        return cached_best_rating_group;
    }

    // повертає випадковий email (порожній коли немає)
    std::string random_email(std::mt19937_64 &rng) const {
        if (students.empty()) return {};
        std::uniform_int_distribution<size_t> di(0, students.size() - 1);
        return students[di(rng)].m_email;
    }

    size_t size() const { return students.size(); }

private:
    std::unordered_map<size_t, size_t> index_pos_in_group;

    void recompute_caches() {
        cached_max_group.clear();
        cached_best_rating_group = {"", 0.0f};

        size_t best_count = 0;
        float best_avg = -1.0f;
        for (const auto &kv : group_to_students) {
            const std::string &gname = kv.first;
            const std::vector<size_t> &vec = kv.second;
            if (vec.empty()) continue;
            size_t cnt = vec.size();
            double sum = 0.0;
            for (size_t idx : vec) sum += students[idx].m_rating;
            float avg = static_cast<float>(sum / cnt);

            if (cnt > best_count) {
                best_count = cnt;
                cached_max_group = gname;
            }
            if (avg > best_avg) {
                best_avg = avg;
                cached_best_rating_group = {gname, avg};
            }
        }
        if (best_avg < 0.0f) cached_best_rating_group = {"", 0.0f};
    }
};


// very small/simple benchmark runner (10:1:1)
static void run_on_csv_files_simple(const std::vector<std::string>& files) {
    std::mt19937_64 rng((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> op(0,11); // 10:1:1 -> 0..11

    std::cout << "file | operation1_count | operation2_count | operation3_count\n";

    for (const auto &file : files) {
        StudentsDatabaseV2 db;
        db.load_from_csv(file.c_str());

        uint64_t counter1 = 0, counter2 = 0, counter3 = 0;
        auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(10);

        while (std::chrono::steady_clock::now() < end_time) {
            for (int counter = 0; counter < 12; ++counter) {
                if (counter < 10) {
                    db.group_with_most_students();
                    ++counter1;
                } else if (counter == 10) {
                    db.change_student_group(db.random_email(rng), "ZZZ-99");
                    ++counter2;
                } else {
                    db.group_with_highest_average();
                    ++counter3;
                }
            }
        }

        std::cout << file << " | " << counter1 << " | " << counter2 << " | " << counter3 << "\n";
    }
}

int main() {
    std::vector<std::string> files = {
        "databases/students_100.csv",
        "databases/students_1000.csv",
        "databases/students_10000.csv",
        "databases/students.csv"
    };
    run_on_csv_files_simple(files);
    return 0;
}
