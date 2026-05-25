#include "Challenge.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Directory.h"
#include "Request.h"
#include "Response.h"
#include "Server.h"

namespace {
const auto photos_path =
    std::filesystem::current_path() / "db" / "challenge" / "photos";
const auto metadata_path =
    std::filesystem::current_path() / "db" / "challenge" / "metadata";
}  // namespace

auto Challenge::get_author_name(const std::string& author_id) -> std::string {
  auto it = map_tgid_name.find(author_id);

  return (it != map_tgid_name.end()) ? it->second : "Unknown Author";
}
auto Challenge::get_author_id(const std::string& author_id) -> std::string {
  auto it = map_name_tgid.find(author_id);

  return (it != map_name_tgid.end()) ? it->second : "";
}

void Challenge::parse_db() {
  std::map<std::string, std::vector<std::string>> map_author_ids_local;
  std::unordered_map<std::string, Challenge::Photo> map_id_photo_local;
  std::unordered_map<std::string, std::filesystem::path> map_id_path_local;
  std::unordered_set<std::string> authors_local;

  std::unordered_map<std::string, std::string> map_tgid_name_local;
  std::unordered_map<std::string, std::string> map_name_tgid_local;

  const auto tgid_map_path = metadata_path / ".tg_id.map";
  std::ifstream tgid_map_file(tgid_map_path);

  std::string line;
  while (std::getline(tgid_map_file, line)) {
    if (line.empty()) {
      continue;
    }

    size_t delim_pos = line.find('#');
    if (delim_pos != std::string::npos) {
      std::string key = line.substr(0, delim_pos);
      std::string val = line.substr(delim_pos + 1);

      map_name_tgid_local[val] = key;
      map_tgid_name_local[std::move(key)] = std::move(val);
    }
  }

  for (const auto& entry : std::filesystem::directory_iterator(photos_path)) {
    if (entry.path().stem().string().starts_with(".")) {
      continue;
    }

    map_id_path_local[entry.path().stem().string()] = entry;
  }

  for (const auto& entry : std::filesystem::directory_iterator(metadata_path)) {
    if (entry.path().extension().string() != ".txt") {  // db format: {id}.txt
      continue;
    }

    const auto& index = entry.path().stem().string();

    std::ifstream db_entry(entry.path());
    std::string unix_time;
    std::string author;
    std::string description;
    std::string json_metadata;

    std::getline(db_entry, unix_time);
    std::getline(db_entry, author);
    std::getline(db_entry, description);

    json_metadata.assign(std::istreambuf_iterator<char>(db_entry),
                         std::istreambuf_iterator<char>());

    authors_local.insert(author);
    map_author_ids_local[author].emplace_back(index);

    map_id_photo_local[index] = {
        .extension = map_id_path_local.at(index).extension().string(),
        .author = std::move(author),
        .description = std::move(description),
        .metadata = std::move(json_metadata),
        .unix_time = std::move(unix_time)};

    map_author_ids_local[""].emplace_back(index);
  }

  std::scoped_lock lock(update_mutex);
  std::swap(this->authors, authors_local);
  std::swap(this->map_author_ids, map_author_ids_local);
  std::swap(this->map_id_photo, map_id_photo_local);
  std::swap(this->map_id_path, map_id_path_local);
  std::swap(this->map_tgid_name, map_tgid_name_local);
  std::swap(this->map_name_tgid, map_name_tgid_local);
}

void Challenge::output_photo(const std::string& index,
                             const Challenge::Photo& photo,
                             std::ostringstream& body) {
  body << R"({"id":")" << index << R"(","author":")"
       << get_author_name(photo.author) << R"(","description":")"
       << photo.description << R"(","metadata":)" << photo.metadata << "}";
};

void Challenge::map_urls(simple_http_server::Server& server) {
  server.MapDirectory("/365photos",
                      simple_http_server::Directory(
                          photos_path,
                          {{"Access-Control-Allow-Origin", "*"},
                           {"Access-Control-Allow-Headers", "*"}},
                          simple_http_server::Directory::AllowType::WHITELIST,
                          {std::regex("^.*\\.(jpg|webp)$")}));

  server.MapUrl("/api/365/authors",
                [this](const simple_http_server::Request& request) -> auto {
                  std::ostringstream body;

                  body << "{\"authors\":[";

                  if (!authors.empty()) {
                    auto author_it = authors.begin();

                    body << '"' << get_author_name(*author_it) << '"';

                    for (++author_it; author_it != authors.end(); ++author_it) {
                      body << ",\"" << get_author_name(*author_it) << '\"';
                    }
                  }

                  body << "]}";

                  static constexpr int OK_CODE = 200;
                  return simple_http_server::Response(
                      OK_CODE, body.str(),
                      {{"Content-Type", "application/json; charset = utf-8"},
                       {"Access-Control-Allow-Origin", "*"},
                       {"Access-Control-Allow-Headers", "*"}});
                });

  server.MapUrl(
      "/api/365/photos",
      [this](const simple_http_server::Request& request) -> auto {
        const auto& arguments = request.GetArguments();

        std::ostringstream body;

        body << "{\"photos\":[";

        std::string name;
        if (arguments.contains("category")) {
          name = arguments.at("category");
        }

        if (map_author_ids.contains(get_author_id(name))) {
          const auto& photos = map_author_ids.at(name);
          auto category_it = photos.begin();

          output_photo(*category_it, map_id_photo.at(*category_it), body);

          for (++category_it; category_it != photos.end(); ++category_it) {
            body << ',';
            output_photo(*category_it, map_id_photo.at(*category_it), body);
          }
        }

        body << "]}";

        static constexpr int OK_CODE = 200;
        return simple_http_server::Response(
            OK_CODE, body.str(),
            {{"Content-Type", "application/json; charset = utf-8"},
             {"Access-Control-Allow-Origin", "*"},
             {"Access-Control-Allow-Headers", "*"}});
      });

  server.MapUrl(
      "/api/365/photo",
      [this](const simple_http_server::Request& request) -> auto {
        std::ostringstream body;

        const auto& arguments = request.GetArguments();

        std::string index;
        if (!arguments.contains("id") ||
            !map_id_photo.contains(arguments.at("id"))) {
          if (request.GetType() == simple_http_server::Request::Type::POST) {
            // TODO(dzen) implement photo upload
            static constexpr int CREATED_CODE = 201;
            return simple_http_server::Response(
                CREATED_CODE, "",
                {{"Access-Control-Allow-Origin", "*"},
                 {"Access-Control-Allow-Headers", "*"}},
                "Created");
          }

          static constexpr int NOT_FOUND_CODE = 404;
          return simple_http_server::Response(
              NOT_FOUND_CODE, "",
              {{"Access-Control-Allow-Origin", "*"},
               {"Access-Control-Allow-Headers", "*"}});
        }
        index = arguments.at("id");

        const auto& photo = map_id_photo.at(index);

        body << "{\"photo\":";

        output_photo(index, photo, body);
        body << '}';

        static constexpr int OK_CODE = 200;
        return simple_http_server::Response(
            OK_CODE, body.str(),
            {{"Content-Type", "application/json; charset = utf-8"},
             {"Access-Control-Allow-Origin", "*"},
             {"Access-Control-Allow-Headers", "*"}});
      });

  server.MapUrl("/api/365/reload",
                [this](const simple_http_server::Request& request) -> auto {
                  parse_db();

                  static constexpr int OK_CODE = 200;
                  return simple_http_server::Response(
                      OK_CODE, "",
                      {{"Content-Type", "application/json; charset = utf-8"},
                       {"Access-Control-Allow-Origin", "*"},
                       {"Access-Control-Allow-Headers", "*"}});
                });
}
