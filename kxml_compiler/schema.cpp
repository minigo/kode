/*
    This file is part of KDE.

    Copyright (c) 2006 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "schema.h"

#include <QDebug>

using namespace Schema;

Document::Document () {}

void Document::setStartElement (const Element &e) {
    _startElement = e;
}

Element Document::startElement () const {
    return _startElement;
}

void Document::addElement (const Element &e) {
    _elements.append (e);
}

Element::List Document::elements () const {
    return _elements;
}

bool Document::hasElement (const Element &element)
{
    foreach(Element e, _elements) {
        if (e.identifier () == element.identifier ())
            return true;
    }
    return false;
}

Element Document::element (const QString &identifier) const
{
    foreach (Element e, _elements)
        if (e.identifier () == identifier)
            return e;

    return Element ();
}

Element Document::element (const Relation &relation) const {
    return element (relation.target ());
}

Element::List Document::usedElements () const
{
    _usedElements.clear ();
    findUsedElements (_startElement);
    return _usedElements;
}

void Document::findUsedElements (const Element &e) const
{
//    if (e.name () == "MOOTWSymbolModifiers") {
//        //а внутри Echelon
//        qDebug () << e.name () << e.baseType ();
//    }

    addUsedElement (e);

    foreach (Relation r, e.elementRelations ()) {
        Element e2 = element (r);

        //-- оказывается это перебор
        //if (e2.isEmpty ()) {
        //    qWarning () << "[Document][findUsedElements] Empty element for relation" << r.target ();
        //    continue;
        //}

        if (!e2.isValid ()) {
            qWarning () << "[Document][findUsedElements] Element is not valid for relation" << r.target ();
            continue;
        }

        if ((!e2.mixed () || r.isList ()) && addUsedElement (e2))
            findUsedElements (e2);
    }

//------------------------------------- вариант автора
//    addUsedElement (e);
//    foreach (Relation r, e.elementRelations ()) {
//        Element e2 = element (r);
//        if (e2.isEmpty ()) {
//            qWarning () << "[Document][findUsedElements] Empty element for relation" << r.target ();
//            continue;
//        }
//        if (!e2.isValid ()) {
//            qWarning () << "[Document][findUsedElements] Element is not valid for relation" << r.target ();
//            continue;
//        }
//        if ((!e2.mixed () || r.isList ()) && addUsedElement (e2))
//            findUsedElements (e2);
//    }
}

bool Document::addUsedElement (const Element &element) const
{
    bool found = false;
    foreach (Element usedElement, _usedElements) {
        if (usedElement.identifier() == element.identifier ()) {
            found = true;
            break;
        }
    }
    if (!found) {
        _usedElements.prepend (element);
        return true;
    } else {
        return false;
    }
}

void Document::addAttribute (const Attribute &a) {
    _attributes.append (a);
}

Attribute::List Document::attributes () const {
    return _attributes;
}

bool Document::hasAttribute (const Attribute &attribute)
{
    foreach (Attribute a, _attributes) {
        if (a.identifier() == attribute.identifier())
            return true;
    }
    return false;
}

Attribute Document::attribute( const QString &identifier ) const
{
    foreach (Attribute a, _attributes) {
        if (a.identifier() == identifier)
            return a;
    }
    return Attribute ();
}

Attribute Document::attribute (const Relation &relation) const {
    return attribute (relation.target());
}

bool Document::isEmpty () const {
    return _elements.isEmpty ();
}

void Document::dump () const
{
    foreach (Element e, _elements) {
        qDebug () << "[Document][dump] ELEMENT" << e.identifier() << ":" << e.name() << e.type();
        if (e.text ())
            qDebug() << "[Document][dump] TEXT";
        if (e.mixed ())
            qDebug() << "[Document][dump] MIXED";

        foreach (Relation r, e.elementRelations ())
            qDebug() << "[Document][dump]" << r.asString ("ELEMENT");

        foreach (Relation r, e.attributeRelations ())
            qDebug() << "[Document][dump]" << r.asString ("ATTRIBUTE");
    }

    foreach (Attribute a, _attributes)
        qDebug () << "[Document][dump] ATTRIBUTE " << a.identifier() << ": " << a.name() << a.type();
}

Relation::Relation ()
    : _minOccurs (1)
    , _maxOccurs (1)
{
}

Relation::Relation (const QString &target)
    : _target (target)
    , _minOccurs (1)
    , _maxOccurs (1)
{
}

void Relation::setMinOccurs (int v) {
    _minOccurs = v;
}

void Relation::setMaxOccurs (int v) {
    _maxOccurs = v;
}

int Relation::minOccurs () const {
    return _minOccurs;
}

int Relation::maxOccurs () const {
    return _maxOccurs;
}

bool Relation::isOptional () const {
    return _minOccurs == 0 && _maxOccurs == 1;
}

bool Relation::isRequired () const {
    return _minOccurs == 1 && _maxOccurs == 1;
}

bool Relation::isList () const {
    return _maxOccurs > 1 || _maxOccurs == Unbounded;
}

void Relation::setTarget (const QString &identifier) {
    _target = identifier;
}

QString Relation::target () const {
    return _target;
}

void Relation::setChoice (const QString &choice) {
    _choice = choice;
}

QString Relation::choice () const {
    return _choice;
}

QString Relation::asString (const QString &type) const
{
    QString out = "     Relation" + type + ":" + target () + "(" +
            QString::number (minOccurs ()) + ',';
    if (maxOccurs() == Relation::Unbounded)
        out += "UNBOUNDED";
    else
        out += QString::number (maxOccurs ());
    out += ')';

    return out;
}


Node::Node ()
    : _type (String)
    , _baseType (None)
{
}

Node::~Node ()
{
}

void Node::setType (Type t) {
    _type = t;
}

Node::Type Node::type () const {
    return _type;
}

void Node::setIdentifier (const QString &i) {
    _identifier = i;
}

QString Node::identifier () const {
    return _identifier;
}

void Node::setName (const QString &n) {
    _name = n;
}

QString Node::name () const {
    return _name;
}

void Node::setBaseType (Type type) {
    _baseType = type;
}

Node::Type Node::baseType () const {
    return _baseType;
}

bool Node::isValid () const {
    return !_identifier.isEmpty () && !_name.isEmpty ();
}

void Node::setEnumerationValues (const QStringList &v) {
    _enumerationValues = v;
}

QStringList Node::enumerationValues () const {
    return _enumerationValues;
}


void Annotatable::setDocumentation (const QString &str) {
    _documentation = str;
}

QString Annotatable::documentation () const {
    return _documentation;
}

void Annotatable::setAnnotations (const QList<QDomElement> &a) {
    _annotations = a;
}

QList<QDomElement> Annotatable::annotations () const {
    return _annotations;
}


Element::Element ()
    : _text (false)
{
}

QString Element::ref () const {
    return name ();
}

bool Element::hasRelation (const QString &identifier) const {
    return hasElementRelation (identifier) || hasAttributeRelation (identifier);
}

void Element::addElementRelation (const Relation &r) {
    _elementRelations.append (r);
}

bool Element::hasElementRelation (const Element &element) const
{
    Relation::List::ConstIterator it;
    for( it = _elementRelations.begin(); it != _elementRelations.end(); ++it ) {
        if ( (*it).target() == element.identifier() ) return true;
    }
    return false;
}

bool Element::hasElementRelation (const QString &identifier) const
{
    Relation::List::ConstIterator it;
    for( it = _elementRelations.begin(); it != _elementRelations.end(); ++it ) {
        if ( (*it).target() == identifier ) return true;
    }
    return false;
}

Relation &Element::elementRelation (const Element &element)
{
    Relation::List::Iterator it;
    for (it = _elementRelations.begin(); it != _elementRelations.end(); ++it) {
        if ((*it).target() == element.identifier())
            return *it;
    }
    return _nullRelation;
}

Relation::List Element::elementRelations () const {
    return _elementRelations;
}

bool Element::hasElementRelations () const {
    return !_elementRelations.isEmpty ();
}

void Element::addAttributeRelation (const Relation &r) {
    //if (name () == "ScenarioID") {
    //    qDebug () << r.asString ();
    //}
    _attributeRelations.append (r);
}

bool Element::hasAttributeRelation (const Attribute &attribute) const
{
    Relation::List::ConstIterator it;
    for (it = _attributeRelations.begin (); it != _attributeRelations.end (); ++it) {
        if ((*it).target() == attribute.identifier ())
            return true;
    }
    return false;
}

bool Element::hasAttributeRelation (const QString &identifier) const
{
    Relation::List::ConstIterator it;
    for( it = _attributeRelations.begin(); it != _attributeRelations.end(); ++it ) {
        if ( (*it).target() == identifier ) return true;
    }
    return false;
}

Relation::List Element::attributeRelations () const {
    return _attributeRelations;
}

bool Element::hasAttributeRelations () const {
    return !_attributeRelations.isEmpty();
}

bool Element::mixed () const {
    return _text && !_elementRelations.isEmpty();
}

void Element::setText (bool text) {
    _text = text;
}

bool Element::text () const {
    return _text;
}

bool Element::isEmpty () const {
    return !_text && _elementRelations.isEmpty ();
}

Attribute::Attribute () {}

bool Attribute::required () const {
    return _required;
}

void Attribute::setRequired (bool required) {
    _required = required;
}

QString Attribute::defaultValue () const {
    return _defVal;
}

void Attribute::setDefaultValue (const QString defVal) {
    _defVal = defVal;
}

QString Attribute::ref () const {
    return '@' + name();
}
