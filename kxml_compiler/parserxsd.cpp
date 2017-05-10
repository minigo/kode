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
    : _verbose (false)
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
    context.setNamespaceManager (&namespaceManager);
    context.setMessageHandler (&messageHandler);

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

    //-- элемента Echelon не нашли в списке элементов?!
    //    QList<XSD::Element> lst = types.elements ();
    //    for (int i = 0; i < lst.size (); ++i) {
    //        if (lst[i].name () == "Echelon") {
    //            qDebug () << lst[i].name () << lst[i].nameSpace () << lst[i].type ().qname ();
    //        }
    //    }

    //-- ищем Echelon в списке элемнтов комплексного типа MOOTWSymbolModifiersType!!!
    //-- HA-HA-HA Echelon найден в списке
    //    QList<XSD::ComplexType> cts = types.complexTypes ();
    //    for (int j = 0; j < cts.size (); ++j)
    //    {
    //        if (cts[j].name () == "MOOTWSymbolModifiersType")
    //        {
    //            QList<XSD::Element> lst = cts[j].elements ();
    //            for (int i = 0; i < lst.size (); ++i) {
    //                if (lst[i].name () == "Echelon")
    //                    qDebug () << "----------------------------> find Echelon";
    //            }
    //        }
    //    }

    //-- ищем SimpleType с типом ENUM - их много, но они никак не связаны с элементами
    //    QList<XSD::SimpleType> lst =  types.simpleTypes ();
    //    for (int i = 0; i < lst.size (); ++i) {
    //        if (lst[i].facetType () == XSD::SimpleType::ENUM)
    //            qDebug () << lst[i].name () << lst[i].nameSpace ();
    //    }

    foreach (XSD::Element element, types.elements ())
        parseElementV1 (element, types);

    setAnnotations (_document, parser.annotations ());

    if ( _verbose )
        qDebug() << "[ParserXsd][parse] Done";

    return _document;


    //---- авторская версия
    //    XSD::Types types = parser.types ();

    //    foreach (XSD::Element element, types.elements ())
    //    {
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
    //                 !complexType.baseTypeName ().isEmpty())
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
    //    }

    //    setAnnotations (_document, parser.annotations ());

    //    if ( _verbose )
    //        qDebug() << "[ParserXsd][parse] Done";

    //    return _document;
    //-----------------------------------------------------
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
    if (element.name () == "Echelon") {
        e.setIdentifier ("Echelon");
        e.setName ("Echelon");
        qDebug () << element.type ().qname ();
        qDebug () << element.groupId ();
        qDebug () << element.qualifiedName().qname();
    } else {
        e.setIdentifier (element.name ());
        e.setName (element.name ());
    }

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
        else if (complexType.baseTypeName().qname().toLower () == "string")
            e.setBaseType (Schema::Node::String);
        else if (complexType.baseTypeName().qname() == "xs:normalizedString")
            e.setBaseType (Schema::Node::NormalizedString);
        else if (complexType.baseTypeName().qname() == "xs:int")
            e.setBaseType (Schema::Node::Integer);
        else if (complexType.baseTypeName().qname() == "xs:integer")
            e.setBaseType (Schema::Node::Integer);
        else if (complexType.baseTypeName().qname() == "xs:long")
            e.setBaseType (Schema::Node::Integer);
        else if (complexType.baseTypeName().qname() == "xs:float")
            e.setBaseType (Schema::Node::Decimal);
        else if (complexType.baseTypeName().qname() == "xs:double")
            e.setBaseType (Schema::Node::Decimal);
        else if (complexType.baseTypeName().qname() == "xs:decimal")
            e.setBaseType (Schema::Node::Decimal);
        else if (complexType.baseTypeName().qname() == "decimal")
            e.setBaseType (Schema::Node::Decimal);
        else if (complexType.baseTypeName().qname() == "xs:boolean")
            e.setBaseType (Schema::Node::Boolean);
        else if (complexType.baseTypeName().qname() == "xs:token")
            e.setBaseType (Schema::Node::Token);
        else if (complexType.baseTypeName().qname() == "token")
            e.setBaseType (Schema::Node::Token);
    }

    if (element.type ().qname () == "xs:string") {
        e.setType (Schema::Node::String);
        e.setText (true);//??
    }
    else if (element.type ().qname ().toLower () == "string") {
        e.setType (Schema::Node::String);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "xs:normalizedString") {
        e.setType (Schema::Node::NormalizedString);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "xs:int") {
        e.setType (Schema::Node::Integer);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "xs:integer") {
        e.setType (Schema::Node::Integer);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "xs:long") {
        e.setType (Schema::Node::Integer);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "xs:float") {
        e.setType (Schema::Node::Decimal);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "xs:double") {
        e.setType (Schema::Node::Decimal);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "xs:decimal") {
        e.setType (Schema::Node::Decimal);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "decimal") {
        e.setType (Schema::Node::Decimal);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "xs:boolean") {
        e.setType (Schema::Node::Boolean);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "xs:token") {
        e.setType (Schema::Node::Token);
        e.setText (true);//??
    }
    else if (element.type ().qname () == "token") {
        e.setType (Schema::Node::Token);
        e.setText (true);//??
    }
    else {
        bool found = false;

        //-- ищем в комплексных типах
        QList<XSD::ComplexType> lst = types.complexTypes ();
        for (int i = 0; i < lst.size (); ++i)
            if (lst[i].name () == element.type ().localName ()) {
                e.setType (Schema::Node::ComplexType);
                found = true;
                break;
            }

        if (!found) {
            //TODO: gim переделать!!!
            //-- не нашли, ищем в простых типах
            QList<XSD::SimpleType> lst = types.simpleTypes ();
            for (int i = 0; i < lst.size (); ++i)
                if (lst[i].name () == element.type ().localName ())
                {
                    //qDebug () << lst[i].baseTypeName ().qname ();
                    //qDebug () << lst[i].subType();
                    //qDebug () << lst[i].facetEnums();


                    if (lst[i].baseTypeName ().qname () == "xs:string") {
                        if (lst[i].facetEnums().isEmpty ())
                            e.setType (Schema::Node::String);
                        else {
                            e.setType (Schema::Node::Enumeration);
                            e.setEnumerationValues (lst[i].facetEnums ());
                        }
                    }
                    else if (lst[i].baseTypeName ().qname ().toLower () == "string") {
                        if (lst[i].facetEnums().isEmpty ())
                            e.setType (Schema::Node::String);
                        else {
                            e.setType (Schema::Node::Enumeration);
                            e.setEnumerationValues (lst[i].facetEnums ());
                        }
                    }
                    else if (lst[i].baseTypeName ().qname () == "xs:normalizedString")
                        e.setType (Schema::Node::NormalizedString);
                    else if (lst[i].baseTypeName ().qname () == "xs:int")
                        e.setType (Schema::Node::Integer);
                    else if (lst[i].baseTypeName ().qname () == "xs:integer")
                        e.setType (Schema::Node::Integer);
                    else if (lst[i].baseTypeName ().qname () == "xs:long")
                        e.setType (Schema::Node::Integer);
                    else if (lst[i].baseTypeName ().qname () == "xs:float")
                        e.setType (Schema::Node::Decimal);
                    else if (lst[i].baseTypeName ().qname () == "xs:double")
                        e.setType (Schema::Node::Decimal);
                    else if (lst[i].baseTypeName ().qname () == "xs:decimal")
                        e.setType (Schema::Node::Decimal);
                    else if (lst[i].baseTypeName ().qname () == "decimal")
                        e.setType (Schema::Node::Decimal);
                    else if (lst[i].baseTypeName ().qname () == "xs:token")
                        e.setType (Schema::Node::Token);
                    else if (lst[i].baseTypeName ().qname () == "token")
                        e.setType (Schema::Node::Token);
                    else if (lst[i].baseTypeName ().qname () == "xs:boolean")
                        e.setType (Schema::Node::Boolean);
                    else {
                        e.setType (Schema::Node::String);
                        qCritical () << "[ParserXsd][parseElement] Could not detect type" << lst[i].baseTypeName ().qname ();
                    }

                    e.setBaseType (e.type ());//??
                    e.setText (true);//??

                    found = true;
                    break;
                }
        }

        if (!found) {
            qCritical () << "[ParserXsd][parseElement] Could not detect element type";
            qCritical () << "   element" << element.name ();
            qCritical () << "   type" << element.type ().qname ();
        }

        //-- authors code
        //e.setType (Schema::Node::ComplexType);
        //--------------------------------------
    }


    foreach (XSD::Element subElement, complexType.elements ())
    {
        if (subElement.name ().isEmpty ()) {
            qCritical () << "[ParserXsd][parseElement] Empty subelement name for"
                         << element.name ();
            continue;
        }

        if (_verbose) {
            qDebug () << "  Element:" << subElement.name () << subElement.minOccurs () << ","
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
            qDebug () << "  Compositor:" << compositor.type ();

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
            if (attribute.type ().qname () == "xs:string")
                a.setType (Schema::Node::String);
            if (attribute.type ().qname ().toLower () == "string")
                a.setType (Schema::Node::String);
            if (attribute.type ().qname () == "xs:ID")
                a.setType (Schema::Node::String);
            if (attribute.type().qname() == "xs:normalizedString")
                a.setType (Schema::Node::String);
            else if (attribute.type ().qname () == "xs:int")
                a.setType (Schema::Node::Integer);
            else if (attribute.type ().qname () == "xs:integer")
                a.setType (Schema::Node::Integer);
            else if (attribute.type().qname() == "xs:long")
                a.setType (Schema::Node::Integer);
            else if (attribute.type().qname() == "xs:float")
                a.setType (Schema::Node::Decimal);
            else if (attribute.type().qname() == "xs:double")
                a.setType (Schema::Node::Decimal);
            else if (attribute.type().qname() == "xs:decimal")
                a.setType (Schema::Node::Decimal);
            else if (attribute.type().qname() == "decimal")
                a.setType (Schema::Node::Decimal);
            else if (attribute.type().qname() == "xs:date")
                a.setType (Schema::Node::Date);
            else if (attribute.type().qname() == "xs:boolean")
                a.setType (Schema::Node::Boolean);
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

void ParserXsd::parseElementV1 (const XSD::Element &element, const XSD::Types &types)
{
    if (element.name ().isEmpty ()) {
        qCritical () << "[ParserXsd][parseElementV1] Empty element name";
        return;
    }

    if (_verbose)
        qDebug() << "[ParserXsd][parseElementV1] Element:" << element.name ();

    Schema::Element e;
    //    if (element.name () == "MilitaryScenario") {
    //        e.setIdentifier ("MilitaryScenario");
    //        e.setName ("MilitaryScenario");
    //        qDebug () << element.type ().qname ();
    //        qDebug () << element.groupId ();
    //        qDebug () << element.qualifiedName().qname();

    //       XSD::ComplexType complextype = types.complexType (element);
    //       qDebug() << complextype.name();
    //       qDebug() << complextype.isValid();
    //       qDebug() << complextype.qualifiedName().qname();
    //       qDebug() << complextype.baseTypeName().qname();

    //    } else {
    e.setIdentifier (element.name ());
    e.setName (element.name ());
    //    }

    //-- флаг что успешно определелили тип элемента
    bool found = false;
    //XSD::ComplexType complextype;

    Schema::Node::Type t = defineXsType (element.type ().qname ());
    if (t != Schema::Node::None) {
        e.setType (t);
        e.setText (true);

        qDebug () << "[ParserXsd][parseElementV1]   Elementary type" << t;
        found = true;
    } else {
        //-- ищем в списке комплексных типов
        XSD::ComplexType complextype = types.complexType (element);

        //        if (element.name() == "MilitaryScenario")
        //        {
        //            qDebug() << element.type().localName();
        //            qDebug() << complextype.isValid();
        //            qDebug() << complextype.name();

        //            XSD::ComplexType::List lst = types.complexTypes();
        //            for (int i = 0; i < lst.size(); ++i)
        //                qDebug() << lst[i].name();

        //            XSD::ComplexType complextype0 = types.complexType (element);
        //            qDebug() << complextype0.isValid();

        //            complextype = types.complexType (element);
        //            qDebug() << complextype.isValid();
        //            qDebug() << complextype.name();
        //        }

        if (complextype.isValid ())
        {
            //-- проверка content model
            if (complextype.contentModel () == XSD::XSDType::MIXED)
            {
                if (_verbose)
                    qDebug () << "[ParserXsd][parseElementV1]   Mixed"
                              << complextype.name ()
                              << complextype.baseTypeName ().qname ()
                              << element.name ();
                e.setText (true);
            }
            else if (complextype.contentModel () == XSD::XSDType::SIMPLE &&
                     !complextype.baseTypeName ().isEmpty ())
            {
                if (element.name() == "version") {
                    qDebug () << complextype.name();
                    qDebug () << complextype.baseTypeName().qname();
                }

                Schema::Node::Type t = defineXsType (complextype.baseTypeName ().qname ());
                if (t != Schema::Node::None) {
                    e.setBaseType (t);
                    found = true;
                } else {
                    XSD::ComplexType::List lst = types.complexTypes ();
                    for (int i = 0; i < lst.size (); ++i) {
                        if (lst[i].name () == complextype.baseTypeName ().localName ()) {
                            e.setBaseType (Schema::Node::ComplexType);
                            break;
                        }
                    }

                    if (e.baseType () == Schema::Node::None) {
                        XSD::SimpleType::List lst = types.simpleTypes ();
                        for (int i = 0; i < lst.size (); ++i) {
                            if (lst[i].name () == complextype.baseTypeName ().localName ()) {
                                Schema::Node::Type t = defineXsType (lst[i].baseTypeName ().qname ());
                                if (t != Schema::Node::None)
                                    e.setBaseType (t);
                                else {
                                    qCritical () << "[ParserXsd][parseElementV1]   JOPA";
                                    qCritical () << "[ParserXsd][parseElementV1]   Could not define base type"
                                                 << complextype.name ()
                                                 << complextype.baseTypeName ().qname ()
                                                 << element.name ();

                                }
                                break;
                            }
                        }
                    }

                    if (e.baseType () == Schema::Node::None) {
                        qCritical () << "[ParserXsd][parseElementV1]   JOPA";
                        qCritical () << "[ParserXsd][parseElementV1]   Could not define base type"
                                     << complextype.name ()
                                     << complextype.baseTypeName ().qname ()
                                     << element.name ();
                    }
                }
            }
            else if (complextype.contentModel () == XSD::XSDType::COMPLEX)
            {
                qWarning () << "[ParserXsd][parseElementV1]   Complex content model. And what to do that?"
                            << complextype.name ()
                            << complextype.baseTypeName ().qname ()
                            << element.name ();
            }

            //-- выставляем тип
            e.setType (Schema::Node::ComplexType);
            found = true;
        }
        else
        {
            XSD::SimpleType simpletype = types.simpleType (element);
            if (simpletype.isValid ())
            {
                Schema::Node::Type t = defineXsType (simpletype.baseTypeName ().qname ());
                if (Schema::Node::None == t) {
                    e.setType (t);
                    qCritical () << "[ParserXsd][parseElementV1]   Undefined simple type"
                                 << simpletype.name ()
                                 << simpletype.baseTypeName ().qname ()
                                 << element.name ();
                }
                else if (Schema::Node::String == t) {
                    if (simpletype.facetEnums ().isEmpty ()) {
                        e.setType (t);
                        e.setText (true);
                    }
                    else {
                        e.setType (Schema::Node::Enumeration);
                        e.setEnumerationValues (simpletype.facetEnums ());
                    }
                    found = true;
                }
                else {
                    e.setType (t);
                    e.setText (true);
                    found = true;
                }
            }
        }
    }

    if (!found) {
        qCritical () << "[ParserXsd][parseElementV1]   Could not detect element type";
        qCritical () << "   element" << element.name ();
        qCritical () << "   type" << element.type ().qname ();
    }

    //-- вот почему это иногда не рботает ?????? заебало!!!!!
    XSD::ComplexType complextype = types.complexType (element);
    foreach (XSD::Element subElement, complextype.elements ())
    {
        if (subElement.name ().isEmpty ()) {
            qCritical () << "[ParserXsd][parseElementV1]   Empty subelement name for"
                         << element.name ();
            continue;
        }

        if (_verbose) {
            qDebug () << "  Element:" << subElement.name () << subElement.minOccurs () << ","
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
            qDebug () << "  Compositor:" << compositor.type ();

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
    }

    foreach (XSD::Attribute attribute, complextype.attributes ())
    {
        if (_verbose)
            qDebug() << "   Attribute:" << attribute.name ();

        Schema::Relation aRelation (attribute.name ());
        e.addAttributeRelation (aRelation);

        Schema::Attribute a;
        a.setIdentifier (attribute.name ());
        a.setName (attribute.name ());

        if (!attribute.type ().isEmpty ()) {
            if (attribute.type ().qname () == "xs:string")
                a.setType (Schema::Node::String);
            if (attribute.type ().qname ().toLower () == "string")
                a.setType (Schema::Node::String);
            if (attribute.type ().qname () == "xs:ID")
                a.setType (Schema::Node::String);
            if (attribute.type().qname() == "xs:normalizedString")
                a.setType (Schema::Node::String);
            else if (attribute.type ().qname () == "xs:int")
                a.setType (Schema::Node::Integer);
            else if (attribute.type ().qname () == "xs:integer")
                a.setType (Schema::Node::Integer);
            else if (attribute.type().qname() == "xs:long")
                a.setType (Schema::Node::Integer);
            else if (attribute.type().qname() == "xs:float")
                a.setType (Schema::Node::Decimal);
            else if (attribute.type().qname() == "xs:double")
                a.setType (Schema::Node::Decimal);
            else if (attribute.type().qname() == "xs:decimal")
                a.setType (Schema::Node::Decimal);
            else if (attribute.type().qname() == "decimal")
                a.setType (Schema::Node::Decimal);
            else if (attribute.type().qname() == "xs:date")
                a.setType (Schema::Node::Date);
            else if (attribute.type().qname() == "xs:boolean")
                a.setType (Schema::Node::Boolean);
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
            parseElementV1 (subElement, types);
    }
}

Schema::Node::Type ParserXsd::defineXsType (const QString &value)
{
    Schema::Node::Type rvalue = Schema::Node::None;

    if (value == "xs:string")
        rvalue = Schema::Node::String;
    else if (value == "xs:normalizedString")
        rvalue = Schema::Node::NormalizedString;
    else if (value == "xs:int")
        rvalue = Schema::Node::Integer;
    else if (value == "xs:integer")
        rvalue = Schema::Node::Integer;
    else if (value == "xs:long")
        rvalue = Schema::Node::Integer;
    else if (value == "xs:float")
        rvalue = Schema::Node::Decimal;
    else if (value == "xs:double")
        rvalue = Schema::Node::Decimal;
    else if (value == "xs:decimal")
        rvalue = Schema::Node::Decimal;
    else if (value == "xs:boolean")
        rvalue = Schema::Node::Boolean;
    else if (value == "xs:token")
        rvalue = Schema::Node::Token;
    else if (value == "xs:date")
        rvalue = Schema::Node::Date;

    return rvalue;
}

//bool ParserXsd::defineBaseTypeV1 (const QString &value, Schema::Element &e)
//{
//    if (value == "xs:string")
//        e.setBaseType (Schema::Node::String);
//    else if (value == "String")
//        e.setBaseType (Schema::Node::String);
//    else if (value == "xs:normalizedString")
//        e.setBaseType (Schema::Node::NormalizedString);
//    else if (value == "xs:int")
//        e.setBaseType (Schema::Node::Integer);
//    else if (value == "xs:integer")
//        e.setBaseType (Schema::Node::Integer);
//    else if (value == "xs:long")
//        e.setBaseType (Schema::Node::Integer);
//    else if (value == "xs:float")
//        e.setBaseType (Schema::Node::Decimal);
//    else if (value == "xs:double")
//        e.setBaseType (Schema::Node::Decimal);
//    else if (value == "xs:decimal")
//        e.setBaseType (Schema::Node::Decimal);
//    else if (value == "decimal")
//        e.setBaseType (Schema::Node::Decimal);
//    else if (value == "xs:boolean")
//        e.setBaseType (Schema::Node::Boolean);
//    else if (value == "xs:token")
//        e.setBaseType (Schema::Node::Token);
//    else if (value == "token")
//        e.setBaseType (Schema::Node::Token);
//    else {
//        qCritical () << "[ParserXsd][setBaseTypeV1] Could not detect base type"
//                     << value;
//        return false;
//    }

//    return true;
//}

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
