// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "searchhandlerproxy.h"
#include "documentdb.h"
#include <vespa/searchlib/engine/searchreply.h>
#include <vespa/searchlib/engine/docsumreply.h>

#include <vespa/log/log.h>
LOG_SETUP(".proton.server.searchhandlerproxy");

namespace proton {

SearchHandlerProxy::SearchHandlerProxy(const DocumentDB::SP &documentDB)
    : _documentDB(documentDB)
{
    _documentDB->retain();
}

SearchHandlerProxy::~SearchHandlerProxy()
{
    _documentDB->release();
}

std::unique_ptr<search::engine::DocsumReply>
SearchHandlerProxy::getDocsums(const DocsumRequest & request)
{
    return _documentDB->getDocsums(request);
}

std::unique_ptr<search::engine::SearchReply>
SearchHandlerProxy::match(const ISearchHandler::SP &searchHandler,
                          const SearchRequest &req,
                          vespalib::ThreadBundle &threadBundle) const
{
    return _documentDB->match(searchHandler, req, threadBundle);
}

} // namespace proton
