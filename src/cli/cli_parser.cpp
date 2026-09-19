#include "cli_parser.h"

CliParser::CliParser(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        m_args << QString::fromLocal8Bit(argv[i]);
    }
}

bool CliParser::wantsTrigger() const
{
    return m_args.contains("--trigger");
}

bool CliParser::wantsVersion() const
{
    return m_args.contains("--version") || m_args.contains("-v");
}

bool CliParser::wantsHelp() const
{
    return m_args.contains("--help") || m_args.contains("-h");
}
