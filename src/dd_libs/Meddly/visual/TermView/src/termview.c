
/* $Id$ */

/*
    termview: terminal viewing utility for Meddly trace data.
    Copyright (C) 2015, Iowa State University Research Foundation, Inc.

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "system.h"
#include "parse.h"
#include "screen.h"
#include <sys/stat.h>

// #define DEBUG_PARSER

int usage(const char* who)
{
  /* Strip leading directory, if any: */
  const char* name = who;
  for (const char* ptr=who; *ptr; ptr++) {
    if ('/' == *ptr) name = ptr+1;
  }
  printf("\nUsage: %s [options] file\n\n", name);
  printf("Read trace data from the specified file, and display to the terminal.\n");
  printf("If conflicting options are passed, then the last one takes precedence.\n\n");
  printf("Options:\n");
  printf("    -p      :  initial state is `playing'.\n");
  printf("    -s      :  initial state is `stopped'.\n");
  printf("\n");
  return 1;
}

int main(int argc, const char** argv)
{
  const char* pathname = 0;
  char poverride = 0;
  char playing = 0;

  /* 
    Parse arguments 
  */
  for (int i=1; i<argc; i++) {
    if ('-' != argv[i][0]) {
      if (pathname) return usage(argv[0]);
      pathname = argv[i];
      continue;
    }

    /* Future expansion - long options */
    if ('-' == argv[i][1]) {
      
      return usage(argv[0]);
    }
    
    /* Still going?  Short options only. */
    if (argv[i][2]) {
      return usage(argv[0]);
    }

    switch (argv[i][1]) {

      case 'p':
        poverride = 1;
        playing = 1;
        continue;

      case 's':
        poverride = 1;
        playing = 0;
        continue;

      default:
        return usage(argv[0]);
    }


  } /* For i (looping over arguments) */
  if (0==pathname) return usage(argv[0]);



  /*
    Initialize parser
  */
  FILE* inf = fopen(pathname, "r");
  if (0==inf) {
    printf("\nError, couldn't open file `%s', giving up\n\n", pathname);
    return 2;
  }
  fprintf(stderr, "Reading from %s\n", pathname);

  /*
    Determine if input is a pipe (if so play immediately)
    or a file (wait to play).
  */
  if (!poverride) {
    struct stat inode;
    fstat(fileno(inf), &inode);
    if (inode.st_mode & (S_IFREG | S_IFLNK)) {
      playing = 0;
      fprintf(stderr, "Input appears to be a file, will wait to play.\n");
    } else {
      playing = 1;
      fprintf(stderr, "Input appears to be a pipe, will play immediately.\n");
    }
  }


  int code = 0; // return code

  /*
    Structures for grabbing parse data
  */
  const int plength = 256;
  char pbuffer[plength];
  update_t* alist = 0;
  forest_t* F = 0; 
  int fid = 0;

  /*
    Initialize screen data
  */
  screen_t S;
  init_screen_t(&S, 16);
  if (!open_screen(&S)) {
    fprintf(stderr, "\nCouldn't initialize screen, bailing out.\n\n");
    return 1;
  }
  S.playing = playing;
  update_windows(&S);

  /*
    Loop to parse file
  */
  for (int line=1;;line++) {
    if (code) break;
    int c = 0;
    int t;
    if (S.playing) {
      c = fgetc(inf);
    }
    if (EOF == c) break;
    if ('\n' == c) continue;  /* In case of empty lines */

    switch (c) {
      case 0:
        /* We're not consuming the file */
        break;


      case 'T':
#ifdef DEBUG_PARSER
        printf("Parsing T record\n");
#endif
        t = parse_T(inf);
        if (t<0) {
          fprintf(stderr, "Parse error line %d\n", line);
          code = 2;
          break;
        }
        if (t!=1) {
          fprintf(stderr, "Unknown file type\n");
          code = 3;
          break;
        }
        break;

      case 'F':
#ifdef DEBUG_PARSER
        printf("Parsing F record\n");
#endif
        F = new_forest();
        parse_F(inf, F);
#ifdef DEBUG_PARSER
        if (0==F) {
          printf("Null F structure?\n");
        } else {
          printf("Got F structure:\n");
          printf("  fid %d\n", F->fid);
          if (F->name) printf("  name `%s'\n", F->name);
          printf("  left %d\n", F->left);
          printf("  right %d\n", F->right);
          if (F->counts) {
            printf("  counts [%d", F->counts[F->left]);
            for (int k=F->left+1; k<=F->right; k++) {
              printf(", %d", F->counts[k]);
            }
            printf("]\n");
          }
        }
#endif
        add_forest(&S, F);
        update_windows(&S);
        break;

      
      case 'a': 
#ifdef DEBUG_PARSER
        printf("Parsing a record\n");
#endif
        alist = parse_a(inf);
#ifdef DEBUG_PARSER
        printf("Got a structure:\n");
        for (update_t* curr = alist; curr; curr=curr->next) {
          printf("\tfid: %2d level: %3d delta: %3d    ", 
                  curr->fid, curr->level, curr->delta);
          if (curr->next) printf("->");
          printf("\n");
        }
#endif
        update_a(&S, alist);
        kill_update(alist);
        break;

      case 'p': 
#ifdef DEBUG_PARSER
        printf("Parsing p record\n");
#endif
        parse_p(inf, &fid, pbuffer, plength);
#ifdef DEBUG_PARSER
        printf("Got p record fid: %d string: `%s'\n", fid, pbuffer);
#endif
        update_p(&S, fid, pbuffer);
        break;
      
      default:
#ifdef DEBUG_PARSER
        if ('#' == c) {
          printf("Comment, ignoring line\n");
        } else {
          printf("Unknown record `%c', ignoring line\n", c);
        }
#endif
        ignoreLine(inf);
        break;
    }

    /*
      Deal with keyboard input, if any
    */
    int key = keystroke(0);
    if (0==key) continue;

    switch (key) {
      case 'q':
      case 'Q': 
        close_screen();
        return 0;

      case 'p':
      case 'P':
        S.playing = 1-S.playing;
        break;
    }
    update_keys(&S);
  }

  /*
    Done processing file.
    Wait for user to explicitly quit.
  */
  for (int i=0; i<=S.nf; i++) {
    if (S.F[i]) update_p(&S, S.F[i]->fid, "End of file");
  }
  for (;;) {
    int key = keystroke(1);
    if ('q' == key || 'Q' == key) break;
  }

  close_screen();
  return 0;
}
