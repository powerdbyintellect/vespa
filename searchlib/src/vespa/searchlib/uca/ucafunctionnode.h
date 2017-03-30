// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/expression/unaryfunctionnode.h>
#include <vespa/searchlib/common/sortspec.h>
#include <vespa/searchlib/expression/stringresultnode.h>
#include <vespa/searchlib/expression/resultvector.h>


namespace search {
namespace expression {

class UcaFunctionNode : public UnaryFunctionNode
{
public:
    DECLARE_EXPRESSIONNODE(UcaFunctionNode);
    DECLARE_NBO_SERIALIZE;
    UcaFunctionNode();
    ~UcaFunctionNode();
    UcaFunctionNode(ExpressionNode::UP arg, const vespalib::string & locale, const vespalib::string & strength);
    UcaFunctionNode(const UcaFunctionNode & rhs);
    UcaFunctionNode & operator = (const UcaFunctionNode & rhs);
private:
    virtual bool onExecute() const;
    virtual void onPrepareResult();
    class Handler {
    public:
        Handler(const UcaFunctionNode & uca);
        virtual ~Handler() { }
        virtual void handle(const ResultNode & arg) = 0;
    protected:
        void handleOne(const ResultNode & arg, RawResultNode & result) const;
    private:
        const common::BlobConverter & _converter;
        char                          _backingBuffer[32];
        vespalib::BufferRef           _buffer;
    };
    class SingleValueHandler : public Handler {
    public:
        SingleValueHandler(UcaFunctionNode & uca) : Handler(uca), _result(static_cast<RawResultNode &>(uca.updateResult())) { }
        virtual void handle(const ResultNode & arg);
    private:
        RawResultNode & _result;
    };
    class MultiValueHandler : public Handler {
    public:
        MultiValueHandler(UcaFunctionNode & uca) : Handler(uca), _result(static_cast<RawResultNodeVector &>(uca.updateResult())) { }
        virtual void handle(const ResultNode & arg);
    private:
        RawResultNodeVector & _result;
    };
    vespalib::string          _locale;
    vespalib::string          _strength;
    common::BlobConverter::SP _collator;
    std::unique_ptr<Handler>    _handler;
};

}
}

