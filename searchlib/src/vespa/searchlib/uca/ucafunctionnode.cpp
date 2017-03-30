// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/searchlib/uca/ucafunctionnode.h>
#include <vespa/searchlib/uca/ucaconverter.h>

namespace search {
namespace expression {

using vespalib::FieldBase;
using vespalib::Serializer;
using vespalib::Deserializer;

IMPLEMENT_EXPRESSIONNODE(UcaFunctionNode, UnaryFunctionNode);

UcaFunctionNode::UcaFunctionNode()
{
}

UcaFunctionNode::~UcaFunctionNode()
{
}

UcaFunctionNode::UcaFunctionNode(ExpressionNode::UP arg, const vespalib::string & locale, const vespalib::string & strength) :
    UnaryFunctionNode(std::move(arg)),
    _locale(locale),
    _strength(strength),
    _collator(new uca::UcaConverter(locale, strength))
{
}

UcaFunctionNode::UcaFunctionNode(const UcaFunctionNode & rhs) :
    UnaryFunctionNode(rhs),
    _locale(rhs._locale),
    _strength(rhs._strength),
    _collator(rhs._collator),
    _handler()
{
}

UcaFunctionNode & UcaFunctionNode::operator = (const UcaFunctionNode & rhs)
{
    if (this != &rhs) {
        UnaryFunctionNode::operator =(rhs);
        _locale = rhs._locale;
        _strength = rhs._strength;
        _collator = rhs._collator;
        _handler.reset();
    }
    return *this;
}

void UcaFunctionNode::onPrepareResult()
{
    if (getArg().getResult().inherits(ResultNodeVector::classId)) {
        setResultType(std::unique_ptr<ResultNode>(new RawResultNodeVector));
        _handler.reset(new MultiValueHandler(*this));
    } else {
        setResultType(std::unique_ptr<ResultNode>(new RawResultNode));
        _handler.reset(new SingleValueHandler(*this));
    }
}

UcaFunctionNode::Handler::Handler(const UcaFunctionNode & uca) :
    _converter(*uca._collator),
    _backingBuffer(),
    _buffer(_backingBuffer, sizeof(_backingBuffer))
{
}

void UcaFunctionNode::Handler::handleOne(const ResultNode & arg, RawResultNode & result) const
{
    vespalib::ConstBufferRef buf = _converter.convert(arg.getString(_buffer));
    result.set(RawResultNode(buf.c_str(), buf.size()));
}

bool UcaFunctionNode::onExecute() const
{
    getArg().execute();
    _handler->handle(getArg().getResult());
    return true;
}

void UcaFunctionNode::SingleValueHandler::handle(const ResultNode & arg)
{
    handleOne(arg, _result);
}

void UcaFunctionNode::MultiValueHandler::handle(const ResultNode & arg)
{
    const ResultNodeVector & v(static_cast<const ResultNodeVector &>(arg));
   _result.getVector().resize(v.size());
    for(size_t i(0), m(_result.getVector().size()); i < m; i++) {
        handleOne(v.get(i), _result.getVector()[i]);
    }
}

Serializer & UcaFunctionNode::onSerialize(Serializer & os) const
{
    UnaryFunctionNode::onSerialize(os);
    return os << _locale << _strength;
}

Deserializer & UcaFunctionNode::onDeserialize(Deserializer & is)
{
    UnaryFunctionNode::onDeserialize(is);
    is >> _locale >> _strength;
    _collator.reset(new uca::UcaConverter(_locale, _strength));
    return is;
}

}
}

// this function was added by ../../forcelink.sh
void forcelink_file_searchlib_expression_ucafunctionnode() {}
