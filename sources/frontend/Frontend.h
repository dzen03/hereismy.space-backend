#ifndef HERE_IS_MY_SPACE_FRONTEND_FRONTEND_H
#define HERE_IS_MY_SPACE_FRONTEND_FRONTEND_H

#include <map>
#include <unordered_set>

#include "common/IDatabase.h"

class Frontend : public IDatabase {
 public:
  Frontend() = default;
  void parse_db() override {};
  void map_urls(simple_http_server::Server& server) override;
};

#endif  // HERE_IS_MY_SPACE_FRONTEND_FRONTEND_H
