#include <exception>
#include <iostream>
#include <string>

#include "Portfolio.h"
#include "Server.h"

auto main() -> int {
  try {
    // std::string categories_json;
    // TODO(dzen) add caching for requests

    auto portfolio = Portfolio();
    portfolio.parse_db();

    static constexpr int DEFAULT_PORT = 8765;
    simple_http_server::Server server("0.0.0.0", DEFAULT_PORT);

    portfolio.map_urls(server);

    server.Start();

  } catch (std::exception& exception) {
    std::cerr << exception.what() << "\n";
    return 1;
  }

  return 0;
}
