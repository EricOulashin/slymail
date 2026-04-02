#include "terminal.h"

using std::string;

void ITerminal::drawHLine(int row, int col, int len)
{
    for (int i = 0; i < len; ++i)
    {
        putCP437(row, col + i, CP437_BOX_DRAWINGS_HORIZONTAL_SINGLE);
    }
}

void ITerminal::drawVLine(int row, int col, int len)
{
    for (int i = 0; i < len; ++i)
    {
        putCP437(row + i, col, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    }
}

void ITerminal::drawBox(int row, int col, int height, int width)
{
    putCP437(row, col, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
    putCP437(row, col + width - 1, CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE);
    putCP437(row + height - 1, col, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
    putCP437(row + height - 1, col + width - 1, CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE);
    drawHLine(row, col + 1, width - 2);
    drawHLine(row + height - 1, col + 1, width - 2);
    drawVLine(row + 1, col, height - 2);
    drawVLine(row + 1, col + width - 1, height - 2);
}

// ============================================================
// Platform utility functions
// ============================================================

#if SLYMAIL_WINDOWS
// Returns true if tar.exe is in the PATH.  Result is cached after the first call.
bool isTarAvailable()
{
    static int cached = -1;
    if (cached < 0)
    {
        cached = (system("where tar >NUL 2>&1") == 0) ? 1 : 0;
    }
    return cached == 1;
}
#endif
