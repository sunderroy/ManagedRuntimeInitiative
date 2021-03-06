/*
 * Copyright 1997-2005 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *  
 */
// This file is a derivative work resulting from (and including) modifications
// made by Azul Systems, Inc.  The date of such changes is 2010.
// Copyright 2010 Azul Systems, Inc.  All Rights Reserved.
//
// Please contact Azul Systems, Inc., 1600 Plymouth Street, Mountain View, 
// CA 94043 USA, or visit www.azulsystems.com if you need additional information 
// or have any questions.
#ifndef DIVNODE_HPP
#define DIVNODE_HPP

#include "multnode.hpp"
#include "opcodes.hpp"
#include "type.hpp"

// Portions of code courtesy of Clifford Click

// Optimization - Graph Style


//------------------------------DivINode---------------------------------------
// Integer division
// Note: this is division as defined by JVMS, i.e., MinInt/-1 == MinInt.
// On processors which don't naturally support this special case (e.g., x86), 
// the matcher or runtime system must take care of this.
class DivINode : public Node {
public:
  DivINode( Node *c, Node *dividend, Node *divisor ) : Node(c, dividend, divisor ) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual bool depends_only_on_test() const { return true; }
};

//------------------------------DivLNode---------------------------------------
// Long division
class DivLNode : public Node {
public:
  DivLNode( Node *c, Node *dividend, Node *divisor ) : Node(c, dividend, divisor ) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return TypeLong::LONG; }
  virtual uint ideal_reg() const { return Op_RegL; }
  virtual bool depends_only_on_test() const { return true; }
};

//------------------------------DivFNode---------------------------------------
// Float division
class DivFNode : public Node {
public:
  DivFNode( Node *c, Node *dividend, Node *divisor ) : Node(c, dividend, divisor) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return Type::FLOAT; }
  virtual uint ideal_reg() const { return Op_RegF; }
};

//------------------------------DivDNode---------------------------------------
// Double division
class DivDNode : public Node {
public:
  DivDNode( Node *c, Node *dividend, Node *divisor ) : Node(c,dividend, divisor) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
};

//------------------------------ModINode---------------------------------------
// Integer modulus
class ModINode : public Node {
public:
  ModINode( Node *c, Node *in1, Node *in2 ) : Node(c,in1, in2) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual bool depends_only_on_test() const { return true; }
};

//------------------------------ModLNode---------------------------------------
// Long modulus
class ModLNode : public Node {
public:
  ModLNode( Node *c, Node *in1, Node *in2 ) : Node(c,in1, in2) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *bottom_type() const { return TypeLong::LONG; }
  virtual uint ideal_reg() const { return Op_RegL; }
};

//------------------------------ModFNode---------------------------------------
// Float Modulus
class ModFNode : public Node {
public:
  ModFNode( Node *c, Node *in1, Node *in2 ) : Node(c,in1, in2) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return Type::FLOAT; }
  virtual uint ideal_reg() const { return Op_RegF; }
};

//------------------------------ModDNode---------------------------------------
// Double Modulus
class ModDNode : public Node {
public:
  ModDNode( Node *c, Node *in1, Node *in2 ) : Node(c, in1, in2) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
};

//------------------------------DivModNode---------------------------------------
// Division with remainder result.
class DivModNode : public MultiNode {
protected:
  DivModNode( Node *c, Node *dividend, Node *divisor );
public:
  enum {
    div_proj_num =  0,      // quotient
    mod_proj_num =  1       // remainder
  };
virtual int Opcode()const=0;
  virtual Node *Identity( PhaseTransform *phase ) { return this; }
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape) { return NULL; }
  virtual const Type *Value( PhaseTransform *phase ) const { return bottom_type(); }
  virtual uint hash() const { return Node::hash(); }
  virtual bool is_CFG() const  { return false; }
  virtual uint ideal_reg() const { return NotAMachineReg; }

  ProjNode* div_proj() { return proj_out(div_proj_num); }
  ProjNode* mod_proj() { return proj_out(mod_proj_num); }
};

//------------------------------DivModINode---------------------------------------
// Integer division with remainder result.
class DivModINode : public DivModNode {
public:
  DivModINode( Node *c, Node *dividend, Node *divisor ) : DivModNode(c, dividend, divisor) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeTuple::INT_PAIR; }
  virtual Node *match( const ProjNode *proj, const Matcher *m );

  // Make a divmod and associated projections from a div or mod.
  static DivModINode* make(Compile* C, Node* div_or_mod);
};

//------------------------------DivModLNode---------------------------------------
// Long division with remainder result.
class DivModLNode : public DivModNode {
public:
  DivModLNode( Node *c, Node *dividend, Node *divisor ) : DivModNode(c, dividend, divisor) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeTuple::LONG_PAIR; }
  virtual Node *match( const ProjNode *proj, const Matcher *m );

  // Make a divmod and associated projections from a div or mod.
  static DivModLNode* make(Compile* C, Node* div_or_mod);
};

#endif //  DIVNODE_HPP
