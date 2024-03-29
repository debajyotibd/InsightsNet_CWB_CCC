/*
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */

/**
 * @file
 *
 * The preprocessor macros here are used largely in parser.y and parse_actions.c
 */


#ifndef _cqp_treemacros_h_
#define _cqp_treemacros_h_

#include "../cl/cl.h"

/** Macro to allocate a new Evaltree object in node-mode and fill its OP/left/right/min/max members */
#define NEW_EVALNODE(n, _relop, _left, _right, _min, _max)                        \
                      do {                                                        \
                        n = (Evaltree)cl_malloc(sizeof(union e_tree));            \
                        n->type       = node;                                     \
                        n->node.op_id = _relop;                                   \
                        n->node.left  = _left;                                    \
                        n->node.right = _right;                                   \
                        n->node.min   = _min;                                     \
                        n->node.max   = _max;                                     \
                      } while (0)

/** Macro to allocate a new Evaltree object in leaf-mode and fill its patindex member */
#define NEW_EVALLEAF(n, _patindex)                                                \
                      do {                                                        \
                        n = (Evaltree)cl_malloc(sizeof(union e_tree));            \
                        n->type = leaf;                                           \
                        n->leaf.patindex = _patindex;                             \
                      } while (0)

/** Macro to allocate a new Constrainttree object */
#define NEW_BNODE(n) n = (Constrainttree)cl_malloc(sizeof(union c_tree))

#endif
