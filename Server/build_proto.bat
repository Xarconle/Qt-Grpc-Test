if not exist generated mkdir generated
%1/protoc -I ../Proto --grpc_out=./generated --plugin=protoc-gen-grpc=%1/grpc_cpp_plugin.exe ../Proto/api.proto
%1/protoc -I ../Proto --cpp_out=./generated ../Proto/api.proto