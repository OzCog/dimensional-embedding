/*
 * opencog/learning/moses/moses/knob_mapper.h
 *
 * Copyright (C) 2002-2008 Novamente LLC
 * All Rights Reserved
 *
 * Written by Moshe Looks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef _MOSES_KNOB_MAPPER_H
#define _MOSES_KNOB_MAPPER_H

#include <map>
#include "eda/field_set.h"
#include "moses/knobs.h"

namespace moses {
  struct knob_mapper {
    typedef eda::field_set field_set;

    //important: knobs are kept sorted in an order consistant with that of the
    //field_set _fields that is constructed according to their corresponding specs
    typedef std::multimap<field_set::disc_spec,disc_knob> disc_map;
    typedef disc_map::value_type disc_v;
    typedef std::multimap<field_set::contin_spec,contin_knob> contin_map;
    typedef contin_map::value_type contin_v;

    disc_map disc;
    contin_map contin;
  };

} //~namespace moses

#endif
