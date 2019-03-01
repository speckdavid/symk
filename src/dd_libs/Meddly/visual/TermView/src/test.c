/* $Id: center.c 272 2006-09-29 14:17:16Z asminer $ */

/* 
    A fancier curses example (number 2).
    Compile with
	gcc <this> -lncurses
*/

#include "system.h"
#include <string.h>

int main()
{
  char mesg[]="A test string";
  int row, col;

  initscr();
  getmaxyx(stdscr, row, col);
  mvprintw(row/2, (col-strlen(mesg))/2, "%s", mesg);

  mvprintw(row-2, 0, "This screen has %d rows and %d columns\n", row, col);
  printw("Try resizing and rerun.  Press any key to exit.");
  refresh();
  getch();
  endwin();
  return 0;
}
