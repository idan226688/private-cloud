CC = g++

CFLAGS = -std=c++17 -pedantic-errors -Wall -I include/
CPPFLAGS = -Iinclude -I../framework/include/ 
DEBUG_FLAGS = -Wextra -g
RELEASE_FLAGS = -Wextra -DNDEBUG -o3

SRC_DIR = ./src
TEST_DIR = ./test
HEADER_DIR = ./include

SRCs = $(wildcard $(SRC_DIR)/*.cpp)
SRC_OBJs = $(notdir $(SRCs:.cpp=.o))

TEST_SRCS = $(wildcard $(TEST_DIR)/*_test.cpp)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

EXEs = $(notdir $(TEST_SRCS:_test.cpp=.out))
DS_NAMES = $(notdir $(SRCs:.cpp=))

LIBs = $(notdir $(SRCs:.cpp=.so))
LIB_NAMES_DEBUG = $(patsubst %.so,lib%_debug.so,$(LIBs))
LIB_NAMES_RELEASE = $(patsubst %.so,lib%_release.so,$(LIBs))

LDFLAGS = -L.

PREFIX = 

.PHONY : all debug release clean
# not to delete in makefile process
.PRECIOUS: $(LIB_NAMES_DEBUG) $(LIB_NAMES_RELEASE) $(EXEs)


all_%: debug_% release_%;

test: $(SRCs) $(TEST_SRCS)
	$(foreach ds,$(DS_NAMES),$(MAKE) $(ds).out;)

lib%_debug.so: $(SRC_DIR)/%.cpp $(HEADER_DIR)/%.hpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(CPPFLAGS) -shared -fpic $< -o $@ 

lib%_release.so: $(SRC_DIR)/%.cpp $(HEADER_DIR)/%.hpp
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $(CPPFLAGS) -shared -fpic $< -o $@ 

%.out: $(TEST_DIR)/%_test.cpp lib%_debug.so
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUG_FLAGS) -Wl,-rpath=. $< -o $@ $(LDFLAGS) $(filter %.so,$^) 

libnbd_proxy_debug.so: src/minion_proxy.cpp src/commands.cpp src/nbd_proxy.cpp ../framework/src/framework.cpp ../framework/src/dir_monitor.cpp ../framework/src/scheduler.cpp  ../framework/src/async_injection.cpp  ../framework/src/reactor.cpp ../framework/src/thread_pool.cpp src/response_manager.cpp ../framework/src/logger.cpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(CPPFLAGS) -shared -fpic $^ -o $@ 

libminion_proxy_debug.so: src/minion_proxy.cpp src/nbd_proxy.cpp ../framework/src/scheduler.cpp ../framework/src/async_injection.cpp ../framework/src/framework.cpp src/response_manager.cpp src/commands.cpp  ../framework/src/reactor.cpp ../framework/src/thread_pool.cpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(CPPFLAGS) -shared -fpic $^ -o $@ 

libmaster_debug.so: src/minion_proxy.cpp src/raid_manager.cpp src/commands.cpp src/nbd_proxy.cpp ../framework/src/framework.cpp ../framework/src/dir_monitor.cpp ../framework/src/scheduler.cpp  ../framework/src/async_injection.cpp  ../framework/src/reactor.cpp ../framework/src/thread_pool.cpp src/response_manager.cpp ../framework/src/logger.cpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(CPPFLAGS) -shared -fpic $^ -o $@ 

libminion_debug.so: src/minion_proxy.cpp ../framework/src/scheduler.cpp ../framework/src/async_injection.cpp ../framework/src/framework.cpp src/response_manager.cpp src/commands.cpp  ../framework/src/reactor.cpp ../framework/src/thread_pool.cpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(CPPFLAGS) -shared -fpic $^ -o $@ 


# adding Target-specific Variable Values
debug_%: lib%_debug.so;
	
release_%: lib%_release.so;


clean:
	rm -f $(EXEs) $(TEST_OBJS) $(SRC_OBJs) $(LIB_NAMES_DEBUG) $(LIB_NAMES_RELEASE)
