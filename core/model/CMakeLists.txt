add_subdirectory(objects)

ADD_LIBRARY(transaction STATIC
  transaction.cpp
  objects/asset.cpp
  objects/object.cpp
  commands/command.cpp
)

target_link_libraries(transaction
  datetime
  hash
  logger
)
