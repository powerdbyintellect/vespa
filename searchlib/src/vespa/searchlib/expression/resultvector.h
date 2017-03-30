// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "enumresultnode.h"
#include "integerresultnode.h"
#include "floatresultnode.h"
#include "stringresultnode.h"
#include "rawresultnode.h"
#include "integerbucketresultnode.h"
#include "floatbucketresultnode.h"
#include "stringbucketresultnode.h"
#include "rawbucketresultnode.h"
#include <vespa/vespalib/objects/visit.hpp>
#include <algorithm>

namespace search {
namespace expression {

class ResultNodeVector : public ResultNode
{
public:
    DECLARE_ABSTRACT_EXPRESSIONNODE(ResultNodeVector);
    DECLARE_RESULTNODE_SERIALIZE;
    typedef std::unique_ptr<ResultNodeVector> UP;
    typedef vespalib::IdentifiablePtr<ResultNodeVector> CP;
    virtual const ResultNode * find(const ResultNode & key) const = 0;
    virtual ResultNodeVector & push_back(const ResultNode & node) = 0;
    virtual ResultNodeVector & push_back_safe(const ResultNode & node) = 0;
    virtual const ResultNode & get(size_t index) const = 0;
    virtual ResultNodeVector & set(size_t index, const ResultNode & node) = 0;
    virtual ResultNode & get(size_t index) = 0;
    virtual void clear() = 0;
    virtual void resize(size_t sz) = 0;
    size_t size() const { return onSize(); }
    bool  empty() const { return size() == 0; }
    /**
     * Sum yourself to the argument
     * @param result the argument
     */
    virtual ResultNode & flattenMultiply(ResultNode & r) const { return r; }
    virtual ResultNode & flattenSum(ResultNode & r) const { return r; }
    virtual ResultNode & flattenMax(ResultNode & r) const { return r; }
    virtual ResultNode & flattenMin(ResultNode & r) const { return r; }
    virtual ResultNode & flattenAnd(ResultNode & r) const { return r; }
    virtual ResultNode &  flattenOr(ResultNode & r) const { return r; }
    virtual ResultNode & flattenXor(ResultNode & r) const { return r; }
    virtual void min(const ResultNode & b) { (void) b; }
    virtual void max(const ResultNode & b) { (void) b; }
    virtual void add(const ResultNode & b) { (void) b; }
private:
    virtual size_t onSize() const = 0;
    virtual void set(const ResultNode & rhs) { (void) rhs; }
    virtual bool isMultiValue() const { return true; }
};

template <typename B>
struct cmpT {
    struct less : public std::binary_function<B, B, bool> {
        bool operator()(const B & a, const B & b) { return a.cmp(b) < 0; }
    };
    struct equal : public std::binary_function<B, B, bool> {
        bool operator()(const B & a, const B & b) { return a.cmp(b) == 0; }
    };
};

template <typename B, typename V>
struct contains {
    struct less : public std::binary_function<B, V, bool> {
        bool operator()(const B & a, const V & b) { return a.contains(b) < 0; }
    };
    struct equal : public std::binary_function<B, V, bool> {
        bool operator()(const B & a, const V & b) { return a.contains(b) == 0; }
    };
};

template <typename B, typename C, typename G>
class ResultNodeVectorT : public ResultNodeVector
{
public:
    DECLARE_NBO_SERIALIZE;
    using Vector = std::vector<B>;
    using BaseType = B;
    const Vector & getVector() const { return _result; }
    Vector & getVector() { return _result; }
    virtual const ResultNode * find(const ResultNode & key) const;
    virtual void sort();
    virtual void reverse();
    virtual ResultNodeVector & push_back(const ResultNode & node);
    virtual ResultNodeVector & push_back_safe(const ResultNode & node);
    virtual ResultNodeVector & set(size_t index, const ResultNode & node);
    virtual const ResultNode & get(size_t index) const { return _result[index]; }
    virtual ResultNode & get(size_t index) { return _result[index]; }
    virtual void clear() { _result.clear(); }
    virtual void resize(size_t sz) { _result.resize(sz); }
    virtual void negate();
private:
    virtual void visitMembers(vespalib::ObjectVisitor &visitor) const { visit(visitor, "Vector", _result); }
    virtual size_t onSize() const { return _result.size(); }
    virtual const vespalib::Identifiable::RuntimeClass & getBaseClass() const { return B::_RTClass; }
    virtual int64_t onGetInteger(size_t index) const { return _result[index].getInteger(index); }
    virtual double onGetFloat(size_t index)    const { return _result[index].getFloat(index); }
    virtual ConstBufferRef onGetString(size_t index, BufferRef buf) const { return  _result[index].getString(index, buf); }
    virtual size_t hash() const;
    virtual int onCmp(const Identifiable & b) const;
    Vector _result;
};

template <typename B, typename C, typename G>
ResultNodeVector & ResultNodeVectorT<B, C, G>::set(size_t index, const ResultNode & node)
{
    _result[index].set(node);
    return *this;
}

template <typename B, typename C, typename G>
ResultNodeVector & ResultNodeVectorT<B, C, G>::push_back_safe(const ResultNode & node)
{
    if (node.inherits(B::classId)) {
        _result.push_back(static_cast<const B &>(node));
    } else {
        B value;
        value.set(node);
        _result.push_back(value);
    }
    return *this;
}

template <typename B, typename C, typename G>
ResultNodeVector & ResultNodeVectorT<B, C, G>::push_back(const ResultNode & node)
{
    _result.push_back(static_cast<const B &>(node));
    return *this;
}

template <typename B, typename C, typename G>
int ResultNodeVectorT<B, C, G>::onCmp(const Identifiable & rhs) const
{
    const ResultNodeVectorT & b(static_cast<const ResultNodeVectorT &>(rhs));
    int diff = _result.size() - b._result.size();
    for (size_t i(0), m(_result.size()); (diff == 0) && (i < m); i++) {
        diff = _result[i].cmp(b._result[i]);
    }
    return diff;
}

template <typename B, typename C, typename G>
void ResultNodeVectorT<B, C, G>::sort()
{
    typedef cmpT<B> LC;
    std::sort(_result.begin(), _result.end(), typename LC::less());
}

template <typename B, typename C, typename G>
void ResultNodeVectorT<B, C, G>::reverse()
{
    std::reverse(_result.begin(), _result.end());
}

template <typename B, typename C, typename G>
size_t ResultNodeVectorT<B, C, G>::hash() const
{
    size_t h(0);
    for(typename Vector::const_iterator it(_result.begin()), mt(_result.end()); it != mt; it++) {
        h ^= it->hash();
    }
    return h;
}

template <typename B, typename C, typename G>
void ResultNodeVectorT<B, C, G>::negate()
{
    for(typename Vector::iterator it(_result.begin()), mt(_result.end()); it != mt; it++) {
        it->negate();
    }
}

template <typename B, typename C, typename G>
const ResultNode * ResultNodeVectorT<B, C, G>::find(const ResultNode & key) const
{
    G getter;
    typename Vector::const_iterator found = std::lower_bound(_result.begin(), _result.end(), getter(key), typename C::less() );
    if (found != _result.end()) {
        typename C::equal equal;
        return equal(*found, getter(key)) ? &(*found) : NULL;
    }
    return NULL;
}

template <typename B, typename C, typename G>
vespalib::Serializer & ResultNodeVectorT<B, C, G>::onSerialize(vespalib::Serializer & os) const
{
    return serialize(_result, os);
}

template <typename B, typename C, typename G>
vespalib::Deserializer & ResultNodeVectorT<B, C, G>::onDeserialize(vespalib::Deserializer & is)
{
    return deserialize(_result, is);
}

struct GetInteger {
    int64_t operator () (const ResultNode & r) { return r.getInteger(); }
};

struct GetFloat {
    double operator () (const ResultNode & r) { return r.getFloat(); }
};

struct GetString {
    ResultNode::BufferRef _tmp;
    ResultNode::ConstBufferRef operator () (const ResultNode & r) { return r.getString(_tmp); }
};

template <typename B>
class NumericResultNodeVectorT : public ResultNodeVectorT<B, cmpT<ResultNode>, std::_Identity<ResultNode> >
{
public:
    virtual ResultNode & flattenMultiply(ResultNode & r) const {
        B v;
        v.set(r);
        const std::vector<B> & vec(this->getVector());
        for(size_t i(0), m(vec.size()); i < m; i++) {
            v.multiply(vec[i]);
        }
        r.set(v);
        return r;
    }
    virtual ResultNode & flattenAnd(ResultNode & r) const {
        Int64ResultNode v;
        v.set(r);
        const std::vector<B> & vec(this->getVector());
        for(size_t i(0), m(vec.size()); i < m; i++) {
            v.andOp(vec[i]);
        }
        r.set(v);
        return r;
    }
    virtual ResultNode & flattenOr(ResultNode & r) const {
        Int64ResultNode v;
        v.set(r);
        const std::vector<B> & vec(this->getVector());
        for(size_t i(0), m(vec.size()); i < m; i++) {
            v.orOp(vec[i]);
        }
        r.set(v);
        return r;
    }
    virtual ResultNode & flattenXor(ResultNode & r) const {
        Int64ResultNode v;
        v.set(r);
        const std::vector<B> & vec(this->getVector());
        for(size_t i(0), m(vec.size()); i < m; i++) {
            v.xorOp(vec[i]);
        }
        r.set(v);
        return r;
    }
    virtual ResultNode & flattenSum(ResultNode & r) const {
        B v;
        v.set(r);
        const std::vector<B> & vec(this->getVector());
        for(size_t i(0), m(vec.size()); i < m; i++) {
            v.add(vec[i]);
        }
        r.set(v);
        return r;
    }
    virtual ResultNode & flattenMax(ResultNode & r) const {
        B v;
        v.set(r);
        const std::vector<B> & vec(this->getVector());
        for(size_t i(0), m(vec.size()); i < m; i++) {
            v.max(vec[i]);
        }
        r.set(v);
        return r;
    }
    virtual ResultNode & flattenMin(ResultNode & r) const {
        B v;
        v.set(r);
        const std::vector<B> & vec(this->getVector());
        for(size_t i(0), m(vec.size()); i < m; i++) {
            v.min(vec[i]);
        }
        r.set(v);
        return r;
    }

};

class Int8ResultNodeVector : public NumericResultNodeVectorT<Int8ResultNode>
{
public:
    Int8ResultNodeVector() { }
    DECLARE_RESULTNODE(Int8ResultNodeVector);

    virtual const IntegerBucketResultNode& getNullBucket() const override { return IntegerBucketResultNode::getNull(); }
};

class Int16ResultNodeVector : public NumericResultNodeVectorT<Int16ResultNode>
{
public:
    Int16ResultNodeVector() { }
    DECLARE_RESULTNODE(Int16ResultNodeVector);

    virtual const IntegerBucketResultNode& getNullBucket() const override { return IntegerBucketResultNode::getNull(); }
};

class Int32ResultNodeVector : public NumericResultNodeVectorT<Int32ResultNode>
{
public:
    Int32ResultNodeVector() { }
    DECLARE_RESULTNODE(Int32ResultNodeVector);

    virtual const IntegerBucketResultNode& getNullBucket() const override { return IntegerBucketResultNode::getNull(); }
};

class Int64ResultNodeVector : public NumericResultNodeVectorT<Int64ResultNode>
{
public:
    Int64ResultNodeVector() { }
    DECLARE_RESULTNODE(Int64ResultNodeVector);

    virtual const IntegerBucketResultNode& getNullBucket() const override { return IntegerBucketResultNode::getNull(); }
};

typedef Int64ResultNodeVector IntegerResultNodeVector;

class EnumResultNodeVector : public NumericResultNodeVectorT<EnumResultNode>
{
public:
    EnumResultNodeVector() {}
    DECLARE_RESULTNODE(EnumResultNodeVector);
};

class FloatResultNodeVector : public NumericResultNodeVectorT<FloatResultNode>
{
public:
    FloatResultNodeVector() { }
    DECLARE_RESULTNODE(FloatResultNodeVector);

    virtual const FloatBucketResultNode& getNullBucket() const override { return FloatBucketResultNode::getNull(); }
};

class StringResultNodeVector : public ResultNodeVectorT<StringResultNode, cmpT<ResultNode>, std::_Identity<ResultNode> >
{
public:
    StringResultNodeVector() { }
    DECLARE_RESULTNODE(StringResultNodeVector);

    virtual const StringBucketResultNode& getNullBucket() const override { return StringBucketResultNode::getNull(); }
};

class RawResultNodeVector : public ResultNodeVectorT<RawResultNode, cmpT<ResultNode>, std::_Identity<ResultNode> >
{
public:
    RawResultNodeVector() { }
    DECLARE_RESULTNODE(RawResultNodeVector);

    virtual const RawBucketResultNode& getNullBucket() const override { return RawBucketResultNode::getNull(); }
};

class IntegerBucketResultNodeVector : public ResultNodeVectorT<IntegerBucketResultNode, contains<IntegerBucketResultNode, int64_t>, GetInteger >
{
public:
    IntegerBucketResultNodeVector() { }
    DECLARE_RESULTNODE(IntegerBucketResultNodeVector);
};

class FloatBucketResultNodeVector : public ResultNodeVectorT<FloatBucketResultNode, contains<FloatBucketResultNode, double>, GetFloat >
{
public:
    FloatBucketResultNodeVector() { }
    DECLARE_RESULTNODE(FloatBucketResultNodeVector);
};

class StringBucketResultNodeVector : public ResultNodeVectorT<StringBucketResultNode, contains<StringBucketResultNode, ResultNode::ConstBufferRef>, GetString >
{
public:
    StringBucketResultNodeVector() { }
    DECLARE_RESULTNODE(StringBucketResultNodeVector);
};

class RawBucketResultNodeVector : public ResultNodeVectorT<RawBucketResultNode, contains<RawBucketResultNode, ResultNode::ConstBufferRef>, GetString >
{
public:
    RawBucketResultNodeVector() { }
    DECLARE_RESULTNODE(RawBucketResultNodeVector);
};

class GeneralResultNodeVector : public ResultNodeVector
{
public:
    DECLARE_EXPRESSIONNODE(GeneralResultNodeVector);
    virtual const ResultNode * find(const ResultNode & key) const;
    virtual ResultNodeVector & push_back(const ResultNode & node) { _v.push_back(node); return *this; }
    virtual ResultNodeVector & push_back_safe(const ResultNode & node) { _v.push_back(node); return *this; }
    virtual const ResultNode & get(size_t index) const { return *_v[index]; };
    virtual ResultNodeVector & set(size_t index, const ResultNode & node) { _v[index] = node; return *this; }
    virtual ResultNode & get(size_t index) { return *_v[index]; }
    virtual void clear() { _v.clear(); }
    virtual void resize(size_t sz) { _v.resize(sz); }
private:
    virtual int64_t onGetInteger(size_t index) const { return _v[index]->getInteger(index); }
    virtual double onGetFloat(size_t index)    const { return _v[index]->getFloat(index); }
    virtual ConstBufferRef onGetString(size_t index, BufferRef buf) const { return  _v[index]->getString(index, buf); }
    virtual size_t hash() const;
    virtual size_t onSize() const { return _v.size(); }
    std::vector<ResultNode::CP> _v;
};


}
}

