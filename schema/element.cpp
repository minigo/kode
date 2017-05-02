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

#include "element.h"

namespace XSD {

class Element::Private
{
public:
    Private()
        : _minOccurs (1)
        , _maxOccurs (1)
        , _qualified (false)
        , _occurrence (0)
    {}

    QName _type;
    QString _documentation;
    int _groupId;
    int _minOccurs;
    int _maxOccurs;
    bool _qualified;
    QString _defaultValue;
    QString _fixedValue;
    int _occurrence;
    QName _reference;
    Compositor _compositor;
};

Element::Element ()
    : XmlElement (), d(new Private)
{
}

Element::Element (const QString &nameSpace)
    : XmlElement (nameSpace )
    , d (new Private)
{
}

Element::Element (const Element &other )
    : XmlElement (other)
    , d (new Private)
{
    *d = *other.d;
}

Element::~Element () {
    delete d;
}

Element &Element::operator = (const Element &other)
{
    if ( this == &other )
        return *this;

    *d = *other.d;
    XmlElement::operator=( other );

    return *this;
}

void Element::setType (const QName &type) {
    d->_type = type;
}

QName Element::type () const {
    return d->_type;
}

void Element::setDocumentation (const QString &documentation) {
    d->_documentation = documentation;
}

QString Element::documentation () const {
    return d->_documentation;
}

void Element::setGroupId (int group) {
    d->_groupId = group;
}

int Element::groupId () const{
    return d->_groupId;
}

void Element::setMinOccurs (int minOccurs) {
    d->_minOccurs = minOccurs;
}

int Element::minOccurs() const {
    return d->_minOccurs;
}

void Element::setMaxOccurs (int maxOccurs) {
    d->_maxOccurs = maxOccurs;
}

int Element::maxOccurs () const {
    return d->_maxOccurs;
}

void Element::setDefaultValue (const QString &defaultValue) {
    d->_defaultValue = defaultValue;
}

QString Element::defaultValue () const {
    return d->_defaultValue;
}

void Element::setFixedValue (const QString &fixedValue) {
    d->_fixedValue = fixedValue;
}

QString Element::fixedValue () const {
    return d->_fixedValue;
}

void Element::setIsQualified (bool isQualified) {
    d->_qualified = isQualified;
}

bool Element::isQualified() const
{
    return d->_qualified;
}

void Element::setOccurrence (int occurrence) {
    d->_occurrence = occurrence;
}

int Element::occurrence () const {
    return d->_occurrence;
}

void Element::setReference (const QName &reference) {
    d->_reference = reference;
}

QName Element::reference () const {
    return d->_reference;
}

bool Element::isResolved () const {
    return !d->_type.isEmpty();
}

void Element::setCompositor (const Compositor &c) {
    d->_compositor = c;
}

Compositor Element::compositor () const {
    return d->_compositor;
}

}
