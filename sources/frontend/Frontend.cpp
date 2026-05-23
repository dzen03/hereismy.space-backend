#include "Frontend.h"

#include <filesystem>
#include <string>

#include "Directory.h"
#include "Request.h"
#include "Response.h"
#include "Server.h"

namespace {
const auto staticfiles_path = std::filesystem::current_path() / "staticfiles";
}  // namespace

void Frontend::map_urls(simple_http_server::Server& server) {
  const std::string front_site = "/site";
  server.MapDirectory(front_site,
                      simple_http_server::Directory(staticfiles_path));

  server.MapUrl(
      front_site,
      [&front_site](const simple_http_server::Request& request) -> auto {
        const auto& file =
            request.GetUrl().substr(front_site.length() + 1, std::string::npos);

        if (!file.empty() && std::filesystem::exists(staticfiles_path / file)) {
          return simple_http_server::Response();
        }
        return simple_http_server::Server::Render(staticfiles_path /
                                                  "index.html");
      },
      true);

  server.MapUrl(
      "/",
      [&front_site](const simple_http_server::Request& /*request*/) -> auto {
        static constexpr int MOVED_CODE = 301;
        return simple_http_server::Response(
            MOVED_CODE, "Moved Permanently. Redirecting to " + front_site + "/",
            {{"Content-Type", "text/plain; charset = utf-8"},
             {"location", front_site + "/"}},
            "Moved Permanently");
      });
}
