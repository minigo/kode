/*
    This file is part of Kung.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QFile>

#include <common/fileprovider.h>
#include <common/parsercontext.h>
#include <common/messagehandler.h>
#include <wsdl/definitions.h>

#include "dispatcher.h"

#include "loader.h"

Loader::Loader ()
    : QObject (0x00)
{
    setObjectName ("KWSDL::Loader");
    _context = new ParserContext ();
}

void Loader::setWSDLUrl (const QString &wsdlUrl)
{
    _wsdlUrl = wsdlUrl;
    _wsdlBaseUrl = _wsdlUrl.left (_wsdlUrl.lastIndexOf ('/'));

    _context->setDocumentBaseUrl (_wsdlBaseUrl);
}

void Loader::run()
{
    download();
}

void Loader::download()
{
    FileProvider provider;

    QString fileName;
    if ( provider.get( _wsdlUrl, fileName ) ) {
        QFile file( fileName );
        if ( !file.open( QIODevice::ReadOnly ) ) {
            qDebug( "Unable to download wsdl file %s", qPrintable( _wsdlUrl ) );
            provider.cleanUp();
            return;
        }

        QString errorMsg;
        int errorLine, errorCol;
        QDomDocument doc;
        if ( !doc.setContent( &file, true, &errorMsg, &errorLine, &errorCol ) ) {
            qDebug( "%s at (%d,%d)", qPrintable( errorMsg ), errorLine, errorCol );
            return;
        }

        parse( doc.documentElement() );

        provider.cleanUp();
    }
}

void Loader::parse( const QDomElement &element )
{
    KWSDL::Definitions def;
    KWSDL::WSDL kwsdl;

    NSManager namespaceManager;
    MessageHandler messageHandler;
    _context->setNamespaceManager(&namespaceManager);
    _context->setMessageHandler(&messageHandler);

    def.loadXML( _context, element );

    kwsdl.setDefinitions(def);

    execute(kwsdl);
}

void Loader::execute(const KWSDL::WSDL &wsdl)
{
    mDispatcher = new Dispatcher;
    mDispatcher->setWSDL( wsdl );

    mDispatcher->run();
}

#include "loader.moc"
