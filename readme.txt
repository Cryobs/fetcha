FETCHA(1)                   General Commands Manual                  FETCHA(1)

NNAAMMEE
       fetcha - neofetch-like program, but faster and lighter

SSYYNNOOPPSSIISS
       ffeettcchhaa

DDEESSCCRRIIPPTTIIOONN
       Fetcha is a CLI system information program written in C following the
       suckless philosophy.  It displays an ASCII image along with minimal
       system information.

FFIILLEESS
       _c_o_n_f_i_g_._d_e_f_._h
              Default configuration file (do not change it!).

       _c_o_n_f_i_g_._h
              Configuration file, requires reconmpilation after modification.

       _d_o_c_s_/
              Documentation directory.

       _f_e_t_c_h_a_._c
              Main code file. Here implement most logic of the program.

       _m_o_d_u_l_e_s_._c
              Modules code file. Here implement basic modules.  You can write
              your own (see ffeettcchhaa--mmoodduulleess(5)

       _m_o_d_u_l_e_s_._h
              Modules header file. Here defined all modules.

       _l_i_c_e_n_s_e_._t_x_t
              License file.

       _M_a_k_e_f_i_l_e
              Build file used to compile and install fetcha.  Supports targets
              for building, cleaning, and instalation.

CCUUSSTTOOMMIIZZAATTIIOONN
       Fetcha can be customized by creating a custom _c_o_n_f_i_g_._h and recompiling
       the source code.  You can also write your own modules in _m_o_d_u_l_e_s_._c and
       define them in _m_o_d_u_l_e_s_._h.

       NOTE: See ffeettcchhaa--mmoodduulleess(5) for a module creation tutorial and ffeettcchhaa--
       ccoonnffiigg(5) for a configuration tutorial



                               15 September 2025                     FETCHA(1)
