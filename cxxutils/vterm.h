#ifndef VTERM_H
#define VTERM_H

// POSIX++
#include <cstdint>
#include <cstdio>

// POSIX
#include <unistd.h>

typedef const char* const string_literal;

#undef CSI
#define CSI "\x1b["

namespace terminal
{
  template<typename... Args>
  inline void write(const char* fmt, Args... args) noexcept { ::dprintf(STDOUT_FILENO, fmt, args...); }

  inline void hideCursor(void) noexcept { write(CSI "?25l"); }
  inline void showCursor(void) noexcept { write(CSI "?25h"); }

  inline void moveCursorUp   (uint16_t rows = 1) noexcept { write(CSI "%huA", rows); }
  inline void moveCursorDown (uint16_t rows = 1) noexcept { write(CSI "%huB", rows); }
  inline void moveCursorLeft (uint16_t cols = 1) noexcept { write(CSI "%huC", cols); }
  inline void moveCursorRight(uint16_t cols = 1) noexcept { write(CSI "%huD", cols); }

  inline void setCursorHorizontalPosition(uint16_t column) noexcept { write(CSI "%huG", column); }
  inline void setCursorPosition(uint16_t row, uint16_t column) noexcept { write(CSI "%hu;%huH", row, column); }

  inline void clearScreenAfter  (void) noexcept { write(CSI "0J"); }
  inline void clearScreenBefore (void) noexcept { write(CSI "1J"); }
  inline void clearScreen       (void) noexcept { write(CSI "2J"); }

  inline void clearLineAfter    (void) noexcept { write(CSI "0K"); }
  inline void clearLineBefore   (void) noexcept { write(CSI "1K"); }
  inline void clearLine         (void) noexcept { write(CSI "2K"); }

  namespace text
  {
    string_literal reset          = CSI "0m" ;
    string_literal bold           = CSI "1m" ;
    string_literal bold_off       = CSI "22m";
    string_literal underline      = CSI "4m" ;
    string_literal underline_off  = CSI "24m";
    string_literal overline       = CSI "53m";
    string_literal overline_off   = CSI "55m";
    string_literal layerswap      = CSI "7m" ;
    string_literal layerswap_off  = CSI "27m";
    string_literal blink          = CSI "5m" ;
    string_literal blink_off      = CSI "25m";
  }

  namespace fg
  {
    string_literal black          = CSI "30m";
    string_literal red            = CSI "31m";
    string_literal green          = CSI "32m";
    string_literal yellow         = CSI "33m";
    string_literal blue           = CSI "34m";
    string_literal magenta        = CSI "35m";
    string_literal cyan           = CSI "36m";
    string_literal white          = CSI "37m";
    string_literal reset          = CSI "39m";
  }

  namespace bg
  {
    string_literal black          = CSI "40m";
    string_literal red            = CSI "41m";
    string_literal green          = CSI "42m";
    string_literal yellow         = CSI "43m";
    string_literal blue           = CSI "44m";
    string_literal magenta        = CSI "45m";
    string_literal cyan           = CSI "46m";
    string_literal white          = CSI "47m";
    string_literal reset          = CSI "49m";
  }

  namespace style
  {
    string_literal reset          = CSI "0m";
    string_literal brightRed      = CSI "0;40;31;1m";
    string_literal brightGreen    = CSI "0;40;32;1m";
    string_literal brightYellow   = CSI "0;40;33;1m";
  }

  string_literal information      = CSI "0;40;34m"   "INFORMATION:"     CSI "0m ";
  string_literal warning          = CSI "0;40;33;1m" "WARNING:"         CSI "0m ";
  string_literal severe           = CSI "0;40;31;1m" "SEVERE WARNING:"  CSI "0m ";
  string_literal critical         = CSI "0;41;37;1m" "CRITICAL ERROR:"  CSI "0m ";
}

#endif // STREAMCOLORS_H
