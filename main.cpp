#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
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
struct Photo {
  std::string extension;  // path = id + extension or id + '.XX' + extension,
                          // where XX in ['sm', 'md']
  std::vector<std::string> categories;
  std::string description;
};

const auto photos_path = std::filesystem::current_path() / "db" / "photos";
const auto metadata_path = std::filesystem::current_path() / "db" / "metadata";
const auto staticfiles_path = std::filesystem::current_path() / "staticfiles";

void parse_db(
    std::map<std::string, std::vector<std::string>>& map_category_ids,
    std::unordered_map<std::string, Photo>& map_id_photo,
    std::unordered_map<std::string, std::filesystem::path>& map_id_path,
    std::unordered_set<std::string>& categories) {
  for (const auto& entry : std::filesystem::directory_iterator(photos_path)) {
    if (entry.path().stem().string().starts_with(".")) {
      continue;
    }

    map_id_path[entry.path().stem().string()] = entry;
  }

  for (const auto& entry : std::filesystem::directory_iterator(metadata_path)) {
    if (entry.path().extension().string() != ".txt") {  // db format: {id}.txt
      continue;
    }

    const auto& index = entry.path().stem().string();

    std::ifstream db_entry(entry.path().string());
    std::string categories_raw;
    std::string description;
    std::vector<std::string> categories_parsed;

    std::getline(db_entry, categories_raw);
    std::getline(db_entry, description);

    categories_raw.push_back('#');

    std::string::size_type pos = 0;
    std::string::size_type prev_pos = -1;
    std::string category;
    while ((pos = categories_raw.find('#', prev_pos + 1)) !=
           std::string::npos) {
      category = categories_raw.substr(prev_pos + 1, pos - prev_pos - 1);
      prev_pos = pos;

      map_category_ids[category].emplace_back(index);
      categories_parsed.emplace_back(category);

      categories.insert(category);
    }

    map_id_photo[index] = {
        .extension = map_id_path.at(index).extension().string(),
        .categories = std::move(categories_parsed),
        .description = std::move(description)};
    map_category_ids[""].emplace_back(index);
  }
}

void output_photo(const std::string& index, const Photo& photo,
                  std::ostringstream& body) {
  body << R"({"id":")" << index << R"(","description":")" << photo.description
       << R"(","categories":[)";
  if (!photo.categories.empty()) {
    auto category_it = photo.categories.begin();

    body << '"' << *category_it << '"';

    for (++category_it; category_it != photo.categories.end(); ++category_it) {
      body << ",\"" << *category_it << '\"';
    }
  }
  body << "]}";
}
}  // namespace

auto main() -> int {
  try {
    std::map<std::string, std::vector<std::string>> map_category_ids;
    std::unordered_map<std::string, Photo> map_id_photo;
    std::unordered_map<std::string, std::filesystem::path> map_id_path;
    std::unordered_set<std::string> categories;
    // std::string categories_json;
    // TODO(dzen) add caching for requests

    parse_db(map_category_ids, map_id_photo, map_id_path, categories);

    static constexpr int DEFAULT_PORT = 8765;
    simple_http_server::Server server("0.0.0.0", DEFAULT_PORT);

    server.MapDirectory("/photos",
                        simple_http_server::Directory(
                            photos_path,
                            {{"Access-Control-Allow-Origin", "*"},
                             {"Access-Control-Allow-Headers", "*"}},
                            simple_http_server::Directory::Directory::WHITELIST,
                            {std::regex("^.*\\.(jpg|webp)$")}));

    const std::string front_site = "/site";
    server.MapDirectory(front_site,
                        simple_http_server::Directory(staticfiles_path));

    server.MapUrl(
        front_site,
        [&front_site](const simple_http_server::Request& request) {
          const auto& file = request.GetUrl().substr(front_site.length() + 1,
                                                     std::string::npos);

          if (!file.empty() &&
              std::filesystem::exists(staticfiles_path / file)) {
            return simple_http_server::Response();
          }
          return simple_http_server::Server::Render(staticfiles_path /
                                                    "index.html");
        },
        true);

    server.MapUrl("/", [&front_site](
                           const simple_http_server::Request& /*request*/) {
      static constexpr int MOVED_CODE = 301;
      return simple_http_server::Response(
          MOVED_CODE, "Moved Permanently. Redirecting to " + front_site + "/",
          {{"Content-Type", "text/plain; charset = utf-8"},
           {"location", front_site + "/"}},
          "Moved Permanently");
    });

    server.MapUrl(
        "/api/categories",
        [&categories](const simple_http_server::Request& /*request*/) {
          std::ostringstream body;

          body << "{\"categories\":[";

          if (!categories.empty()) {
            auto category_it = categories.begin();

            body << '"' << *category_it << '"';

            for (++category_it; category_it != categories.end();
                 ++category_it) {
              body << ",\"" << *category_it << '\"';
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
        "/api/photos", [&map_id_photo, &map_category_ids](
                           const simple_http_server::Request& request) {
          const auto& arguments = request.GetArguments();

          std::ostringstream body;

          body << "{\"photos\":[";

          std::string name;
          if (arguments.contains("category")) {
            name = arguments.at("category");
          }

          if (map_category_ids.contains(name)) {
            const auto& categories = map_category_ids.at(name);
            auto category_it = categories.begin();

            output_photo(*category_it, map_id_photo.at(*category_it), body);

            for (++category_it; category_it != categories.end();
                 ++category_it) {
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
        "/api/photo",
        [&map_id_photo](const simple_http_server::Request& request) {
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

    server.Start();

  } catch (std::exception& exception) {
    std::cerr << exception.what() << "\n";
    return 1;
  }

  return 0;
}
