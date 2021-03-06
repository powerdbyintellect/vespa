// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.model.builder.xml.dom;

import com.yahoo.text.XML;
import com.yahoo.config.model.producer.AbstractConfigProducer;
import com.yahoo.vespa.model.container.component.Component;
import com.yahoo.vespa.model.container.component.Handler;
import org.w3c.dom.Element;

/**
 * @author gjoranv
 * @since 5.1.6
 */
public class DomClientProviderBuilder extends DomHandlerBuilder {

    @Override
    protected Handler doBuild(AbstractConfigProducer ancestor, Element clientElement) {
        Handler<? super Component<?, ?>> client = getHandler(clientElement);

        for (Element binding : XML.getChildren(clientElement, "binding"))
            client.addClientBindings(XML.getValue(binding));

        for (Element serverBinding : XML.getChildren(clientElement, "serverBinding"))
            client.addServerBindings(XML.getValue(serverBinding));

        DomComponentBuilder.addChildren(ancestor, clientElement, client);

        return client;
    }
}
