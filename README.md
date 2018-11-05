# probe
C++ type to aid unit testing of memory leaks/moves/copies inside of containers

For full API info, check the source file.

# Example
This shows how the probe is used to inspect what an std::vector is doing to the contents

```cpp
#include "include/probe.hpp"
#include <vector>
#include <iostream>
#include <cassert>

int main()
{
    using probe = pr::probe_t<int>; //we want to use it with ints as underlying values

    probe::reset(); //resets all metrics that is stored in class static storage

    std::vector<probe> numbers;

    numbers.push_back({});

    assert(numbers.back() == 0); //can be converted back to value

    pr::probe_counts c = probe::last(); //returns all counts of events happened since last time last()/reset() was called

    assert(c.copies == 0); //that should not have invoked copies
    assert(c.moves > 0); //that should have invoked moves

    //print all events as text
    std::cout << "after push_back:\n" + to_string(c) << "\n\n";

    numbers.emplace_back();

    {
        probe p = std::move(numbers[0]);
    }

    pr::probe_state s = probe::state();
    assert(s.existing == 2); //2 instances should be existing at this point (has active lifetime. ends with destructor)
    assert(s.active == 1); //1 instance should be active at this point (holds meaningful value. ends by being moved from, or destructed)

    c = probe::last();

    std::cout << "after emplace_back:\n" + to_string(c) << "\n\n";

    numbers.insert(numbers.begin(), probe{});

    c = probe::last();
    std::cout << "after insert:\n" + to_string(c) << "\n\n";

    numbers.pop_back();

    c = probe::last();

    assert(c.destr == 1); //should have called 1 destructor

    std::cout << "after pop_back:\n" + to_string(c) << "\n\n";

    numbers.clear();

    c = probe::last();
    std::cout << "after clear:\n" + to_string(c) << "\n\n";

    assert(probe::clean()); //clean returns true if there are exactly 0 `existing` and `active` items. i.e. no leaks nor double deletes

    //prints a summary of the current state
    std::cout << "final report:\n" << probe::report() << "\n";
}
```

## Possible output

```
after push_back:
-calls-
 default constructor: 1
 value constructor:   0
 copy constructor:    0
 move constructor:    1
 copy assignment:     0
 move assignment:     0
 destructor:          1
-semantics-
 copies:              0
 moves:               1
-min/max-
 min active:          0
 max active:          1
 min existing:        0
 max existing:        2

after emplace_back:
-calls-
 default constructor: 1
 value constructor:   0
 copy constructor:    0
 move constructor:    2
 copy assignment:     0
 move assignment:     0
 destructor:          2
-semantics-
 copies:              0
 moves:               2
-min/max-
 min active:          0
 max active:          2
 min existing:        0
 max existing:        3

after insert:
-calls-
 default constructor: 1
 value constructor:   0
 copy constructor:    0
 move constructor:    3
 copy assignment:     0
 move assignment:     0
 destructor:          3
-semantics-
 copies:              0
 moves:               3
-min/max-
 min active:          0
 max active:          2
 min existing:        0
 max existing:        6

after pop_back:
-calls-
 default constructor: 0
 value constructor:   0
 copy constructor:    0
 move constructor:    0
 copy assignment:     0
 move assignment:     0
 destructor:          1
-semantics-
 copies:              0
 moves:               0
-min/max-
 min active:          0
 max active:          1
 min existing:        0
 max existing:        2

after clear:
-calls-
 default constructor: 0
 value constructor:   0
 copy constructor:    0
 move constructor:    0
 copy assignment:     0
 move assignment:     0
 destructor:          2
-semantics-
 copies:              0
 moves:               0
-min/max-
 min active:          0
 max active:          0
 min existing:        0
 max existing:        1

final report:
===LAST===
-calls-
 default constructor: 0
 value constructor:   0
 copy constructor:    0
 move constructor:    0
 copy assignment:     0
 move assignment:     0
 destructor:          0
-semantics-
 copies:              0
 moves:               0
-min/max-
 min active:          0
 max active:          0
 min existing:        0
 max existing:        0
===TOTAL===
-calls-
 default constructor: 3
 value constructor:   0
 copy constructor:    0
 move constructor:    6
 copy assignment:     0
 move assignment:     0
 destructor:          9
-semantics-
 copies:              0
 moves:               6
-min/max-
 min active:          0
 max active:          2
 min existing:        0
 max existing:        6
===STATE===
active:   0
existing: 0
```
