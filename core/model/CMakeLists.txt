<<<<<<< HEAD
add_subdirectory(commands)
=======
add_subdirectory(objects)
>>>>>>> feature/api

ADD_LIBRARY(transaction STATIC
  transaction.cpp
)

target_link_libraries(transaction
  objects
  datetime
  hash
  logger
)
