# Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g

# Targets
TARGETS = condition lock mutex rwlock sem threadDemo timedMutex

# Default target
all: $(TARGETS)

# Individual build targets
condition: condition.cpp
	$(CXX) $(CXXFLAGS) -o condition condition.cpp

lock: lock.cpp
	$(CXX) $(CXXFLAGS) -o lock lock.cpp

mutex: mutex.cpp
	$(CXX) $(CXXFLAGS) -o mutex mutex.cpp

rwlock: rwlock.cpp
	$(CXX) $(CXXFLAGS) -o rwlock rwlock.cpp

sem: sem.cpp
	$(CXX) $(CXXFLAGS) -o sem sem.cpp

threadDemo: threadDemo.cpp
	$(CXX) $(CXXFLAGS) -o threadDemo threadDemo.cpp

timedMutex: timedMutex.cpp
	$(CXX) $(CXXFLAGS) -o timedMutex timedMutex.cpp

# Clean target
clean:
	rm -f $(TARGETS)
