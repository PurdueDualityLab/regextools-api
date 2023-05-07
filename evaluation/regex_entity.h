//
// Created by charlie on 1/9/23.
//

#ifndef REGEX_EXTRACTOR_REGEX_ENTITY_H
#define REGEX_EXTRACTOR_REGEX_ENTITY_H

#include <string>
#include <nlohmann/json.hpp>

/**
 * Represents a regex entity that got pulled out of a file in a project
 */
struct ParsedRegexEntity {
    ParsedRegexEntity(std::string pattern, std::string flags, std::size_t line_no, std::string source_file)
            : pattern(std::move(pattern))
            , flags(std::move(flags))
            , line_no(line_no)
            , source_file(std::move(source_file))
    {}

    // The regex pattern
    std::string pattern;
    // Any flags associated with the regex
    std::string flags;
    // What line the pattern occurs on in the source
    std::size_t line_no;
    // The source file relative to the project root
    std::string source_file;
};

/**
 * Represents a complete regex entity with all source information and metadata
 */
struct RegexEntity : ParsedRegexEntity {
    RegexEntity()
    : ParsedRegexEntity("", "", 0, "") {
    }

    RegexEntity(std::string pattern, std::string flags, std::size_t line_no, std::string source_file, std::string repo_url, std::string license, std::string commit)
    : ParsedRegexEntity(std::move(pattern), std::move(flags), line_no, std::move(source_file))
    , project_url(std::move(repo_url))
    , license(std::move(license))
    , commit(std::move(commit))
    {}

    RegexEntity(const RegexEntity &copy)
            : ParsedRegexEntity(copy.pattern, copy.flags, copy.line_no, copy.source_file)
            , project_url(copy.project_url)
            , license(copy.license)
            , commit(copy.commit) {
    }

    RegexEntity(RegexEntity &&move) noexcept
    : ParsedRegexEntity(std::move(move.pattern), std::move(move.flags), move.line_no, std::move(move.source_file))
    , project_url(std::move(move.project_url))
    , license(std::move(move.license))
    , commit(std::move(move.commit)) {
    }

    RegexEntity &operator=(RegexEntity &&move) noexcept {
        this->pattern = std::move(move.pattern);
        this->flags = std::move(move.flags);
        this->line_no = move.line_no;
        this->source_file = std::move(move.source_file);
        this->project_url = std::move(move.project_url);
        this->license = std::move(move.license);
        this->commit = std::move(move.commit);

        return *this;
    }

    // Link to the repo for the project
    std::string project_url;
    // What the project is licensed under
    std::string license;
    // The commit that this regex was cloned at
    std::string commit;
};

// JSON helpers for this entity
void to_json(nlohmann::json& j, const RegexEntity& p) {
    j = {
            {"pattern", p.pattern},
            {"flags", p.flags},
            {"line_no", p.line_no},
            {"source_file", p.source_file},
            {"repo_url", p.project_url},
            {"license", p.license},
            {"commit", p.commit},
    };
}

void from_json(const nlohmann::json& j, RegexEntity& p) {
    RegexEntity raw(
            j.at("pattern").get<std::string>(),
            j.at("flags").get<std::string>(),
            j.at("line_no").get<std::size_t>(),
            j.at("source_file").get<std::string>(),
            j.at("repo_url").get<std::string>(),
            j.at("license").get<std::string>(),
            j.at("commit").get<std::string>()
            );

    std::swap(raw, p);
}

#endif //REGEX_EXTRACTOR_REGEX_ENTITY_H
