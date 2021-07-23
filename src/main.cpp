#include "ncurses_display.h"
#include "system.h"

//below only for testing
#include <string>
#include <iostream>

int main() {
  System system;
  NCursesDisplay::Display(system);
}