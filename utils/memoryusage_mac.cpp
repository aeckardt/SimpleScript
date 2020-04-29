#ifdef __APPLE__

#include "memoryusage.h"

#include <QProcess>

#include <string>
#include <vector>

#include <sys/sysctl.h>

// grammar [A-Za-z0-9] skip rest

MemoryUsage::MemoryUsage() :
    _used(0),
    _unused(0),
    _total(0),
    _wired(0)
{
}

enum TokenId {
    Empty,
    String,
    Number
};

struct Token {
    TokenId id;
    std::string content;
};

void MemoryUsage::retrieveInfo()
{
    clear();

    QProcess p;
    p.start("bash", QStringList() << "-c" << "top -l 1 | grep PhysMem");
    p.waitForFinished();
    std::string system_info = p.readAllStandardOutput().toStdString();
    p.close();

    std::vector<Token> tokens;

    std::string::const_iterator it = system_info.begin();
    Token token = {Empty, std::string()};
    while (it != system_info.end()) {
        if (*it == ' ' || *it == '\n') {
            if (token.id != Empty)
                tokens.push_back(token);
            token.id = Empty;
            token.content.clear();
        } else if ((*it >= 'A' && *it <= 'Z') || (*it >= 'a' && *it <= 'z') || (*it >= '0' && *it <= '9')) {
            if (token.id == Empty) {
                if (*it >= '0' && *it <= '9')
                    token.id = Number;
                else
                    token.id = String;
            }
            if (!(token.id == Number && ((*it >= 'A' && *it <= 'Z') || (*it >= 'a' && *it <= 'z'))))
                token.content.push_back(*it);
        }
        it++;
    }

    if (token.id != Empty)
        tokens.push_back(token);

    std::vector<Token>::const_iterator token_it = tokens.begin();
    while (token_it != tokens.end()) {
        if (token_it->id == Number) {
            if (token_it + 1 != tokens.end() && (token_it + 1)->id == String) {
                if ((token_it + 1)->content == "used")
                    _used = atoi(token_it->content.c_str());
                else if ((token_it + 1)->content == "unused")
                    _unused = atoi(token_it->content.c_str());
                else if ((token_it + 1)->content == "wired")
                    _wired = atoi(token_it->content.c_str());
            }
        }
        token_it++;
    }

    int mib[2];
    int64_t physical_memory = 0;
    size_t length;

    // Get the Physical memory size
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    length = sizeof(int64_t);
    sysctl(mib, 2, &physical_memory, &length, NULL, 0);

    _total = (physical_memory / 1024 / 1024);
}

void MemoryUsage::clear()
{
    _total = 0;
    _used = 0;
    _wired = 0;
    _unused = 0;
}

#endif // __APPLE__
