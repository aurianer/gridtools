if(BUILD_TESTING)
    add_library(regression_main regression_fixture.cpp)
    target_link_libraries(regression_main gtest gridtools)
endif()
