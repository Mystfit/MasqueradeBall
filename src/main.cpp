#include "app.hpp"
#include "level/stdin_reader.hpp"

#include <memory>

int main() {
    auto stdin_reader = std::make_unique<StdinReader>();
    stdin_reader->start();

    App app(std::move(stdin_reader));
    app.run();

    return 0;
}
