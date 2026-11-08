cmake_minimum_required(VERSION 3.20)
project(CreditRisk_MonteCarlo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(CreditRisk_MonteCarlo
    main.cpp

    src/Customer.cpp
    src/IndividualCustomer.cpp
    src/CorporateCustomer.cpp

    src/Loan.cpp
    src/ConsumerLoan.cpp
    src/CorporateLoan.cpp

    src/Portfolio.cpp
    src/PortfolioFilter.cpp
    src/PortfolioView.cpp
    src/CSVRepo.cpp

    src/MonteCarloSimulator.cpp
    src/SimulationResult.cpp
)

target_include_directories(CreditRisk_MonteCarlo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Use an absolute path to sample data, independent of CLion's working directory.
target_compile_definitions(CreditRisk_MonteCarlo PRIVATE
    PROJECT_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)
