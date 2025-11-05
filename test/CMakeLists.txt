# Download the testing framework                                                
fetch_external_module(
    Catch2
    GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
    GIT_TAG         ee1450f268dfd5c13aa8670ba97e93cabaf2e15d #v2.x
)

# Define the test                                                               
add_langulus_test(LangulusLiteralTest
    SOURCES		main.cpp 
                test_literal_t.cpp
    LIBRARIES	Catch2 LangulusLiteral
)

target_compile_definitions(LangulusLiteralTest PRIVATE LANGULUS_OPTION_TESTING)
