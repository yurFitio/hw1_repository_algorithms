# Student Database Performance Comparison

## Variant V4 10;1:1

### Required Operations:
1) Find the group with the most students
2) Change a student's group by their email (m_email)
3) Find the group with the highest average student rating

### Usage

```bash
g++ realizationV1.cpp -o realizationV1 -O3 && ./realizationV1
```

```bash
g++ realizationV2.cpp -o realizationV2 -O3 && ./realizationV2
```

```bash
g++ realizationV3.cpp -o realizationV3 -O3 && ./realizationV3
```


Sort: I implemented quick sort
```bash
g++ sort_comparison.cpp -o sort_comparison && ./sort_comparison
```

## Implementation Approaches

### First Attempt (V3 - Initial)
Since operations 1 and 3 have very similar requirements in terms of the parameters they need, I tried using a hash table where I tracked the sum and count of students for each group. When a student changed groups, the value would be subtracted from the old group and added to the new one.

**Data structures:**
```cpp
std::unordered_map<std::string, GroupStats> groups;

struct GroupStats {
    int num = 0;
    float sum_rating = 0.0;
};
```

**Result:** The idea turned out to be somewhat unsuccessful (I still don't fully understand why).

### Second Attempt (V2 - Another very optimized approach)
I tried using different data structures with full caching:
```cpp
std::vector<Student> students;
std::unordered_map<std::string, size_t> email_to_index;
std::unordered_map<std::string, std::vector<size_t>> group_to_students;

std::string cached_max_group;
std::pair<std::string, float> cached_best_rating_group;
```

Perhaps because all groups are relatively small, the sum and average rating of a group can be quickly recalculated with this approach.

### Third Attempt (V1 - V3 With Caching)
I tried adding caching of the best result to my first algorithm. The speed increased by 9 times, but it was still slightly more than 2 times slower than the second algorithm.

**Added:**
```cpp
std::string cached_best_group;
float cached_best_avg = 0.0f;
```

## Performance Results

Performance test with 10:1:1:

V1: file | operation1_count | operation2_count | operation3_count
students_100.csv | 95879760 | 9587976 | 9587976
students_1000.csv | 91897740 | 9189774 | 9189774
students_10000.csv | 50304730 | 5030473 | 5030473
students.csv | 328090 | 32809 | 32809

students_100.csv | 204220930 | 20422093 | 20422093
students_1000.csv | 206595760 | 20659576 | 20659576
students_10000.csv | 178705940 | 17870594 | 17870594
students.csv | 47130 | 4713 | 4713

V3: file | operation1_count | operation3_count
students_100.csv | 11588278 | 2316815
students_1000.csv | 1512185 | 301674
students_10000.csv | 345411 | 69274
students.csv | 298676 | 59441


Sort comparison:
std::sort: 294666 us
quick sort: 344508 us

**Conclusion:** V2 with group-based indexing and full caching provides the best performance, approximately 2x faster than V1 with caching and 18x faster than the initial implementation.
# hw1_repository_algorithms
