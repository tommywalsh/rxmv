/*
 * Regex Renamer
 * Copyright 2013 The People
 * 
 * This code may be used under The Software License of the People
 * (or whichever version of the GPL you prefer)
 */

#include <iostream>
#include <string>
#include <vector>
#include <set>

#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>


class Renamer {
public:
    enum Status {
        VALID,
        BAD_MATCH_PATTERN,
        BAD_FORMAT_STRING,
        NO_MATCH,
        CANT_RENAME
    };

    Renamer(std::string matchPattern, std::string renamePattern);

    bool isValid() const;

    Status getStatus() const;

    std::set<std::size_t> getBadIndexes() const;

    void rename(const std::vector<std::string> &filenames);

    void printRenameReport(const std::vector<std::string> &filenames);

private:
    Status m_status;
    std::set<std::size_t> m_badFiles;

    boost::regex m_matchRx;
    boost::format m_format;
    std::vector<std::pair<std::string, std::string>> m_renameMap;

    void loadRenameMap(const std::vector<std::string> &filenames);

    void doRename(const std::vector<std::string> &filenames, bool reallyRename);

};


int main(int argc, const char **argv) {
    // Process command-line arguments

    std::size_t skip = 0;
    bool rename = false;

    if (argc > 3) {
        std::string secondArg{*(argv + 1)};
        if (secondArg == "-x") {
            rename = true;
            skip = 1;
        }
    }

    if (argc < 4 + skip) {
        std::cout << "Usage: " << argv[0] << " [-x] matchPattern replaceString files\n";
        std::cout
                << "Renames the given list of files.  Each file is compared against the given matchPattern and renamed according to the replaceString.\n\n";
        std::cout << "The matchPattern is any regex accepted by Boost's regex implementation.\n";
        std::cout
                << "The replaceString gives the new filename's format.  Matches from the matchPattern are substituted according to the rules of Boost's format implementation.\n\n";
        std::cout
                << "Renaming only occurs if the '-x' flag is passed in.  Otherwise, a report is printed showing what the results of the renaming would be.\n";

        return -1;
    }

    std::string matchPattern{*(argv + 1 + skip)};
    std::string renamePattern{*(argv + 2 + skip)};
    std::vector<std::string> filenames(argv + 3 + skip, argv + argc);




    // Do the actual renaming (or printing)
    Renamer renamer{matchPattern, renamePattern};
    if (renamer.isValid()) {
        if (rename) {
            renamer.rename(filenames);
        } else {
            renamer.printRenameReport(filenames);
        }
    }



    // Report any errors
    if (renamer.isValid()) {
        return 0;
    } else {
        switch (renamer.getStatus()) {
            case Renamer::BAD_MATCH_PATTERN:
                std::cout << "Bad match pattern: " << matchPattern << "\n";
                break;
            case Renamer::BAD_FORMAT_STRING:
                std::cout << "Bad format string: " << renamePattern << "\n";
                break;
            case Renamer::NO_MATCH:
                std::cout << "These filenames did not match: \n";
                for (auto ix : renamer.getBadIndexes()) {
                    std::cout << filenames[ix] << "\n";
                }
                break;
            case Renamer::CANT_RENAME:
                std::cout << "These files could not be renamed: \n";
                for (auto ix : renamer.getBadIndexes()) {
                    std::cout << filenames[ix] << "\n";
                }
                break;
        }
        return -1;
    }
}


Renamer::Renamer(std::string matchPattern, std::string renamePattern) {
    try {
        m_matchRx = boost::regex{matchPattern};
        m_format = boost::format{renamePattern};
        m_status = VALID;
    } catch (boost::bad_expression) {
        m_status = BAD_MATCH_PATTERN;
    } catch (boost::io::bad_format_string) {
        m_status = BAD_FORMAT_STRING;
    }
}

bool Renamer::isValid() const {
    return m_status == VALID;
}

Renamer::Status Renamer::getStatus() const {
    return m_status;
}

std::set<std::size_t> Renamer::getBadIndexes() const {
    return m_badFiles;
}

void Renamer::rename(const std::vector<std::string> &filenames) {
    doRename(filenames, true);
}

void Renamer::printRenameReport(const std::vector<std::string> &filenames) {
    doRename(filenames, false);
}

void Renamer::loadRenameMap(const std::vector<std::string> &filenames) {
    for (auto i = 0; i < filenames.size(); ++i) {
        auto filename = filenames[i];
        boost::cmatch match;
        if (boost::regex_match(filename.c_str(), match, m_matchRx)) {
            for (auto i = 1; i < match.size(); ++i) {
                m_format % match[i];
            }
            m_renameMap.push_back(std::make_pair(filename, m_format.str()));

        } else {
            m_status = NO_MATCH;
            m_badFiles.insert(i);
        }
    }
}

void Renamer::doRename(const std::vector<std::string> &filenames, bool reallyRename) {
    loadRenameMap(filenames);
    if (m_status == VALID) {
        for (auto i = 0; i < m_renameMap.size(); ++i) {
            auto renameRule = m_renameMap[i];
            std::cout << renameRule.first << " --> " << renameRule.second << "\n";
            if (reallyRename) {
                try {
                    boost::filesystem::rename(renameRule.first, renameRule.second);
                } catch (boost::filesystem::filesystem_error) {
                    m_status = CANT_RENAME;
                    m_badFiles.insert(i);
                }
            }
        }
    }
}

