add_executable(stringbuffer-jdk1.4 main.cpp stringbuffer.cpp)
target_link_libraries(stringbuffer-jdk1.4 PRIVATE Threads::Threads)
configure_bug_target(stringbuffer-jdk1.4)
