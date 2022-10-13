//
// Created by charlie on 10/12/22.
//

#include <gtest/gtest.h>
#include <unordered_set>
#include <string>
#include "librereuse/db/cluster.h"
#include "librereuse/query/cluster_match_query.h"
#include "librereuse/db/regex_repository.h"
#include "librereuse/query/match_query.h"

static const std::unordered_set<std::string> regexes = {
        "^[0-9]*\\.\\ (.*)",
        "([0-9\\.]+)\\s+(.*)$",
        "^([0-9]+[.][ ]+)(.*)",
        "^[0-9]+\\.\\s+(.*?)",
        "^[\\d]\\. (.*)$",
        "^((?:(\\d+)|\\#)\\. +)(.*?)$",
        "(\\d+?)\\.\\s*(.*)",
        "^(\\d{1,2}\\.?)\\s?(\\d{1,})?(.*)$",
        "([\\d:.]+) \\S*omxplayer\\S* --adev (\\S+).*(\"\\S+\") < \\S+",
        "^([\\d.]+)\\s+PER\\s+(.*?)(?:\\sIn\\s*(.+?))?(?:\\s*DEAD.*)?$",
        "([\\d.]+)\\s+(\\S+)",
        "^([\\d.]+) (\\S+) (\\S+) \\[([\\w\\d:/]+\\s[+\\-]\\d{4})\\] \"(.+?)\" (\\d{3}) ([\\d\\-]+) \"([^\"]+)\" \"([^\"]+)\"",
        "^(\\d+)\\. (\\S.+)",
        "([\\d,\\.\\s]+)\\s*([^\\s]+)",
        "[0-9a-zA-Z :,.]{2,}",
        "^[\\d\\.]+\\s+([\\sa-zA-Z]+)\\s*$",
        "(?:(?:[^a-zA-Z]+')|(?:'[^a-zA-Z]+))|(?:[^a-zA-Z']+)",
        "([0-9A-F\\.]+) *; (NF.*_QC); ([YNM]) #.*",
        "\\.\\s*(\\S+.*)$",
        "^[-.a-zA-Z\\d\\s]+$",
        "[^A-Za-z'-]+",
        "^[A-Za-z0-9 _.-]+$",
        "^[\\w\\-. ]+$",
        "^[\\s\\w\\.\\-_]+$",
        "^[A-Za-z0-9_\\-\\. ]+$",
        "^[\\w., ]+$",
        "^[\\w\\d\\s\\.\\-]+$",
        "(?i)[^\\\\sa-z]+",
        "\\. (\\D+\\S+)",
        "\\.(\\s|\\s\\|\\s)(.*)",
        "^[a-z]*\\s[a-z]*\\s[a-z]*\\s[a-z]*$",
        "/<([a-z_]+:[a-z_]+|[a-z_]+)>",
        "[.\\s\\w]*",
        "[A-Za-z0-9_]|.{4,15}",
        "[^a-z^A-Z]+",
        "[^a-z_$]+",
        "^[^a-zA-Z']+|[^a-zA-Z']+$",
        "FROM\\s*\\(([a-z_]+\\s+as\\s+[a-z_]+)\\s*,\\s*([a-z_]+\\s+as\\s+[a-z_]+\\s*),\\s*([a-z_]+\\s+as\\s+[a-z_]+\\s*)\\)",
        "[ ]*=[ ]*[\\&quot;]*cid[ ]*:[ ]*([^\\&quot;&lt;&gt; ]+)",
        "System.out.println(str.replaceAll(\"[^a-zA-Z]+\", \" \"));",
        "^[-\\w\\s\\.\\+]+$",
        "^[-/.a-zA-Z0-9_ ]+$",
        "^[\\w{L}\\s.]+$",
        "^[\\w\\s\\/:.]+",
        "\\w{3}\\W{2}\\d{4}\\W{2}\\w{6}\\W{2}\\d{2}\\W{2}",
        "^[\\w\\ \\.\\-']+$",
        "^[A-Z0-9._%+-]+@domain.com.ph$",
        "string text1 = \"'\"+Regex.Replace(text,@\"[^A-Za-z0-9\\-\\.\\']+\",\"','\")+\"'\";",
        "grep page_footer /home/foobar/testfile.txt | sed -e 's/[^'\\'']*//' -e 's/[^'\\'']*$//'",
        "\\W(Standard\\WML\\Wof\\WNew\\WJersey\\WLicense)\\W",
        "echo commons-logging.jar | perl -pe 's/(.*?)([^s]+\\.jar)/$1 $1$2/'",
        "\\b[a-z]*(_[a-z]*){3}[a-z]*\\b",
        "(\\$\\{[a-z_]+(\\[[0-9]+\\])?\\}|__[A-Z_]+__)",
        "^[a-zA-Z0-9.&@\\-+_!$% ]{1,11}$",
        "[\\d\\.]+[ ](.*?)[ ]{2,}(.+)",
        "#<\\b(html|body)\\b[^>]*>(.*?)</\\b(html|body)\\b>#im",
        "@PowerShell \"GC 'print.txt'|%%{[Regex]::Matches($_,'\\d{10}')}|%%{$_.Value}\">\"newfile.txt\"",
        "^[a-z][a-z\\d]*([A-Z]+[a-z\\d]*)+$",
        "\\b([a-z]+[A-Z]+[a-zA-Z]*|[A-Z]+[a-z]+[a-zA-Z]*)\\b",
        "^([A-Z0-9,*>-]+):(.+)$",
        "(<style\\b[^>]*?>([\\s\\S]*?)<\\/style>)\\s*",
        "[a-z0-9]+\\..*",
        "^([a-z]+)\\|([a-z]+:[a-z0-9]+,?)+$",
        "w([^a-zA-Z0-9]){1,3}a([^a-zA-Z0-9]){1,3}t([^a-zA-Z0-9]){1,3}c([^a-zA-Z0-9]){1,3}h([^a-zA-Z0-9]){1,3}e([^a-zA-Z0-9]){1,3}s([^a-zA-Z0-9]){1,6}",
        "[^a-zA-Z]+?",
        "^([^A-Za-z]+)",
        "^[A-Z \\d\\W]+$",
        "\\d{3},\\d,\\\"sip:\\d{8}@\\d{3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\",\"\\*{10}\",\"[\" ]\",\"\\d{8}\",\\d,\\d{2},\\d{3},\"\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\",\\d",
        "((0[0-9]{1})|(1[0-2]{1}))-((3[0-1]{1}|[0-2]{1}[0-9]{1}))-[2-9]{1}[0-9]{3} (([0-1]{1}[0-9]{1}|2[0-3]{1})):[0-5]{1}[0-9]{1}:[0-5]{1}[0-9]{1}.[0-9]{3}",
        "([^#][^<\\\"\\'>]+)",
        "(\\d+).+",
        "\\b\\d[^A-Za-z]+|\\d[^A-Za-z]+\\b",
        "([\\sa-zA-Z0-9._-]*)",
        "[\\w\\-\\.\\_\\ ]*",
        "^(\\d+)(.+)",
        "strings.select { |s| /^[a-zA-Z]+$/ =~ s }\n# which is equal to strings.grep /^[a-zA-Z]+$/",
        "(%+|_|[^%_]+)",
        "[^\\.[a-zA-z]*].*",
        "^([-+@\\w. ]+)$",
        "([\\d]+)[^\\\\\\/]*$",
        "^([0-9]{5}\\s[a-z]+)$|^([a-z]+\\s[0-9]{5})$|^([0-9]{2,5})$",
        "^([-.0-9a-z%]+)\\s([-.0-9a-z%]+)($|,)",
        "^([0-9A-Za-z ,._/-]+)$",
        "^(?P<cover>VV|CLR|SKC|SCK|NSC|NCD|BKN|SCT|FEW|[O0]VC|///)\n                        (?P<height>[\\dO]{2,4}|///)?\n                        (?P<cloud>([A-Z][A-Z]+|///))?\\s+",
        "(.[^_?]+)",
        "^\\d{1,2}\\.? [^\\d ]+",
        "(?:INSERT|UPDATE|REPLACE)(?:[A-Z_\\s]*)`?([^\\s`]+)`?\\s*",
        "cat test.txt | perl -pe 's/(?:(@ )|^[^@]).*?(?: (@)|$)/$1$2/g'",
        "([a-z]+)(?: \\.*) ([a-z]+)(?: \\.)* ([a-z]+)",
        "^(?:[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]+\\/)*[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]+$",
        "(?:(\\d{2}[a-z]{2}\\d{2})\\s+)(?:\\d{2}[a-z]{2}\\d{2}\\s+)*|(?:([a-z]{2}\\d{2}[a-z]{2})\\s+)(?:[a-z]{2}\\d{2}[a-z]{2}\\s+)*",
        "^[\\w.!#$%&'*+\\-\\/=?\\^`{|}~]+@[a-z\\d\\-]+(\\.[a-z\\d\\-]+)+$",
        "([A-Z0-9_-]+): (.*?)$",
        "\\b[A-Z][-'a-zA-Z]+,?\\s[A-Z][-'a-zA-Z]{0,19}\\b",
        "Building:.*(\\b\\d+\\s\\w+)",
        "^.*\\bgem 'rails.*$",
        "Track ID (\\d+): audio \\([A-Z0-9_/]+\\) \\[language:[a-z]{3} default_track:[01] forced_track:[01]\\]",
        "(pls[a-zA-Z0-9 .*-]*) \\(([A-Z 0-9]*)\\)",
        "(?is).*\\b@hello\\b.*",
        "^[a-z0-9!#$%&'*+\\-\\/=?^_`{|}~.]+@([a-z0-9%\\-]+\\.){1,}([a-z0-9\\-]+)?$",
        "^[a-z0-9!#$%&'*+\\-/=?^_`\\{|\\}~.]+@([a-z0-9%\\-]+\\.){1,}([a-z0-9\\-]+)?$"
};

std::unordered_set<std::string> missing_regexes = {
        "^[a-z0-9!#$%&'*+\\-\\/=?^_`{|}~.]+@([a-z0-9%\\-]+\\.){1,}([a-z0-9\\-]+)?$",
        "^[a-z0-9!#$%&'*+\\-/=?^_`\\{|\\}~.]+@([a-z0-9%\\-]+\\.){1,}([a-z0-9\\-]+)?$",
        "^[\\w.!#$%&'*+\\-\\/=?\\^`{|}~]+@[a-z\\d\\-]+(\\.[a-z\\d\\-]+)+$",
        "^(?:[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]+\\/)*[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]+$",
};

TEST(ClusterMatchQueryLive, gets_proper_results) {
    auto cluster = std::make_shared<rereuse::db::Cluster>(missing_regexes);
    cluster->compile();

    std::unordered_set<std::string> pos {"cmsale@purdue.edu", "chucks.8090@gmail.com"};
    std::unordered_set<std::string> neg {"cmsale", "cmsale@", "cmsale@purdue"};
    std::unique_ptr<rereuse::query::BaseClusterQuery> query = std::make_unique<rereuse::query::ClusterMatchQuery>(pos, neg);
    std::unique_ptr<rereuse::query::BaseRegexQuery> simple_query = std::make_unique<rereuse::query::MatchQuery>(pos, neg);
    rereuse::db::RegexRepository repo(std::vector(missing_regexes.cbegin(), missing_regexes.cend()));

    auto results = repo.query(simple_query);
    EXPECT_EQ(results.size(), 4);

    ASSERT_TRUE(query->test(cluster, nullptr));

    auto cluster_results = query->query(cluster, nullptr);
    EXPECT_EQ(cluster_results.size(), results.size());
}
