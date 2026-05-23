#ifndef HERE_IS_MY_SPACE_365_365_H
#define HERE_IS_MY_SPACE_365_365_H

#include <map>
#include <string>
#include <unordered_set>
#include <chrono>

#include "common/IDatabase.h"

class Challenge : public IDatabase {
 public:
  void parse_db() override;
  void map_urls(simple_http_server::Server& server) override;

  struct Photo {
    std::string extension;  // path = id + extension or id + '.XX' + extension,
                            // where XX in ['sm', 'md']
    std::string author;
    std::string description;
    std::string metadata;  // in json format
    std::chrono::time_point<std::chrono::system_clock> time;
  };

 private:
  std::map<std::string, std::vector<std::string>> map_author_ids;
  std::unordered_map<std::string, Photo> map_id_photo;
  std::unordered_map<std::string, std::filesystem::path> map_id_path;
  std::unordered_set<std::string> authors;
};

#endif  // HERE_IS_MY_SPACE_365_365_H
