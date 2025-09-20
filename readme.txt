FETCHA(1)                   General Commands Manual                  FETCHA(1)

NAME
       fetcha - neofetch-like program, but faster and lighter

SYNOPSIS
       fetcha

DESCRIPTION
       Fetcha is a CLI system information program written in C following the
       suckless philosophy.  It displays an ASCII image along with minimal
       system information.

FILES
       config.def.h
              Default configuration file (do not change it!).

       config.h
              Configuration file, requires reconmpilation after modification.

       docs/
              Documentation directory.

       fetcha.c
              Main code file. Here implement most logic of the program.

       modules.c
              Modules code file. Here implement basic modules.  You can write
              your own (see fetcha-modules(5)

       modules.h
              Modules header file. Here defined all modules.

       license.txt
              License file.

       Makefile
              Build file used to compile and install fetcha.  Supports targets
              for building, cleaning, and instalation.

CUSTOMIZATION
       Fetcha can be customized by creating a custom config.h and recompiling
       the source code.  You can also write your own modules in modules.c and
       define them in modules.h.

       NOTE: See fetcha-modules(5) for a module creation tutorial and fetcha-
       config(5) for a configuration tutorial

SEE ALSO
       fetcha-modules(5), fetcha-config(5)


                               15 September 2025                     FETCHA(1)
