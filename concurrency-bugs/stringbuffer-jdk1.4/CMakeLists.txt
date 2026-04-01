set(STRINGBUFFER_CASE_DIR "${jcbugs_SOURCE_DIR}/stringbuffer-jdk1.4")

add_executable(stringbuffer-jdk1.4 "${STRINGBUFFER_CASE_DIR}/main.cpp"
                                   "${STRINGBUFFER_CASE_DIR}/stringbuffer.cpp")
target_link_libraries(stringbuffer-jdk1.4 PRIVATE Threads::Threads)
configure_bug_target(stringbuffer-jdk1.4)
