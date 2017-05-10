/* 
    This file is part of KDE Schema Parser

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
                       based on wsdlpull parser by Vivek Krishna

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

#include "attribute.h"

namespace XSD {

class Attribute::Private
{
public:
    Private ()
        : _qualified (false)
        , _use (false)
    {}

    QName _type;
    QString _documentation;
    QString _defaultValue;
    QString _fixedValue;
    bool _qualified;
    bool _use;
    QName _reference;
    QName _arrayType;
};

Attribute::Attribute ()
    : XmlElement ()
    , d (new Private)
{
}

Attribute::Attribute (const QString &nameSpace)
    : XmlElement (nameSpace)
    , d (new Private)
{
}

Attribute::Attribute (const Attribute &other)
    : XmlElement (other)
    , d (new Private)
{
    *d = *other.d;
}

Attribute::~Attribute ()
{
    delete d;
}

Attribute &Attribute::operator = (const Attribute &other)
{
    if (this == &other)
        return *this;

    *d = *other.d;
    XmlElement::operator=( other );

    return *this;
}

void Attribute::setType (const QName &type) {
    d->_type = type;
}

QName Attribute::type () const {
    return d->_type;
}

void Attribute::setDocumentation (const QString &documentation) {
    d->_documentation = documentation;
}

QString Attribute::documentation () const {
    return d->_documentation;
}

void Attribute::setDefaultValue (const QString &defaultValue) {
    d->_defaultValue = defaultValue;
}

QString Attribute::defaultValue () const {
    return d->_defaultValue;
}

void Attribute::setFixedValue (const QString &fixedValue) {
    d->_fixedValue = fixedValue;
}

QString Attribute::fixedValue () const {
    return d->_fixedValue;
}

void Attribute::setIsQualified (bool isQualified) {
    d->_qualified = isQualified;
}

bool Attribute::isQualified () const {
    return d->_qualified;
}

void Attribute::setIsUsed (bool isUsed) {
    d->_use = isUsed;
}

bool Attribute::isUsed () const {
    return d->_use;
}

void Attribute::setReference (const QName &reference) {
    d->_reference = reference;
}

QName Attribute::reference () const {
    return d->_reference;
}

bool Attribute::isResolved () const {
    return !d->_type.isEmpty ();
}

void Attribute::setArrayType (const QName &arrayType) {
    d->_arrayType = arrayType;
}

QName Attribute::arrayType () const {
    return d->_arrayType;
}

}
