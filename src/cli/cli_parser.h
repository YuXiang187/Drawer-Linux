#pragma once

#include <QStringList>

class CliParser
{
public:
    explicit CliParser(int argc, char *argv[]);

    bool wantsTrigger() const;
    bool wantsVersion() const;
    bool wantsHelp() const;

private:
    QStringList m_args;
};
