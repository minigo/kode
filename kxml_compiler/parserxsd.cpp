/*
    This file is part of KDE.

    Copyright (c) 2006 Cornelius Schumacher <schumacher@kde.org>

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

#include "parserxsd.h"

#include <schema/parser.h>

#include <common/nsmanager.h>
#include <common/messagehandler.h>
#include <common/parsercontext.h>

#include <QDebug>

using namespace RNG;

ParserXsd::ParserXsd ()
    : _verbose ( false )
{
}

void ParserXsd::setVerbose (bool verbose) {
    _verbose = verbose;
}

Schema::Document ParserXsd::parse (const QString &data)
{
    if (_verbose)
        qDebug() << "[ParserXsd][parse] Start parse data";

    NSManager namespaceManager;
    MessageHandler messageHandler;
    ParserContext context;
    context.setNamespaceManager (&namespaceManager);
    context.setMessageHandler (&messageHandler);

    XSD::Parser parser;
    if (!parser.parseString (&context, data)) {
        qDebug() << "[ParserXsd][parse] Error parsing data.";
        return Schema::Document ();
    } else {
        return parse (parser);
    }
}

Schema::Document ParserXsd::parse (QFile &file)
{
    if (_verbose)
        qDebug() << "[ParserXsd][parse] Start parse file" << file.fileName ();

    NSManager namespaceManager;
    MessageHandler messageHandler;
    ParserContext context;
    context.setNamespaceManager( &namespaceManager );
    context.setMessageHandler( &messageHandler );

    XSD::Parser parser;
    if (!parser.parseFile (&context, file)) {
        qDebug () << "[ParserXsd][parse] Error parsing file " << file.fileName();
        return Schema::Document ();
    } else
        return parse (parser);
}

Schema::Document ParserXsd::parse (const XSD::Parser &parser)
{
    XSD::Types types = parser.types ();

    foreach (XSD::Element element, types.elements ())
    {
        //        if (_verbose)
        //            qDebug() << "[ParserXsd][parse] Element:" << element.name ();

        //        Schema::Element e;
        //        e.setIdentifier (element.name ());
        //        e.setName (element.name ());
        //        XSD::ComplexType complexType = types.complexType (element);

        //        if (complexType.contentModel () == XSD::XSDType::MIXED)
        //        {
        //            if (_verbose)
        //                qDebug() << "  Mixed";
        //            e.setText (true);
        //        }
        //        else if (complexType.contentModel () == XSD::XSDType::SIMPLE &&
        //                  !complexType.baseTypeName ().isEmpty())
        //        {
        //            if (complexType.baseTypeName().qname() == "xs:string")
        //                e.setBaseType (Schema::Node::String);
        //            else if (complexType.baseTypeName().qname() == "xs:normalizedString")
        //                e.setBaseType (Schema::Node::NormalizedString);
        //            else if (complexType.baseTypeName().qname() == "xs:token")
        //                e.setBaseType (Schema::Node::Token);
        //        }

        //        if (element.type ().qname () == "xs:string")
        //            e.setType (Schema::Node::String);
        //        else if (element.type ().qname () == "xs:normalizedString")
        //            e.setType (Schema::Node::NormalizedString);
        //        else if (element.type ().qname () == "xs:token")
        //            e.setType (Schema::Node::Token);
        //        else
        //            e.setType (Schema::Node::ComplexType);

        //        foreach (XSD::Element subElement, complexType.elements ())
        //        {
        //            if (_verbose) {
        //                qDebug () << "  Element:" << subElement.name ();
        //                qDebug () << "  " << subElement.minOccurs () << ","
        //                          << subElement.maxOccurs ();
        //            }

        //            Schema::Relation eRelation (subElement.name ());
        //            eRelation.setMinOccurs (subElement.minOccurs ());
        //            if (subElement.maxOccurs () == XSD::Parser::UNBOUNDED)
        //                eRelation.setMaxOccurs (Schema::Relation::Unbounded);
        //            else
        //                eRelation.setMaxOccurs (subElement.maxOccurs ());

        //            XSD::Compositor compositor = subElement.compositor ();
        //            if (_verbose)
        //                qDebug () << "  Compositor" << compositor.type ();

        //            if (compositor.type () == XSD::Compositor::Choice) {
        //                QString choice;
        //                foreach (QName qname, compositor.children ()) {
        //                    if (!choice.isEmpty ())
        //                        choice += '+';

        //                    choice += qname.qname ();
        //                }
        //                eRelation.setChoice (choice);
        //            }
        //            e.addElementRelation (eRelation);
        //        }

        //        foreach (XSD::Attribute attribute, complexType.attributes ())
        //        {
        //            if (_verbose)
        //                qDebug() << "   Attribute:" << attribute.name ();

        //            Schema::Relation aRelation (attribute.name ());
        //            e.addAttributeRelation (aRelation);

        //            Schema::Attribute a;
        //            a.setIdentifier (attribute.name ());
        //            a.setName (attribute.name ());

        //            if (!attribute.type ().isEmpty ()) {
        //                if (attribute.type().qname() == "xs:string")
        //                    a.setType (Schema::Node::String);
        //                else if (attribute.type().qname() == "xs:integer")
        //                    a.setType (Schema::Node::Integer);
        //                else if (attribute.type().qname() == "xs:decimal")
        //                    a.setType (Schema::Node::Decimal);
        //                else if (attribute.type().qname() == "xs:date")
        //                    a.setType (Schema::Node::Date);
        //                else {
        //                    XSD::SimpleType simpleType = types.simpleType (attribute.type ());
        //                    setType (a, simpleType);
        //                }
        //            }

        //            a.setRequired (attribute.isUsed ());
        //            a.setDefaultValue (attribute.defaultValue ());

        //            _document.addAttribute (a);
        //        }

        //        setAnnotations (e, element.annotations ());
        //----------------------------------------------------------------------------

        parseElement (element, types);

        //        XSD::ComplexType complexType = types.complexType (element);
        //        foreach (XSD::Element subElement, complexType.elements ())
        //            parseElement (subElement, types);


        //        XSD::ComplexType complexType = types.complexType (element);
        //        Schema::Element e = parseElement (element, complexType);
        //        _document.addElement (e);

        //        if (_document.startElement ().isEmpty())
        //            _document.setStartElement (e);
    }

    setAnnotations (_document, parser.annotations ());

    if ( _verbose )
        qDebug() << "[ParserXsd][parse] Done";

    return _document;
}

void ParserXsd::setAnnotations (Schema::Annotatable &annotatable, XSD::Annotation::List annotations)
{
    QString documentation;
    QList<QDomElement> domElements;
    foreach (XSD::Annotation a, annotations) {
        if (a.isDocumentation ())
            documentation.append (a.documentation ());
        if (a.isAppinfo ())
            domElements.append (a.domElement ());
    }
    annotatable.setDocumentation (documentation);
    annotatable.setAnnotations (domElements);
}

void ParserXsd::parseElement (const XSD::Element &element, const XSD::Types &types)
{
    if (element.name ().isEmpty ()) {
        qCritical () << "[ParserXsd][parseElement] Empty element name";
        return;
    }

    if (_verbose)
        qDebug() << "[ParserXsd][parse] Element:" << element.name ();

    Schema::Element e;
    e.setIdentifier (element.name ());
    e.setName (element.name ());

    XSD::ComplexType complexType = types.complexType (element);
    if (complexType.contentModel () == XSD::XSDType::MIXED)
    {
        if (_verbose)
            qDebug() << "  Mixed";
        e.setText (true);
    }
    else if (complexType.contentModel () == XSD::XSDType::SIMPLE &&
             !complexType.baseTypeName ().isEmpty())
    {
        if (complexType.baseTypeName().qname() == "xs:string")
            e.setBaseType (Schema::Node::String);
        else if (complexType.baseTypeName().qname() == "xs:normalizedString")
            e.setBaseType (Schema::Node::NormalizedString);
        else if (complexType.baseTypeName().qname() == "xs:token")
            e.setBaseType (Schema::Node::Token);
    }

    if (element.type ().qname () == "xs:string")
        e.setType (Schema::Node::String);
    else if (element.type ().qname () == "xs:normalizedString")
        e.setType (Schema::Node::NormalizedString);
    else if (element.type ().qname () == "xs:token")
        e.setType (Schema::Node::Token);
    else
        e.setType (Schema::Node::ComplexType);

    foreach (XSD::Element subElement, complexType.elements ())
    {
        if (subElement.name ().isEmpty ()) {
            qCritical () << "[ParserXsd][parseElement] Empty subelement name for"
                         << element.name ();
            continue;
        }

        if (_verbose) {
            qDebug () << "  Element:" << subElement.name ();
            qDebug () << "  " << subElement.minOccurs () << ","
                      << subElement.maxOccurs ();
        }

        Schema::Relation eRelation (subElement.name ());
        eRelation.setMinOccurs (subElement.minOccurs ());
        if (subElement.maxOccurs () == XSD::Parser::UNBOUNDED)
            eRelation.setMaxOccurs (Schema::Relation::Unbounded);
        else
            eRelation.setMaxOccurs (subElement.maxOccurs ());

        XSD::Compositor compositor = subElement.compositor ();
        if (_verbose)
            qDebug () << "  Compositor" << compositor.type ();

        if (compositor.type () == XSD::Compositor::Choice) {
            QString choice;
            foreach (QName qname, compositor.children ()) {
                if (!choice.isEmpty ())
                    choice += '+';

                choice += qname.qname ();
            }
            eRelation.setChoice (choice);
        }
        e.addElementRelation (eRelation);

        //-- recursion
        //parseElement (subElement, types);
    }

    foreach (XSD::Attribute attribute, complexType.attributes ())
    {
        if (_verbose)
            qDebug() << "   Attribute:" << attribute.name ();

        Schema::Relation aRelation (attribute.name ());
        e.addAttributeRelation (aRelation);

        Schema::Attribute a;
        a.setIdentifier (attribute.name ());
        a.setName (attribute.name ());

        if (!attribute.type ().isEmpty ()) {
            if (attribute.type().qname() == "xs:string")
                a.setType (Schema::Node::String);
            else if (attribute.type().qname() == "xs:integer")
                a.setType (Schema::Node::Integer);
            else if (attribute.type().qname() == "xs:decimal")
                a.setType (Schema::Node::Decimal);
            else if (attribute.type().qname() == "xs:date")
                a.setType (Schema::Node::Date);
            else {
                XSD::SimpleType simpleType = types.simpleType (attribute.type ());
                setType (a, simpleType);
            }
        }

        a.setRequired (attribute.isUsed ());
        a.setDefaultValue (attribute.defaultValue ());

        _document.addAttribute (a);
    }
    setAnnotations (e, element.annotations ());

    _document.addElement (e);
    if (_document.startElement ().isEmpty())
        _document.setStartElement (e);

    //-- рекурсивно для всех дочерних элементов
    {
        XSD::ComplexType complexType = types.complexType (element);
        foreach (XSD::Element subElement, complexType.elements ())
            parseElement (subElement, types);
    }
}

void ParserXsd::setType (Schema::Node &node, const XSD::SimpleType &simpleType)
{
    if (simpleType.subType () == XSD::SimpleType::TypeRestriction) {
        if ( simpleType.facetType () == XSD::SimpleType::NONE) {
            // Nothing to do.
        } else if (simpleType.facetType () == XSD::SimpleType::ENUM) {
            node.setType (Schema::Node::Enumeration);
            node.setEnumerationValues (simpleType.facetEnums ());
        } else
            qDebug () << "[SimpleType][facetType]" << simpleType.facetType () << "not supported.";
    } else
        qDebug () << "[SimpleType][subType]" << simpleType.subType () << "not supported.";
}
