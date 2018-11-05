# probe
C++ type to aid unit testing of memory leaks/moves/copies inside of containers

# Example
This shows how the probe is used to inspect what an std::vector is doing to the contents

```cpp
void probe_test()
{
    using probe = probe_t<int>; //we want to use it with ints as underlying values

    probe::reset(); //resets all metrics that is stored in class static storage

    stc::static_vector<probe, 100> numbers;

    numbers.push_back({});

    probe_counts c = probe::last(); //returns all counts of events happened since last time last()/reset() was called

    ASSERT(c.copies == 0); //that should not have invoked copies
    ASSERT(c.moves > 0); //that should have invoked moves

    //print all events as text
    std::cout << "after push_back:\n" + to_string(c) << "\n\n";

    numbers.emplace_back();
    probe p = std::move(numbers[0]);

    probe_state s = probe::state();
    ASSERT(s.existing == 2); //2 instances should be existing at this point (has active lifetime. ends with destructor)
    ASSERT(s.active == 1); //1 instance should be active at this point (holds meaningful value. ends by being moved from, or destructed)

    c = probe::last();

    std::cout << "after emplace_back:\n" + to_string(c) << "\n\n";

    numbers.insert(numbers.begin(), probe{});

    c = probe::last();
    std::cout << "after insert:\n" + to_string(c) << "\n\n";

    numbers.pop_back();

    c = probe::last();

    ASSERT(c.destr == 1); //should have called 1 destructor

    std::cout << "after pop_back:\n" + to_string(c) << "\n\n";

    numbers.clear();

    c = probe::last();
    std::cout << "after clear:\n" + to_string(c) << "\n\n";

    ASSERT(probe::clean()); //clean returns true if there are exactly 0 `existing` and `active` items. i.e. no leaks nor double deletes

    //prints a summary of the current state
    std::cout << probe::report() << "\n";
}
```
