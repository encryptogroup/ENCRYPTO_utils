add_executable(test
	test_main.cpp
	test_cbitvector.cpp
)
target_link_libraries(test encrypto_utils gtest)
