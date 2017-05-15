/*
    This file is part of KDE.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "creator.h"

#include "namer.h"

#include "parsercreatordom.h"
#include "writercreator.h"

#include <libkode/code.h>
#include <libkode/printer.h>
#include <libkode/typedef.h>
#include <libkode/statemachine.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <QFile>
#include <QTextStream>
#include <qdom.h>
#include <QRegExp>
#include <QMap>
#include <QList>

#include <iostream>

Creator::ClassFlags::ClassFlags (const Schema::Element &element)
{
    _hasId = element.hasRelation( "id" );
    _hasUpdatedTimestamp = element.hasRelation( "updated_at" );
}

bool Creator::ClassFlags::hasUpdatedTimestamp () const {
    return _hasUpdatedTimestamp;
}

bool Creator::ClassFlags::hasId () const {
    return _hasId;
}


Creator::Creator (const Schema::Document &document, XmlParserType p)
    : _document (document)
    , _xmlParserType (p)
    , _verbose (false)
    , _useKde (false)
{
    setExternalClassNames ();
}

// FIXME: Handle creation options via flags enum.

void Creator::setVerbose (bool verbose) {
    _verbose = verbose;
}

void Creator::setUseKde (bool useKde) {
    _useKde = useKde;
}

bool Creator::useKde () const {
    return _useKde;
}

void Creator::setCreateCrudFunctions (bool enabled) {
    _createCrudFunctions = enabled;
}

void Creator::setLicense (const KODE::License &l) {
    _file.setLicense (l);
}

void Creator::setExportDeclaration (const QString &name) {
    _exportDeclaration = name;
}

void Creator::setExternalClassPrefix (const QString &prefix)
{
    _externalClassPrefix = prefix;
    setExternalClassNames();
}

void Creator::setExternalClassNames()
{
    _parserClass.setName( _externalClassPrefix + "Parser" );
    _writerClass.setName( _externalClassPrefix + "Writer" );
}

KODE::File &Creator::file () {
    return _file;
}

void Creator::createProperty (KODE::Class &c,
                              const ClassDescription &description, const QString &type,
                              const QString &name )
{
    if (type.startsWith ("Q"))
        c.addHeaderInclude (type);

    KODE::MemberVariable v (Namer::getClassName (name), type);
    c.addMemberVariable (v);

    KODE::Function mutator (Namer::getMutator (name), "void");
    if (type == "int" || type == "double" || type == "float" || type == "bool") {
        mutator.addArgument (type + " v");
    } else {
        mutator.addArgument ("const " + type + " &v");
    }
    mutator.addBodyLine (v.name() + " = v;");
    if (_createCrudFunctions) {
        if (name != "UpdatedAt" && name != "CreatedAt") {
            if ( description.hasProperty("UpdatedAt")) {
                mutator.addBodyLine ("setUpdatedAt (QDateTime::currentDateTime ());");
            }
        }
    }
    c.addFunction (mutator);

    KODE::Function accessor (Namer::getAccessor (name), type);
    accessor.setConst (true);
    if (type.right (4) == "Enum") {
        accessor.setReturnType (c.name () + "::" + type);
    }

    accessor.addBodyLine ("return " + v.name() + ';');
    c.addFunction (accessor);
}

void Creator::createCrudFunctions (KODE::Class &c, const QString &type)
{
    if (!c.hasEnum ("Flags")) {
        QStringList enumValues;
        enumValues << "None" << "AutoCreate";
        KODE::Enum flags ("Flags", enumValues);
        c.addEnum (flags);
    }

    KODE::Function finder ("find" + type, type);

    finder.addArgument ("const QString &id");
    finder.addArgument (KODE::Function::Argument ("Flags flags", "Flags_None"));

    //-- now
    QString ntype = type;
    ntype[0].toLower ();
    QString listMember = "_" + ntype + "List";
    //-- was
    //QString listMember = "m" + type + "List";
    //-----------------

    KODE::Code code;
    code += "foreach (" + type + " v, " + listMember + ") {";
    code += "   if (v.id () == id) return v;";
    code += "}";
    code += type + " v;";
    code += "if (flags == Flags_AutoCreate) {";
    code += "   v.setId (id);";
    code += "}";
    code += "return v;";

    finder.setBody( code );

    c.addFunction( finder );

    KODE::Function inserter ("insert", "bool");

    code.clear();

    inserter.addArgument ("const " + type + " &v");

    code += "int i = 0;";
    code += "for ( ; i < " + listMember + ".size(); ++i) {";
    code += "   if (" + listMember + "[i].id () == v.id ()) {";
    code += "       " + listMember + "[i] = v;";
    code += "       return true;";
    code += "   }";
    code += "}";
    code += "if (i == " + listMember + ".size ()) {";
    code += "   add" + type + "(v);";
    code += "}";
    code += "return true;";

    inserter.setBody( code );

    c.addFunction( inserter );

    KODE::Function remover ("remove", "bool");

    remover.addArgument ("const " + type + " &v");

    code.clear();

    code += type + "::List::Iterator it;";
    code += "for (it = " + listMember + ".begin (); it != " + listMember +
            ".end (); ++it) {";
    code += "   if ((*it).id () == v.id ()) break;";
    code += "}";
    code += "if (it != " + listMember + ".end ()) {";
    code += "   " + listMember + ".erase (it);";
    code += "}";
    code += "return true;";

    remover.setBody (code);

    c.addFunction (remover);
}

//TODO: gim переработать вариант с определением типа
ClassDescription Creator::createClassDescription (const Schema::Element &element)
{
    ClassDescription description (Namer::getClassName (element));

    //-- отладка
    //if (element.name () == "ScenarioID") {
    //    Schema::Relation::List lst = element.attributeRelations ();
    //    for (int i = 0; i < lst.size (); ++i)
    //        qDebug () << lst[i].asString ();
    //}

    foreach (Schema::Relation r, element.attributeRelations ()) {
        Schema::Attribute a = _document.attribute (r);
        if (a.enumerationValues ().count ()) {
            if (!description.hasEnum (a.name ()))
                description.addEnum (KODE::Enum (Namer::getClassName (a.name ()) + "Enum", a.enumerationValues ()));

            description.addProperty (Namer::getClassName (a.name ()) + "Enum",
                                     Namer::getClassName (a.name ()));
        } else {
            description.addProperty (typeName (a.type ()),
                                     Namer::getClassName (a.name ()));
        }
    }

    if (element.text ())
        description.addProperty (typeName (element.type ()), "Value");

    foreach (Schema::Relation r, element.elementRelations ())
    {
        Schema::Element targetElement = _document.element (r);
        QString targetClassName = Namer::getClassName (targetElement);

        if (targetElement.type () == Schema::Element::Enumeration)
        {
            description.addProperty (targetClassName, targetClassName);
        }
        else if (targetElement.text () && !targetElement.hasAttributeRelations () && !r.isList ())
        {
            //if (_verbose)
            //    qDebug() << "  FLATTEN";

            if (targetElement.type () == Schema::Element::Integer) {
                description.addProperty ("int", targetClassName);
            } else if (targetElement.type () == Schema::Element::Decimal) {
                description.addProperty ("double", targetClassName);
            } else if (targetElement.type () == Schema::Element::Boolean) {
                description.addProperty ("bool", targetClassName);
            } else if (targetElement.type () == Schema::Element::Date) {
                description.addProperty ("QDate", targetClassName);
            } else {
                description.addProperty ("QString", targetClassName);
            }
        } else {
            if (!_file.hasClass (targetClassName))
                createClass (targetElement);

            QString name = KODE::Style::lowerFirst( targetClassName );

            if ( r.isList() ) {
                ClassProperty p( targetClassName, Namer::getClassName( name ) );
                p.setIsList( true );

                ClassFlags targetClassFlags( targetElement );
                if ( targetClassFlags.hasId() ) {
                    p.setTargetHasId( true );
                }

                description.addProperty( p );
            } else {
                description.addProperty( targetClassName, Namer::getClassName( name ) );
            }
        }
    }

    return description;
}

void Creator::createClass (const Schema::Element &element)
{
    if (!element.isValid ()) {
        qCritical () << "[Creator][createClass] Could not create class for the invalid element"
                     << element.name ()
                     << element.identifier ();
        return;
    }

    QString className = Namer::getClassName (element);
    if (className.isEmpty ()) {
        qCritical () << "[Creator][createClass] Could not create class"
                     << element.name ()
                     << element.identifier ();
        return;
    }

//    if (className == "Echelon") {
//        qDebug () << element.name ();
//        qDebug () << element.type ();
//        qDebug () << element.text ();
//    }

    if (_verbose) {
        qDebug () << "[Creator][createClass]" << element.identifier() << className;
        foreach (Schema::Relation r, element.elementRelations ())
            qDebug() << "   SUBELEMENTS" << r.target();
    }

    if (_processedClasses.contains (className)) {
        if (_verbose)
            qDebug() << "   ALREADY DONE" << className;
        return;
    }

    _processedClasses.append (className);

    ClassDescription description = createClassDescription (element);
    KODE::Class c (className);

    if (!_exportDeclaration.isEmpty ())
        c.setExportDeclaration (_exportDeclaration);

    bool hasCreatedAt = description.hasProperty ("CreatedAt");
    bool hasUpdatedAt = description.hasProperty ("UpdatedAt");

    if (_createCrudFunctions) {
        if (hasCreatedAt || hasUpdatedAt) {
            KODE::Function constructor (className, "");
            KODE::Code code;
            code += "QDateTime now = QDateTime::currentDateTime ();";
            if (hasCreatedAt)
                code += "setCreatedAt (now);";
            if (hasUpdatedAt)
                code += "setUpdatedAt (now);";
            constructor.setBody (code);
            c.addFunction (constructor);
        }

        if (description.hasProperty ("Id")) {
            KODE::Function isValid( "isValid", "bool" );
            isValid.setConst( true );
            KODE::Code code;
            code += "return !mId.isEmpty();";
            isValid.setBody( code );
            c.addFunction( isValid );
        }
    }

    // для отладки
    //if (c.name () == "ScenarioID") {
    //    ClassProperty::List lst = description.properties ();
    //    for (int i =0; i < lst.size (); ++i) {
    //        qDebug () << lst[i].name ();
    //    }
    //}

    foreach (ClassProperty p, description.properties ())
    {
        if (p.isList ()) {
            registerListTypedef (p.type ());

            c.addHeaderInclude ("QList");
            QString listName = p.name () + "List";

            KODE::Function adder ("add" + p.type (), "void");
            adder.addArgument ("const " + p.type () + " &v");

            KODE::Code code;
            code += '_' + KODE::Style::lowerFirst (listName) + ".append (v);";

            adder.setBody (code);

            c.addFunction (adder);

            createProperty (c, description, p.type () + "::List", listName);

            if (_createCrudFunctions && p.targetHasId ())
                createCrudFunctions (c, p.type ());
        } else {
            createProperty (c, description, p.type (), p.name ());
        }
    }

    foreach (KODE::Enum e, description.enums ())
        c.addEnum (e);

    createElementParser (c, element);

    WriterCreator writerCreator (_file, _document, _dtd);
    writerCreator.createElementWriter (c, element);

    _file.insertClass (c);
}

void Creator::createEnumeration (const Schema::Element &element)
{
    if (!element.isValid ()) {
        qCritical () << "[Creator][createEnumeration] Could not create class for the invalid element"
                     << element.name ()
                     << element.identifier ();
        return;
    }

    QString className = Namer::getClassName (element);
    if (className.isEmpty ()) {
        qCritical () << "[Creator][createEnumeration] Could not create class"
                     << element.name ()
                     << element.identifier ();
        return;
    }

    KODE::Enum e (element.name (), element.enumerationValues ());
    _file.addFileEnum (e);
}

void Creator::createElementParser (KODE::Class &c, const Schema::Element &e)
{
    ParserCreator *parserCreator = 0;

    switch ( _xmlParserType ) {
    case XmlParserDom:
    case XmlParserDomExternal:
        parserCreator = new ParserCreatorDom( this );
        break;
    }

    if ( parserCreator == 0 ) {
        return;
    }

    parserCreator->createElementParser( c, e );

    delete parserCreator;
}

void Creator::registerListTypedef (const QString &type)
{
    if (!_listTypedefs.contains (type))
        _listTypedefs.append (type);
}

void Creator::createListTypedefs ()
{
    QStringList::ConstIterator it;
    for( it = _listTypedefs.constBegin(); it != _listTypedefs.constEnd(); ++it ) {
        KODE::Class c = _file.findClass( *it );
        if ( !c.isValid() ) continue;
        c.addTypedef( KODE::Typedef( "QList<" + *it + '>', "List" ) );
        _file.insertClass( c );
    }
}

void Creator::setDtd (const QString &dtd) {
    _dtd = dtd;
}

void Creator::createFileWriter (const Schema::Element &element)
{
    WriterCreator writerCreator( _file, _document, _dtd );
    writerCreator.createFileWriter( Namer::getClassName( element ),
                                    errorStream() );
}

void Creator::createFileParser( const Schema::Element &element )
{
    ParserCreator *parserCreator = 0;

    switch ( _xmlParserType ) {
    case XmlParserDom:
    case XmlParserDomExternal:
        parserCreator = new ParserCreatorDom( this );
        break;
    }

    parserCreator->createFileParser( element );
    parserCreator->createStringParser( element );

    delete parserCreator;
}

void Creator::printFiles (KODE::Printer &printer)
{
    if (externalParser ()) {
        KODE::File parserFile( file() );
        parserFile.setFilename( _baseName + "_parser" );

        parserFile.clearCode();

        _parserClass.addHeaderInclude( file().filenameHeader() );
        parserFile.insertClass( _parserClass );

        if ( _verbose ) {
            qDebug() <<"Print external parser header" << parserFile.filenameHeader();
        }
        printer.printHeader( parserFile );
        if ( _verbose ) {
            qDebug() <<"Print external parser implementation"
                    << parserFile.filenameImplementation();
        }
        printer.printImplementation( parserFile );
    }

    if ( _verbose ) {
        qDebug() <<"Print header" << file().filenameHeader();
    }
    printer.printHeader( file() );

    if ( _verbose ) {
        qDebug() <<"Print implementation" << file().filenameImplementation();
    }
    printer.printImplementation( file() );

}

bool Creator::externalParser () const {
    return _xmlParserType == XmlParserDomExternal;
}

bool Creator::externalWriter () const {
    return false;
}

const Schema::Document &Creator::document () const {
    return _document;
}

void Creator::setParserClass (const KODE::Class &c) {
    _parserClass = c;
}

KODE::Class &Creator::parserClass () {
    return _parserClass;
}

QString Creator::errorStream() const
{
    return "qCritical ()";
    //    if ( useKde() ) {
    //        return "kError()";
    //    } else {
    //        return "qCritical()";
    //    }
}

QString Creator::debugStream () const
{
    return "qDebug ()";
    //    if ( useKde() ) {
    //        return "kDebug()";
    //    } else {
    //        return "qDebug()";
    //    }
}

void Creator::create ()
{
    Schema::Element startElement = _document.startElement();
    setExternalClassPrefix (KODE::Style::upperFirst (startElement.name()));
    createFileParser (startElement);
    createFileWriter (startElement);

    createListTypedefs ();
}

void Creator::setFilename (const QString& baseName)
{
    _file.setFilename (baseName);
    _baseName = baseName;
}

QString Creator::typeName (Schema::Node::Type type)
{
    if (type == Schema::Element::DateTime) {
        return "QDateTime";
    } else if (type == Schema::Element::Date) {
        return "QDate";
    } else if (type == Schema::Element::Integer) {
        return "int";
    } else if (type == Schema::Element::Decimal) {
        return "double";
    } else if (type == Schema::Element::Boolean) {
        return "bool";
    } else {
        return "QString";
    }
}


ParserCreator::ParserCreator (Creator *c)
    : _creator (c)
{
}

ParserCreator::~ParserCreator () {}

Creator *ParserCreator::creator () const {
    return _creator;
}
