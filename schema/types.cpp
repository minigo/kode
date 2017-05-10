/* 
    This file is part of KDE Schema Parser

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

#include "types.h"

#include <QDebug>

namespace XSD {

class Types::Private
{
public:
    SimpleType::List _simpleTypes;
    ComplexType::List _complexTypes;
    Element::List _elements;
    Attribute::List _attributes;
    AttributeGroup::List _attributeGroups;
    QStringList _namespaces;
};

Types::Types ()
    : d (new Private)
{
}

Types::Types (const Types &other)
    : d (new Private)
{
    *d = *other.d;
}

Types::~Types () {
    delete d;
}

Types &Types::operator = (const Types &other)
{
    if ( this == &other )
        return *this;

    *d = *other.d;

    return *this;
}

void Types::setSimpleTypes (const SimpleType::List &simpleTypes) {
    d->_simpleTypes = simpleTypes;
}

SimpleType::List Types::simpleTypes () const {
    return d->_simpleTypes;
}

void Types::setComplexTypes (const ComplexType::List &complexTypes) {
    d->_complexTypes = complexTypes;
}

ComplexType::List Types::complexTypes () const {
    return d->_complexTypes;
}

void Types::setElements (const Element::List &elements) {
    d->_elements = elements;
}

Element::List Types::elements () const {
    return d->_elements;
}

void Types::setAttributes (const Attribute::List &attributes) {
    d->_attributes = attributes;
}

Attribute::List Types::attributes () const {
    return d->_attributes;
}

void Types::setAttributeGroups (const AttributeGroup::List &attributeGroups) {
    d->_attributeGroups = attributeGroups;
}

AttributeGroup::List Types::attributeGroups () const {
    return d->_attributeGroups;
}

void Types::setNamespaces (const QStringList &namespaces) {
    d->_namespaces = namespaces;
}

QStringList Types::namespaces () const {
    return d->_namespaces;
}

ComplexType Types::complexType (const Element &element) const
{
    foreach (ComplexType type, d->_complexTypes) {
        //---- now
        if (element.type ().localName () == type.name ())
            return type;
        //---- was/author code
        //if (element.type () == type.name ())
        //    return type;
        //-------------------------------------

    }

    return ComplexType ();
}

SimpleType Types::simpleType (const Element &element) const
{
    foreach (SimpleType type, d->_simpleTypes) {
        if (element.type ().localName () == type.name ())
            return type;
    }

    return SimpleType ();
}

SimpleType Types::simpleType (const QName &typeName) const
{
    foreach (SimpleType type, d->_simpleTypes) {
        if (type.qualifiedName () == typeName)
            return type;
    }

    qCritical () << "[Types][simpleType] Type not found" << typeName.qname ();
    return SimpleType ();
}

}
