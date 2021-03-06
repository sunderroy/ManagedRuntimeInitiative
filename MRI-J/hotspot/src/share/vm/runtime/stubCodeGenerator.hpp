/*
 * Copyright 1997-2000 Sun Microsystems, Inc.  All Rights Reserved.
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
#ifndef STUBCODEGENERATOR_HPP
#define STUBCODEGENERATOR_HPP

#include "assembler_pd.hpp"

// All the basic framework for stubcode generation/debugging/printing.

// A StubCodeDesc describes a piece of generated code (usually stubs).
// This information is mainly useful for debugging and printing.
// Currently, code descriptors are simply chained in a linked list,
// this may have to change if searching becomes too slow.

class StubCodeDesc: public CHeapObj {
 protected:
  static StubCodeDesc* _list;                  // the list of all descriptors
  static int           _count;                 // length of list

  StubCodeDesc*        _next;                  // the next element in the linked list
  const char*          _group;                 // the group to which the stub code belongs
  const char*          _name;                  // the name assigned to the stub code
  int                  _index;                 // serial number assigned to the stub
  int                  _framesize;             // framesize for gdb
  address              _begin;                 // points to the first byte of the stub code    (included)
  address              _end;                   // points to the first byte after the stub code (excluded)

  void set_end(address end) {
    assert(_begin <= end, "begin & end not properly ordered");
    _end = end;
  }

  void set_begin(address begin) {
    assert(begin >= _begin, "begin may not decrease");
    assert(_end == NULL || begin <= _end, "begin & end not properly ordered");
    _begin = begin;
  }

  friend class StubCodeMark;
  friend class StubCodeGenerator;

 public:
  static StubCodeDesc* desc_for(address pc);     // returns the code descriptor for the code containing pc or NULL
  static StubCodeDesc* desc_for_index(int);      // returns the code descriptor for the index or NULL
  static const char*   name_for(address pc);     // returns the name of the code containing pc or NULL

  StubCodeDesc(const char* group, const char* name, address begin, int framesize) {
    assert(name != NULL, "no name specified");
    _next           = _list;
    _group          = group;
    _name           = name;
    _index          = ++_count;	// (never zero)
_framesize=framesize;
    _begin          = begin;
    _end            = NULL;
    _list           = this;    
  };

  const char* group() const                      { return _group; }
  const char* name() const                       { return _name; }
  int         index() const                      { return _index; }
  address     begin() const                      { return _begin; }
  address     end() const                        { return _end; }
  int         size_in_bytes() const              { return _end - _begin; }
  bool        contains(address pc) const         { return _begin <= pc && pc < _end; }
virtual void print_on(outputStream*os)const;
  void        print_xml_on(xmlBuffer *xb, bool ref=true);
};

// The base class for all stub-generating code generators.
// Provides utility functions.

class StubCodeGenerator: public StackObj {
 protected:
  MacroAssembler*  _masm;

  StubCodeDesc* _first_stub;
  StubCodeDesc* _last_stub;

 public:
  StubCodeGenerator(CodeBlob::BlobType type, const char *name) {
     _masm = new MacroAssembler(Thread::current()->resource_area(), type, 0, name);
     _first_stub = _last_stub = NULL;
  }
  StubCodeGenerator(MacroAssembler *masm)                      {
     _masm = masm;
     _first_stub = _last_stub = NULL;
  }
  ~StubCodeGenerator();         // final masm/codeblob setup
  virtual MacroAssembler* assembler() const { return _masm; }


  virtual void stub_prolog(StubCodeDesc* cdesc); // called by StubCodeMark constructor
  virtual void stub_epilog(StubCodeDesc* cdesc); // called by StubCodeMark destructor
};


// Stack-allocated helper class used to assciate a stub code with a name.
// All stub code generating functions that use a StubCodeMark will be registered
// in the global StubCodeDesc list and the generated stub code can be identified
// later via an address pointing into it.

class StubCodeMark: public StackObj {
 protected:
  StubCodeGenerator* _cgen;
  StubCodeDesc*      _cdesc;
  
 public:
StubCodeMark(StubCodeGenerator*cgen,const char*group,const char*name,int framesize);
  ~StubCodeMark();

};

#endif // STUBCODEGENERATOR_HPP
