#include <drogon/drogon.h>
#include <signal.h>

using namespace drogon;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    drogon::app().quit();
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    //Set HTTP listener address and port
    // drogon::app().addListener("0.0.0.0", 5555);
    //Load config file
    //drogon::app().loadConfigFile("../config.json");
    //drogon::app().loadConfigFile("../config.yaml");
    //Run HTTP framework,the method will block in the internal event loop
    app().loadConfigFile("../config.yaml").run();
    return 0;
}
