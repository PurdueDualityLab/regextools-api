//
// Created by charlie on 10/20/22.
//

#include "librereuse/db/cluster.h"

int main() {

    rereuse::db::Cluster cluster({
                                         "^cf-",
                                         ".(?m:.).",
                                         "^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}(,[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,})*$",
                                         "To:.*<[A-Z0-9._+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}>",
                                         "([^[:space:]@;]+@[^[:space:]@;]+(\\s*))+",
                                         "^([_A-Z]+)\\/([_A-Z]+)\\/([_A-Z]+)$",
                                         "(\\w{,3}), (\\d{,4})/(\\d{,2})/(\\d{2}) (\\d{,2}):(\\d{2}):(\\d{2}) (\\w{,3})",
                                         "^([[:lower:]][[:lower:]\\d]*[\\.\\?:\\/]?)+",
                                         "[A-Z]*(?: *[A-Z]+){4,}",
                                         "[A-Z0-9]+(-[A-Z0-9]+)?",
                                         "[^[:alnum:]##.-]",
                                         "[A-Z]+_[A-Z]+",
                                         "\\$\\{?(?i:ENV):([A-Z][A-Z0-9_]*)\\}?",
                                         "([A-Z]+[A-Z0-9]*)",
                                         "^.*\\040\\02..",
                                         "^[\\-[:alnum:]_:.]+$",
                                         "[^a-zA-Z 0-9 *?:.+-_\\[\\]]+",
                                         "([^\\w\\-%\\/,\\!\\?=\\`~@#\\$^&*\\(\\)+\\[\\]\\{\\}\\|;:\"\\.<> ])",
                                         "sed 's/[^-_a-zA-Z0-9[:space:]]//g' file",
                                         "[\"\\\\[:^ascii:][:cntrl:]]",
                                         "^(.*)_[A-Z]+(\\.[^.]+)$",
                                         "^\\\\|\\\\|((\\\\w+\\\\.)+[[:alpha:]]+)[^[:alpha:]]",
                                         "[^[:alnum:]\\\\-]",
                                         "SELECT SUBSTRING('abc a c' FROM 'a[^[:space:]]c');",
                                         ".+/(?i:current)$",
                                         "[[:digit:]][[:digit:]]",
                                         "[[:alnum:]]{3}([8-9]|1[0-2])",
                                         "([A-Z]*_?[A-Z]*)*",
                                         "^=({|\\()*[A-Z]+\\d+(\\+|-|\\*|\\/)?(}|\\))*$",
                                         "[^[:punct:]]",
                                         "^\\w+([-+.'']*\\w*)*@\\w*([-.çöişğü]\\w*)*\\.\\w+([-.]\\w+)*$",
                                         "^(.);(\\d{4});(\\d{8});([A-K]);(\\d{7,8});(\\d{8});([A-Z ]+);([ ,\\d]+);(\\d{8});(\\d);(\\d); *$",
                                         "([^a-zA-Z0-9\\-+=*\\/_~!\\@\\$%^&:\\<\\>{}])",
                                         "([A-Z]+[a-z0-9]+)+",
                                         "[A-Z]+:.*$",
                                         "[A-Z0]*0[A-Z0]*",
                                         "[\\\\\"[:cntrl:]]",
                                         "([A-Z]+[a-z0-9]+)",
                                         "^([A-Z]+)([a-z]+)$",
                                         "[a-z]+[A-Z]+",
                                         "(?:[^A-Za-z0-9@＠$#＃#{invalid_chars_group}]|^)",
                                         "[[:digit:]]*",
                                         "[^\\/].?([A-Z]*)_(.*)_(.*)[\\.$]",
                                         "([D]{1}[C]{1}[0]{3})?([D]{1}[C]{1}[0]{3}[1-9]{1}[0-9]{3})|([D]{1}[C]{1}[1-9]{1})?([D]{1}[C]{1}[1-9]{1}[0-9]{6})|([D]{1}[C]{1}[0-9]{1}[1-9]{1})?([D]{1}[C]{1}[0-9]{1}[1-9]{1}[0-9]{5})|([D]{1}[C]{1}[0-9]{2}[1]{1})?([D]{1}[C]{1}[0-9]{2}[1]{1}[0-9]{4})",
                                         "@[SE]RR[A-Z0-9\\-.]+[ _]([a-zA-Z0-9\\-]*)[:_]([0-9]{1,2})[:_]([0-9]+)[:_]([0-9]+)[:_]([0-9]+)",
                                         "^.{,13}$",
                                         "\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\+\\d{2}:\\d{2}",
                                         "[[:graph:] ]",
                                         "[^A-Za-zàèìòùáéíóú(othercharacters..)]",
                                         "[^a-zA-Z0-9\\d\\s:]+[^a-zA-Z0-9\\d\\s:]",
                                         "(\\[?[A-Z@]+\\]?\\[?[0-9]+\\]?)",
                                         "(?:\\s[[:upper:]][[:lower:]])*",
                                         ".*[:öäë]",
                                         "[^a-z0-9A-Z\\-\\'\\|\\!\\.\\?\\:\\)\\(\\;\\*\\\"]",
                                         "([A-Z]+)\\((0x[a-f0-9]+)\\)$",
                                         "(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}),(\\d{3})\\|",
                                         "\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{6}$",
                                         "[^a-zA-Z0-9()\\-+/*\\s%,<>]",
                                         "^(?:[A-Z]+\\. ?)+$",
                                         "[A-Z ]+(:|\\[[a-zA-Z ]+\\]:)",
                                         "([0-9]+)\\. ([a-zÃ¡Ã©Ã­Ã³ÃºÃ¢ÃªÃ ])",
                                         "[^a-zA-Z0-9\\-\\.~_]+",
                                         "^\\{{(\\\\[A-Z][\\\\A-Za-z\\d_]+::[A-Z\\d_]+)\\}}$",
                                         ",((\\d{4})-(\\d{2})-(\\d{2}) (\\d{2}):(\\d{2}):(\\d{2}))",
                                         "<!(?i:DOCTYPE)|<(?i:html)|<\\?(?i:php)",
                                         "^[A-Z]+-\\d+$",
                                         "[^A-Za-z0-9_.-]+",
                                         "(\\d+|\\})[年|載|歳]",
                                         "(?:10*){,5}",
                                         "^([A-Z]+)(\\d+)$",
                                         "[^\\.\\w-]+",
                                         "([A-Z]+-\\d+)",
                                         "<CreatedDate>(\\d{4})-(\\d{2})-(\\d{2})T(\\d{2}:\\d{2}:\\d{2})<\\/CreatedDate>",
                                         "([^\\w\\s\\-\\:\\.\\!\\/\\(\\)])",
                                         "(&)|([^\\x20-\\x25\\x27-\\x7e]+)",
                                         "[1][الهه اردونی]",
                                         "[^а-яА-ЯёЁa-zA-Z0-9\\s\\.,-_]*",
                                         "\\d{2}\\.\\d{3}\\.\\d{3}\\.\\/\\d{4}-\\d{2}",
                                         "^type: ([A-Z]+)$",
                                         "^[A-Z]+_",
                                         "^([A-Z]+)$",
                                         "[ 查, 詢, 促, 進, 民, 間, 參, 與, 公, 共, 建, 設, 法, （, 210, ＢＯＴ, 法, ）, ., ]",
                                         "[^n0-9:\\(\\)\\?=!<>/%&| ]",
                                         "([[:alnum:]]+|\\*)",
                                         "FREQ=([A-Z]+)",
                                         "^[A-Z]+://",
                                         "([A-Z]+[a-z])|([a-z]+[A-Z])",
                                         "([A-Z][a-z]+)\\/([A-Z][a-z]+)\\/([A-Z][a-z]+)(\\/)?",
                                         "[[:punct:]]{2}",
                                         "<[a-z]{1,}-[a-z]{1,}>",
                                         "^(\\d{3}-\\d{2}-\\d{4})|(\\d{3}\\d{2}\\d{4})|(\\d{3}\\s{1}\\d{2}\\s{1}\\d{4})$",
                                         "[^\\d\\w\\s,.()]",
                                         "^(?:([A-Z-]+\\??))",
                                         "{[A-Z_]+[0-9]*}",
                                         "(?:^|;)[A-Z-]+:",
                                         ".*[^a-z]+",
                                         "%([A-Z0-9-]+)\\\\.",
                                         "[^a-zA-Z0-9\\.\\-_\\+\\/]",
                                         "[^a-z0-9\\'\\\"\\)\\]\\.\\!\\:\\?]+$",
                                         "^[A-Z]{2} [0-9]{2} [A-Z]{2} [0-9]{4}$",
                                         "^\\d{3} \\d{3} \\d{3} \\d{3}$"
    });

    cluster.compile();
    cluster.prime();

    return 0;
}