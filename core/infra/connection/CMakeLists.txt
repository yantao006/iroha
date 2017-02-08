
SET(CMAKE_CXX_FLAGS "-std=c++14 -L/usr/local/lib `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl")
SET(CRYPTO_PATH  "${PROJECT_SOURCE_DIR}/core/crypto")


link_directories(
  ${CRYPTO_PATH}
  /usr/local/lib
)


ADD_LIBRARY(connection_with_grpc_flatbuf STATIC
  connection_with_grpc_flatbuf.cpp
  $ENV{IROHA_HOME}/flatbuf/api.grpc.fb.cc
)
target_link_libraries(connection_with_grpc_flatbuf
  grpc++
  gpr
  pthread
peer_service_with_json
  peer_service_with_json
  transaction
)
