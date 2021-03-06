// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "errorpolicy.h"
#include <vespa/documentapi/messagebus/documentprotocol.h>
#include <vespa/messagebus/emptyreply.h>

namespace documentapi {

ErrorPolicy::ErrorPolicy(const string &msg) :
    _msg(msg)
{}

void
ErrorPolicy::select(mbus::RoutingContext &ctx)
{
    ctx.setError(DocumentProtocol::ERROR_POLICY_FAILURE, _msg);
}

void
ErrorPolicy::merge(mbus::RoutingContext &)
{
    abort();
}

}
