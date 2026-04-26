#include "i18n.h"
#include <cstdlib>
#include <clocale>

#ifndef _WIN32
#include <libintl.h>
#endif

void i18n_init(const std::string& localedir, const std::string& locale)
{
#ifndef _WIN32
    if (!locale.empty())
    {
        // Override LANGUAGE so gettext honours the user-chosen language.
        // setenv is POSIX; on Windows this is not needed.
        ::setenv("LANGUAGE", locale.c_str(), 1);
        ::setlocale(LC_ALL, "");
    }
    else
    {
        ::setlocale(LC_ALL, "");
    }

    ::bindtextdomain(SLYMAIL_I18N_DOMAIN, localedir.c_str());
    ::bind_textdomain_codeset(SLYMAIL_I18N_DOMAIN, "UTF-8");
    ::textdomain(SLYMAIL_I18N_DOMAIN);
#else
    (void)localedir;
    (void)locale;
    ::setlocale(LC_ALL, "");
#endif
}

std::vector<LangEntry> i18n_supported_languages()
{
    return {
        { "",      "OS Default"                      },
        { "cy",    "Cymraeg (Welsh)"                 },
        { "da",    "Dansk (Danish)"                  },
        { "de",    "Deutsch (German)"                },
        { "el",    "Ελληνικά (Greek)" },
        { "es",    "Español (Spanish)"          },
        { "fi",    "Suomi (Finnish)"                 },
        { "fr",    "Français (French)"          },
        { "ga",    "Gaeilge (Irish)"                 },
        { "ja",    "日本語 (Japanese)"   },
        { "nb",    "Norsk Bokmål (Norwegian)"   },
        { "pirate","Pirate Speak"                    },
        { "pl",    "Polski (Polish)"                 },
        { "pt_BR", "Português do Brasil"        },
        { "ru",    "Русский (Russian)" },
        { "sv",    "Svenska (Swedish)"               },
        { "zh_CN", "简体中文 (Simplified Chinese)"  },
        { "zh_TW", "繁體中文 (Traditional Chinese)" },
    };
}
