//
// Created by charlie on 2/10/22.
//

#ifndef REGEXTOOLS_MCL_WRAPPER_H
#define REGEXTOOLS_MCL_WRAPPER_H

#include <string>
#include <vector>

class MclWrapper {
public:
    explicit MclWrapper(std::string mcl_path)
    : mcl_path(std::move(mcl_path))
    , has_temp_cluster_file(false) {  }

    ~MclWrapper();

    std::vector<std::vector<unsigned long>> cluster(const std::string &abc_file, double inflation, double pruning, unsigned int top_k);
    std::vector<std::vector<unsigned long>> cluster(const std::string &abc_file, double inflation, double pruning, unsigned int top_k, const std::string &clusters_output_file) const;
private:
    std::string mcl_path;
    std::string temp_cluster_file;
    bool has_temp_cluster_file;
};


#endif //REGEXTOOLS_MCL_WRAPPER_H
