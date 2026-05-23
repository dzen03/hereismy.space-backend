#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Server.h"
#include "challenge/Challenge.h"
#include "common/IDatabase.h"
#include "frontend/Frontend.h"
#include "portfolio/Portfolio.h"

auto main() -> int {
  try {
    // std::string categories_json;
    // TODO(dzen) add caching for requests

    auto modules = std::vector<std::unique_ptr<IDatabase>>();
    modules.emplace_back(std::make_unique<Frontend>());
    modules.emplace_back(std::make_unique<Portfolio>());
    modules.emplace_back(std::make_unique<Challenge>());

    for (const auto& module : modules) {
      module->parse_db();
    }

    static constexpr int DEFAULT_PORT = 8765;
    simple_http_server::Server server("0.0.0.0", DEFAULT_PORT);

    for (const auto& module : modules) {
      module->map_urls(server);
    }

    server.Start();

  } catch (std::exception& exception) {
    std::cerr << exception.what() << "\n";
    return 1;
  }

  return 0;
}
