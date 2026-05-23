#include <map>
#include <unordered_set>

#include "IDatabase.h"

class Portfolio : public IDatabase {
 public:
  void parse_db() override;
  void map_urls(simple_http_server::Server& server) override;

  struct Photo {
    std::string extension;  // path = id + extension or id + '.XX' + extension,
                            // where XX in ['sm', 'md']
    std::vector<std::string> categories;
    std::string description;
  };

 private:
  std::map<std::string, std::vector<std::string>> map_category_ids;
  std::unordered_map<std::string, Photo> map_id_photo;
  std::unordered_map<std::string, std::filesystem::path> map_id_path;
  std::unordered_set<std::string> categories;
};
