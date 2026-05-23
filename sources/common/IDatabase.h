#ifndef HERE_IS_MY_SPACE_INCLUDES_IDATABASE_H
#define HERE_IS_MY_SPACE_INCLUDES_IDATABASE_H

#include "Server.h"

class IDatabase {
 public:
  virtual void parse_db() = 0;
  virtual void map_urls(simple_http_server::Server& server) = 0;
  IDatabase() = default;
  virtual ~IDatabase() = default;

  IDatabase(const IDatabase& source) = default;
  IDatabase(IDatabase&& source) = default;

  auto operator=(const IDatabase& source) -> IDatabase& = default;
  auto operator=(IDatabase&& source) -> IDatabase& = default;
};

#endif  // HERE_IS_MY_SPACE_INCLUDES_IDATABASE_H
