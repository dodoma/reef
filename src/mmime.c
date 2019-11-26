#include "reef.h"

#define TYPE_LEN      128

/* https://gcc-help.gcc.gnu.narkive.com/T8njTEOG/initialization-of-nested-flexible-array-members */
#define MAX_MINOR_NUM 50

/*
 * 不认识的类型请手动添加
 */
struct _minor {
    char *type;
    char *ext;
};

struct _mime {
    char *major;
    struct _minor minor[MAX_MINOR_NUM];
} mimes[] = {
    {"text", {
            {"calendar", "ics"},
            {"css", "css"},
            {"csv", "csv"},
            {"html", "html"},
            {"javascript", "js"},
            {"plain", "txt"},
            {"x-lua", "lua"},
            {"x-shellscript", "sh"},
            {"x-perl", "perl"},
            {"x-python", "py"},
            {"x-ruby", "ruby"},
            {"x-tcl", "tcl"},
            {"x-c", "c"},
            {NULL, NULL}
        }
    },
    {"image", {
            {"bmp", "bmp"},
            {"gif", "gif"},
            {"jpeg", "jpg"},
            {"png", "png"},
            {"svg+xml", "svg"},
            {"tiff", "tiff"},
            {"vnd.microsoft.icon", "ico"},
            {"webp", "webp"},
            {NULL, NULL}
        }
    },
    {"audio", {
            {"aac", "aac"},
            {"midi", "midi"},
            {"x-midi", "midi"},
            {"mpeg", "mp3"},
            {"ogg", "oga"},
            {"wav", "wav"},
            {"webm", "weba"},
            {NULL, NULL}
        }
    },
    {"video", {
            {"3gpp", "3gp"},
            {"3gpp2", "3g2"},
            {"mpeg", "mpeg"},
            {"ogg", "ogv"},
            {"webm", "webm"},
            {"x-msvideo", "avi"},
            {"x-flv", "flv"},
            {"x-m4v", "mp4"},
            {"mp4", "mp4"},
            {NULL, NULL}
        }
    },
    {"font", {
            {"otf", "otf"},
            {"ttf", "ttf"},
            {"woff", "woff"},
            {"woff2", "woff2"},
            {NULL, NULL}
        }
    },
    {"application", {
            {"epub+zip",                                                      "epub"},
            {"java-archive",                                                  "jar"},
            {"json",                                                          "json"},
            {"ld+json",                                                       "jsonld"},
            {"msword",                                                        "doc"},
            {"octet-stream",                                                  "bin"},
            {"ogg",                                                           "ogx"},
            {"pdf",                                                           "pdf"},
            {"rtf",                                                           "rtf"},
            {"vnd.amazon.ebook",                                              "azw"},
            {"vnd.apple.installer+xml",                                       "mpkg"},
            {"vnd.mozilla.xul+xml",                                           "xul"},
            {"vnd.ms-excel",                                                  "xls"},
            {"vnd.ms-fontobject",                                             "eot"},
            {"vnd.ms-powerpoint",                                             "ppt"},
            {"vnd.oasis.opendocument.presentation",                           "odp"},
            {"vnd.oasis.opendocument.spreadsheet",                            "ods"},
            {"vnd.oasis.opendocument.text",                                   "odt"},
            {"vnd.openxmlformats-officedocument.presentationml.presentation", "pptx"},
            {"vnd.openxmlformats-officedocument.spreadsheetml.sheet",         "xlsx"},
            {"vnd.openxmlformats-officedocument.wordprocessingml.document",   "docx"},
            {"vnd.visio",                                                     "vsd"},
            {"x-7z-compressed",                                               "7z"},
            {"x-abiword",                                                     "abw"},
            {"x-bzip",                                                        "bz"},
            {"x-bzip2",                                                       "bz2"},
            {"x-csh",                                                         "csh"},
            {"x-freearc",                                                     "arc"},
            {"xhtml+xml",                                                     "xhtml"},
            {"xml",                                                           "xml"},
            {"x-rar-compressed",                                              "rar"},
            {"x-sh",                                                          "sh"},
            {"x-shockwave-flash",                                             "swf"},
            {"x-tar",                                                         "tar"},
            {"zip",                                                           "zip"},
            {NULL, NULL}
        }
    },
    {.major = NULL, .minor = {{.type = NULL, .ext = NULL}}}
};


/*
 * audio/mpeg; charset=binary
 */
const char *mmime_extension(const char *type)
{
    if (!type) return NULL;

    /*
     * 1. 找出 audio 和 mpeg
     */
    char major_type[TYPE_LEN], minor_type[TYPE_LEN];
    memset(major_type, 0x0, sizeof(major_type));
    memset(minor_type, 0x0, sizeof(minor_type));

    char *p = (char*)type;
    size_t pos = 0;
    while (*p && *p != '/' && pos < TYPE_LEN) {
        major_type[pos] = *p;
        pos++;
        p++;
    }
    if (*p != '/') return NULL;
    p++;
    pos = 0;
    while (*p && *p != ';' && pos < TYPE_LEN) {
        minor_type[pos] = *p;
        pos++;
        p++;
    }
    if (*p != ';') return NULL;

    /*
     * 表中查找
     */
    struct _mime *m = mimes;
    while (m->major) {
        if (!strncmp(m->major, major_type, strlen(major_type))) {
            struct _minor *n = m->minor;
            while (n->type) {
                if (!strncmp(n->type, minor_type, strlen(minor_type))) return n->ext;

                n++;
            }
        }

        m++;
    }

    return NULL;
}
