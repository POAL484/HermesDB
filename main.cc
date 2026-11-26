#include <drogon/drogon.h>

int main() {
    std::string cfg = std::getenv("HERMES_CONFIG");

    if(std::filesystem::exists(cfg)) {
        drogon::app()
            .loadConfigFile("../config.json");
    }

    drogon::app().addListener("0.0.0.0", 80)
            .run();

    return 0;
}
