#ifndef SRC_SHA256IDGENERATOR_HPP_
#define SRC_SHA256IDGENERATOR_HPP_

#include "immutable/idGenerator.hpp"
#include "immutable/pageId.hpp"

class Sha256IdGenerator : public IdGenerator {
private:
    // Length of Sha256 hash and one char for \0 character.
    static constexpr int bufferSize = 65;
public:
    virtual PageId generateId(std::string const& content) const
    {
        thread_local static char buffer[bufferSize];
        std::string command("echo -n \"" + content + "\" | sha256sum");

        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << "popen failed\n";
            exit(EXIT_FAILURE);
        }
        if (fscanf(pipe, "%64s", buffer) != 0) {
            buffer[bufferSize - 1] = '\0';
        } else {
            std::cerr << "unexpected error in reading\n";
            exit(EXIT_FAILURE);
        }

        if (pclose(pipe) != 0) {
            std::cerr << "pclose failed\n";
            exit(EXIT_FAILURE);
        }
        return PageId(buffer);
    }
};

#endif /* SRC_SHA256IDGENERATOR_HPP_ */
