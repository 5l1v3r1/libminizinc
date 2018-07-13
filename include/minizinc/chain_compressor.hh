/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __MINIZINC_CHAIN_COMPRESSOR_H__
#define __MINIZINC_CHAIN_COMPRESSOR_H__

#include <map>
#include <minizinc/model.hh>

namespace MiniZinc {

  class ChainCompressor {
  public:
    ChainCompressor(EnvI &env, Model &m, std::vector<VarDecl *> &deletedVarDecls)
        : env(env), m(m), deletedVarDecls(deletedVarDecls) {};

    virtual bool trackItem(Item *i) = 0;

    virtual void compress() = 0;

  protected:
    EnvI &env;
    Model &m;
    std::vector<VarDecl *> &deletedVarDecls;

    std::multimap<VarDecl *, Item *> items;
    typedef std::multimap<VarDecl *, Item *>::iterator iterator;

    void storeItem(VarDecl *v, Item *i) { items.emplace(v, i); }

    void updateCount();

    unsigned long count(VarDecl *v) { return items.count(v); }

    std::pair<iterator, iterator> find(VarDecl *v) {return items.equal_range(v);};

    void removeItem(Item *i);
    void addItem(Item *i);

    // Replaces the Nth argument of a Call c by Expression e, c must be located on Item i
    void replaceCallArgument(Item *i, Call *c, unsigned int n, Expression *e);
  };

  class ImpCompressor : public ChainCompressor {
  public:
    ImpCompressor(EnvI &env, Model &m, std::vector<VarDecl *> &deletedVarDecls)
        : ChainCompressor(env, m, deletedVarDecls) {};

    bool trackItem(Item *i) override;

    void compress() override;

  protected:
    // Compress two implications. e.g. (x -> y) /\ (y -> z) => x -> z
    // In this case i: (y -> z), newLHS: x
    // Function returns true if compression was successful (and the implication that contains newLHS can be removed)
    // Side effect: Item i might be removed.
    bool compressItem(Item *i, VarDecl *newLHS);

    // Constructs a clause constraint item with pos and neg as parameters.
    // if pos/neg are not ArrayLit then they will inserted into an ArrayLit.
    ConstraintI *constructClause(Expression *pos, Expression *neg);

    ConstraintI *constructHalfReif(Call *call, Id *control);
  };

  class LECompressor : public ChainCompressor {
  public:
    LECompressor(EnvI &env, Model &m, std::vector<VarDecl *> &deletedVarDecls)
    : ChainCompressor(env, m, deletedVarDecls) {};

    bool trackItem(Item *i) override;

    void compress() override;

  protected:
    // Compress inequalities. e.g. (x <= y) /\ (y <= z) => x <= z
    // In this case i: (y <= z), newLHS: x
    // Function returns true if compression was successful (and the implication that contains newLHS can be removed)
    // Side effect: Item i might be removed.
    bool compressItem(Item *i, VarDecl *newLHS);

    ConstraintI *ConstructLE(Expression *lhs, Expression *rhs);
  };

}

#endif //__MINIZINC_CHAIN_COMPRESSOR_H__
