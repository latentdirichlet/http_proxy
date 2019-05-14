#include "server.hpp"
#include <iostream>

int main(){
  server server_(1234);
  server_.run();
  return 0;
}
