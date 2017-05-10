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

#include "xmlelement.h"

namespace XSD {

class XmlElement::Private
{
public:
    QString _name;
    QString _nameSpace;

    Annotation::List _annotations;
};

XmlElement::XmlElement ()
    : d (new Private)
{
}

XmlElement::XmlElement (const QString &nameSpace)
    : d (new Private)
{
    d->_nameSpace = nameSpace;
}

XmlElement::XmlElement (const XmlElement &other)
    : d (new Private)
{
    *d = *other.d;
}

XmlElement::~XmlElement ()
{
    delete d;
}

XmlElement &XmlElement::operator = (const XmlElement &other)
{
    if (this == &other)
        return *this;

    *d = *other.d;
    return *this;
}

void XmlElement::setName (const QString &name) {
    d->_name = name;
}

QString XmlElement::name () const {
    return d->_name;
}

void XmlElement::setNameSpace (const QString &nameSpace) {
    d->_nameSpace = nameSpace;
}

QString XmlElement::nameSpace () const {
    return d->_nameSpace;
}

QName XmlElement::qualifiedName () const {
    return QName (d->_nameSpace, d->_name);
}

void XmlElement::addAnnotation (const Annotation &a) {
    d->_annotations.append (a);
}

void XmlElement::setAnnotations (const Annotation::List &l) {
    d->_annotations = l;
}

Annotation::List XmlElement::annotations () const {
    return d->_annotations;
}

}
