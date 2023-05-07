//
// Created by charlie on 4/4/23.
//

#include <string>
#include <fstream>
#include "librereuse/util/ndjson.h"
#include "librereuse/db/parallel_regex_cluster_repository.h"
#include "librereuse/util/chunk.h"
#include "evaluation/regex_entity.h"
#include "evaluation/use_case.h"

int main(int argc, char **argv) {

    /*
     * 0. Load in the necessary data. Inputs are:
     * - NDJSON file of regex entities
     * - ndjson file of projects to query
     */

    std::string regex_entities_path = "TODO";
    std::ifstream regex_entities_file(regex_entities_path);
    auto entities_raw = rereuse::util::read_ndjson<RegexEntity>(regex_entities_file);
    std::vector<std::shared_ptr<RegexEntity>> entities;
    for (auto &entity : entities_raw) {
        auto ptr = std::make_shared<RegexEntity>(std::move(entity));
        entities.push_back(std::move(ptr));
    }

    std::string measure_pkg_specs_path = "TODO";
    // ...
    std::vector<std::string> projects;

    // Overall steps:

    /*
     * 1. Remove any regexes that would create a conflict of interest from our database
     *
     * organize all regexes into two maps:
     *  - one indexed by github location
     *  - one indexed by regex
     *
     *  Start with a set of projects that we want to study. These projects are the ones that we mined regex use cases
     *  from. We want to exclude any regex from that database that came from exclusively one of these projects.
     *  With the maps and input projects we described, do the following:
     *  - get a set of regexes that came from that repository
     *  - for each regex in this set:
     *      - check how many projects it exists in
     *      - if it belongs to just this one, then remove it from consideration
     *      - if it belongs to multiple, then leave it
     */
    std::vector<std::shared_ptr<RegexEntity>> flagged_for_removal;
    {
        // extra scoping so that these maps drop the shared pointers
        std::unordered_multimap<std::string, std::shared_ptr<RegexEntity>> pattern_map;
        std::unordered_multimap<std::string, std::shared_ptr<RegexEntity>> repo_map;

        // First, index all the regexes by regex
        for (const auto &entity: entities) {
            pattern_map.insert(std::make_pair(entity->pattern, entity));
            repo_map.insert(std::make_pair(entity->project_url, entity));
        }

        // for each project in the set of projects that we want to measure...
        for (const auto &project: projects) {
            // ...find all of the regexes that came from that repo...
            for (auto repo_regex = repo_map.find(project); repo_regex != repo_map.end(); ++repo_regex) {
                // ...and if any of them only comes from this project...
                if (pattern_map.count(repo_regex->second->pattern) <= 1) {
                    // ...remove it from consideration
                    flagged_for_removal.push_back(repo_regex->second);
                }
            }
        }
    }

    // Once we have a set of items flagged for removal, we can remove them from the entities list
    entities.erase(std::remove_if(entities.begin(), entities.end(), [&flagged_for_removal](std::shared_ptr<RegexEntity>& item) {
        return std::count(flagged_for_removal.begin(), flagged_for_removal.end(), item) > 0;
    }), entities.end());

    // reduce entities into raw entities
    std::vector<RegexEntity> finalized_entities;
    for (auto &entity : entities) {
        finalized_entities.push_back(*entity);
    }

    for (auto &entity : entities) {
        // clear everything
        entity.reset();
    }
    entities.clear();

    std::vector<rereuse::db::RegexEntity> db_entities;
    for (const auto &finalized_entity : finalized_entities) {
        db_entities.emplace_back(finalized_entity.pattern, finalized_entity.commit);
    }

    // Cluster all the entities and make a database
    auto chunk_size = finalized_entities.size() / 100;
    auto chunks = rereuse::util::chunk<rereuse::db::RegexEntity>(std::move(db_entities), chunk_size);
    std::vector<std::unique_ptr<rereuse::db::Cluster>> clusters;
    for (const auto &chunk : chunks) {
        auto cluster = std::make_unique<rereuse::db::Cluster>(chunk);
        clusters.push_back(std::move(cluster));
    }
    rereuse::db::ParallelRegexClusterRepository repo(std::thread::hardware_concurrency());
    for (auto &cluster : clusters) {
        repo.add_cluster(std::move(cluster));
    }

    /*
     * at this point, we have a dataset of regexes that can be used in the database. Our next step is to prep queries.
     *
     * 2. Create queries out of the use-cases of regexes
     *
     * This step is easy, just create a list of structures.
     */
    std::string use_cases_path;
    std::vector<UseCase> use_cases; // TODO

    /*
     * At this point, we have a cleaned database and a set of queries to run.
     *
     * 3. Run the queries and record the details
     *
     * For each query, record the result set as well. The most general option would be to save the entire regex entity,
     * which would give us information on where the other results came from. This would be helpful
     */

    for (auto &use_case : use_cases) {
        std::unique_ptr<rereuse::query::BaseClusterQuery> query = use_case.create_query();
        auto results = repo.query(query);
        for (const auto &result : results) {
            use_case.add_result(result); // TODO make this work with the correct thing...
        }
    }

    /*
     * Finally, just dump everything
     */

    return 0;
}
