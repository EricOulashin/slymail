#ifndef SLYMAIL_I18N_H
#define SLYMAIL_I18N_H

// ============================================================
// Internationalization (i18n) support for SlyMail
//
// Uses GNU gettext on POSIX (Linux/macOS/BSD).
// On Windows (MSVC), falls back to returning the original
// English string (no translation) unless libintl is available.
//
// Usage:
//   #include "i18n.h"
//   const char* s = _("Hello");   // returns translated string
// ============================================================

#ifdef _WIN32
  // On Windows/MSVC, link against a bundled libintl if available;
  // otherwise define a no-op stub so the code still compiles.
  #ifdef HAVE_LIBINTL
    #include <libintl.h>
    #define _(str) gettext(str)
  #else
    // No-op stub: English only
    #define _(str) (str)
    #define gettext(str) (str)
    #define ngettext(s1, s2, n) ((n) == 1 ? (s1) : (s2))
    #define bindtextdomain(d, p) ((void)0)
    #define textdomain(d)        ((void)0)
  #endif
#else
  // POSIX: GNU gettext is available as part of libc (glibc) or libintl
  #include <libintl.h>
  #define _(str) gettext(str)
  #define N_(str) str        // no-op marker for string extraction only
#endif

#include <string>
#include <vector>

// The gettext domain name used for all SlyMail messages
#define SLYMAIL_I18N_DOMAIN "slymail"

// Initialise gettext for the given locale.
// localedir should be the directory that contains the
//   <lang>/LC_MESSAGES/slymail.mo  tree.
// If locale is empty, the OS locale (LANG / LC_ALL) is used.
// Call once at program startup, before any UI is drawn.
void i18n_init(const std::string& localedir,
               const std::string& locale = "");

// Return the list of all languages supported by SlyMail
// (code, display-name pairs).  The special entry {"", "OS Default"}
// means "use the operating-system locale".
struct LangEntry
{
    std::string code;        // BCP-47-ish code (e.g. "de", "zh-CN", "pirate")
    std::string displayName; // Human-readable name in that language
};
std::vector<LangEntry> i18n_supported_languages();

#endif // SLYMAIL_I18N_H
