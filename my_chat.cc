#include "my_chat.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage [addr] [port]" << std::endl;
    return -1;
  }
  Server server(argv[1], argv[2]);
  server.Run();
  return 0;
}
