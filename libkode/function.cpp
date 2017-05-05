/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2009 David Faure <dfaure@kdab.com>

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

#include <QtCore/QStringList>

#include "function.h"

using namespace KODE;

class Function::Argument::ArgumentPrivate
{
public:
    QString declaration;
    QString defaultArgument;
};

Function::Argument::Argument( const QString &declaration,
                              const QString &defaultArgument )
    : d( new ArgumentPrivate )
{
    d->declaration = declaration;
    d->defaultArgument = defaultArgument;
}

QString Function::Argument::headerDeclaration() const
{
    if ( d->defaultArgument.isEmpty() ) {
        return d->declaration;
    } else {
        return d->declaration + " = " + d->defaultArgument;
    }
}

QString Function::Argument::bodyDeclaration() const
{
    return d->declaration;
}


class Function::FunctionPrivate
{
public:
    FunctionPrivate()
        : _access( Public ), _isConst( false ), _isStatic( false ), _virtualMode( NotVirtual )
    {
    }

    int _access;
    bool _isConst;
    bool _isStatic;
    QString _returnType;
    QString _name;
    Argument::List _arguments;
    QStringList _initializers;
    QString _body;
    QString _docs;
    Function::VirtualMode _virtualMode;
};

Function::Function ()
    : d (new FunctionPrivate)
{
}

Function::Function (const Function &other)
    : d (new FunctionPrivate)
{
    *d = *other.d;
}

Function::Function (const QString &name, const QString &returnType,
                    int access, bool isStatic )
    : d (new FunctionPrivate)
{
    d->_access = access;
    d->_isStatic = isStatic;
    d->_name = name;
    setReturnType(returnType);
}

Function::~Function()
{
    delete d;
}

Function& Function::operator=( const Function &other )
{
    if ( this == &other )
        return *this;

    *d = *other.d;

    return *this;
}

void Function::setConst( bool isConst )
{
    d->_isConst = isConst;
}

bool Function::isConst() const
{
    return d->_isConst;
}

void Function::setStatic( bool isStatic )
{
    d->_isStatic = isStatic;
}

bool Function::isStatic() const
{
    return d->_isStatic;
}

void Function::addArgument( const Function::Argument &argument )
{
    d->_arguments.append( argument );
}

void Function::addArgument (const QString &argument, const QString &defaultArgument) {
    d->_arguments.append (Argument (argument, defaultArgument));
}

void Function::setArgumentString( const QString &argumentString )
{
    d->_arguments.clear();

    const QStringList arguments = argumentString.split( "," );
    QStringList::ConstIterator it;
    for ( it = arguments.constBegin(); it != arguments.constEnd(); ++it ) {
        addArgument( *it );
    }
}

Function::Argument::List Function::arguments() const
{
    return d->_arguments;
}

void Function::addInitializer( const QString &initializer )
{
    d->_initializers.append( initializer );
}

QStringList Function::initializers() const
{
    return d->_initializers;
}

void Function::setBody( const QString &body )
{
    d->_body = body;
}

void Function::setBody( const Code &body )
{
    d->_body = body.text();
}

void Function::addBodyLine( const QString &bodyLine )
{
    d->_body.append( bodyLine );
    if ( bodyLine.right( 1 ) != "\n" )
        d->_body.append( '\n' );
}

QString Function::body() const
{
    return d->_body;
}

void Function::setAccess( int access )
{
    d->_access = access;
}

int Function::access() const
{
    return d->_access;
}

QString Function::accessAsString() const
{
    QString access;

    if ( d->_access & Public )
        access = "public";
    if ( d->_access & Protected )
        access = "protected";
    if ( d->_access & Private )
        access = "private";

    if ( d->_access & Signal )
        access = "signals";
    if ( d->_access & Slot )
        access += " slots";

    return access;
}

void Function::setReturnType( const QString &returnType )
{
    Q_ASSERT(returnType != "*");
    d->_returnType = returnType;
}

QString Function::returnType() const
{
    return d->_returnType;
}

void Function::setName( const QString &name )
{
    d->_name = name;
}

QString Function::name() const
{
    return d->_name;
}

void Function::setDocs (const QString &docs) {
    d->_docs = docs;
}

QString Function::docs () const {
    return d->_docs;
}

bool Function::hasArguments() const
{
    return !d->_arguments.isEmpty();
}

void Function::setVirtualMode( Function::VirtualMode v )
{
    d->_virtualMode = v;
}

Function::VirtualMode Function::virtualMode() const
{
    return d->_virtualMode;
}
