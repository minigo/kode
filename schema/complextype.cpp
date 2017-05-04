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

#include "complextype.h"

namespace XSD {

class ComplexType::Private
{
public:
    Private ()
        : _anonymous (false)
        , _isArray (false)
    {}

    QString _documentation;

    Element::List _elements;
    Attribute::List _attributes;
    AttributeGroup::List _attributeGroups;

    bool _anonymous;
    bool _isArray;

    Derivation _baseDerivation;
    QName _baseTypeName;
};

ComplexType::ComplexType (const QString &nameSpace)
    : XSDType (nameSpace)
    , d (new Private)
{
}

ComplexType::ComplexType ()
    : XSDType ()
    , d (new Private)
{
}

ComplexType::ComplexType (const ComplexType &other)
    : XSDType (other)
    , d (new Private)
{
    *d = *other.d;
}

ComplexType::~ComplexType () {
    delete d;
}

ComplexType &ComplexType::operator=( const ComplexType &other )
{
    if (this == &other)
        return *this;

    *d = *other.d;
    return *this;
}

void ComplexType::setDocumentation (const QString &documentation) {
    d->_documentation = documentation;
}

QString ComplexType::documentation () const {
    return d->_documentation;
}

void ComplexType::setBaseTypeName (const QName &baseTypeName) {
    d->_baseTypeName = baseTypeName;
}

QName ComplexType::baseTypeName () const {
    return d->_baseTypeName;
}

void ComplexType::setBaseDerivation (Derivation derivation) {
    d->_baseDerivation = derivation;
}

ComplexType::Derivation ComplexType::baseDerivation () const {
    return d->_baseDerivation;
}

bool ComplexType::isSimple () const {
    return false;
}

void ComplexType::setIsArray (bool isArray) {
    d->_isArray = isArray;
}

bool ComplexType::isArray () const {
    return d->_isArray;
}

void ComplexType::setAnonymous (bool anonymous) {
    d->_anonymous = anonymous;
}

bool ComplexType::isAnonymous () const {
    return d->_anonymous;
}

void ComplexType::setElements (const Element::List &elements) {
    d->_elements = elements;
}

Element::List ComplexType::elements () const {
    return d->_elements;
}

void ComplexType::setAttributes (const Attribute::List &attributes) {
    d->_attributes = attributes;
}

Attribute::List ComplexType::attributes () const {
    return d->_attributes;
}

void ComplexType::setAttributeGroups (const AttributeGroup::List &attributeGroups) {
    d->_attributeGroups = attributeGroups;
}

AttributeGroup::List ComplexType::attributeGroups () const {
    return d->_attributeGroups;
}

void ComplexType::addAttribute (const Attribute &attribute) {
    d->_attributes.append (attribute);
}

void ComplexType::addElement (const Element &element) {
    d->_elements.append (element);
}

bool ComplexType::isValid () {
    if (d->_baseTypeName.qname ().isEmpty ())
        return false;
    return true;
}

}
