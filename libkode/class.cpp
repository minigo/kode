/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2010 David Faure <dfaure@kdab.com>

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

#include "class.h"

#include <QDebug>

using namespace KODE;

class Class::Private
{
public:
    Private ()
        : _dPointer ()
        , _useSharedData (false)
        , _canBeCopied (false)
    {
    }

    QString _name;
    QString _nameSpace;
    QString _exportDeclaration;
    QString _dPointer;
    bool _useSharedData;
    bool _canBeCopied;
    Function::List _functions;
    MemberVariable::List _memberVariables;
    QStringList _includes;
    QStringList _forwardDeclarations;
    QStringList _headerIncludes;
    Class::List _baseClasses;
    Typedef::List _typedefs;
    Enum::List _enums;
    QString _docs;
    Class::List _nestedClasses;
    QString _parentClassName;
    QStringList _declMacros;
};

Class::Class ()
    : d (new Private)
{
}

Class::Class (const Class &other)
    : d (new Private)
{
    *d = *other.d;
}

Class::Class (const QString &name, const QString &nameSpace)
    : d (new Private)
{
    Q_ASSERT(!name.isEmpty());
    d->_name = name;
    d->_nameSpace = nameSpace;
}

Class::~Class ()
{
    delete d;
}

Class& Class::operator=( const Class &other )
{
    if ( this == &other )
        return *this;

    *d = *other.d;
    return *this;
}

void Class::setName (const QString &name)
{
    Q_ASSERT(!name.isEmpty());
    d->_name = name;
}

QString Class::name () const {
    return d->_name;
}

void Class::setNameSpace (const QString &nameSpace) {
    d->_nameSpace = nameSpace;
}

QString Class::nameSpace () const
{
    return d->_nameSpace;
}

QString Class::qualifiedName () const
{
    if (d->_nameSpace.isEmpty())
        return d->_name;
    return d->_nameSpace + QLatin1String("::") + d->_name;
}

void Class::setExportDeclaration (const QString &name)
{
    addHeaderInclude (name.toLower () + "_export.h");
    if (name.contains ("/"))
        d->_exportDeclaration = name.split ("/").value (1);
    else
        d->_exportDeclaration = name;
}

QString Class::exportDeclaration () const {
    return d->_exportDeclaration;
}

void Class::setUseDPointer (bool useDPointer, const QString& dPointer) {
    d->_dPointer = useDPointer ? dPointer : QString ();
}

bool Class::useDPointer () const {
    return !d->_dPointer.isEmpty();
}

void Class::setUseSharedData (bool b, const QString& dPointer)
{
    d->_useSharedData = b;
    if (b) {
        setUseDPointer (true, dPointer);
        d->_canBeCopied = true;
    }
}

bool Class::useSharedData () const {
    return d->_useSharedData;
}

void Class::setCanBeCopied (bool b) {
    d->_canBeCopied = b;
}

bool Class::canBeCopied () const {
    return d->_canBeCopied;
}

void Class::addInclude (const QString &include,
                        const QString &forwardDeclaration)
{
    if ( !include.isEmpty() && !d->_includes.contains( include ) )
        d->_includes.append( include );

    if ( !forwardDeclaration.isEmpty() &&
         !d->_forwardDeclarations.contains( forwardDeclaration ) )
        d->_forwardDeclarations.append( forwardDeclaration );
}

void Class::addIncludes( const QStringList &files,
                         const QStringList &forwardDeclarations )
{
    for ( int i = 0; i < files.count(); ++i ) {
        if ( !d->_includes.contains( files[ i ] ) )
            if ( !files[ i ].isEmpty() )
                d->_includes.append( files[ i ] );
    }

    for ( int i = 0; i < forwardDeclarations.count(); ++i ) {
        if ( !d->_forwardDeclarations.contains( forwardDeclarations[ i ] ) )
            d->_forwardDeclarations.append( forwardDeclarations[ i ] );
    }
}

QStringList Class::includes() const
{
    return d->_includes;
}

QStringList Class::forwardDeclarations() const
{
    return d->_forwardDeclarations;
}

void Class::addHeaderInclude( const QString &include )
{
    if ( include.isEmpty() )
        return;

    if ( !d->_headerIncludes.contains( include ) )
        d->_headerIncludes.append( include );
}

void Class::addHeaderIncludes( const QStringList &includes )
{
    QStringList::ConstIterator it;
    for ( it = includes.constBegin(); it != includes.constEnd(); ++it )
        addHeaderInclude( *it );
}

QStringList Class::headerIncludes() const
{
    return d->_headerIncludes;
}

void Class::addBaseClass( const Class &c )
{
    d->_baseClasses.append( c );
}

Class::List Class::baseClasses() const
{
    return d->_baseClasses;
}

void Class::addFunction( const Function &function )
{
    d->_functions.append( function );
}

Function::List Class::functions() const
{
    return d->_functions;
}

void Class::addMemberVariable( const MemberVariable &v )
{
    d->_memberVariables.append( v );
}

MemberVariable::List Class::memberVariables() const
{
    return d->_memberVariables;
}

void Class::addTypedef( const Typedef &typeDefinition )
{
    d->_typedefs.append( typeDefinition );
}

Typedef::List Class::typedefs () const {
    return d->_typedefs;
}

void Class::addEnum (const Enum &enumValue)
{
    d->_enums.append( enumValue );
    KODE::Function enumParseFunction = enumValue.parserMethod();
    enumParseFunction.setReturnType( d->_name + "::" + enumParseFunction.returnType() );
    enumParseFunction.setAccess( KODE::Function::Private );
    addFunction( enumParseFunction );

    KODE::Function enumWriteFunction = enumValue.writerMethod();
    enumParseFunction.setReturnType( d->_name + "::" + enumWriteFunction.returnType() );
    enumParseFunction.setAccess( KODE::Function::Private );
    addFunction( enumWriteFunction );
}

Enum::List Class::enums() const
{
    return d->_enums;
}

bool Class::hasEnum( const QString &name ) const
{
    foreach( Enum e, d->_enums ) {
        if ( e.name() == name ) return true;
    }
    return false;
}

bool Class::isValid() const
{
    return !d->_name.isEmpty();
}

bool Class::hasFunction( const QString &functionName ) const
{
    Function::List::ConstIterator it;
    for ( it = d->_functions.constBegin(); it != d->_functions.constEnd(); ++it ) {
        if ( (*it).name() == functionName )
            return true;
    }

    return false;
}

bool Class::isQObject() const
{
    Function::List::ConstIterator it;
    for ( it = d->_functions.constBegin(); it != d->_functions.constEnd(); ++it ) {
        if ( (*it).access() & Function::Signal || (*it).access() & Function::Slot )
            return true;
    }

    return false;
}

void Class::setDocs( const QString &str )
{
    d->_docs = str;
}

QString Class::docs() const
{
    return d->_docs;
}

void Class::addNestedClass( const Class &nestedClass )
{
    Class addedClass = nestedClass;
    addedClass.setParentClassName( name() );

    d->_nestedClasses.append( addedClass );
}

Class::List Class::nestedClasses() const
{
    return d->_nestedClasses;
}

QString Class::parentClassName() const
{
    return d->_parentClassName;
}

void Class::setParentClassName( const QString &parentClassName )
{
    d->_parentClassName = parentClassName;
}

QString Class::dPointerName() const
{
    return d->_dPointer;
}

////

// Returns what a class depends on: its base class(es) and any by-value member var
static QStringList dependenciesForClass( const Class& aClass, const QStringList& allClasses, const QStringList& excludedClasses )
{
    QStringList lst;
    Q_FOREACH (const Class& baseClass, aClass.baseClasses ()) {
        const QString baseName = baseClass.name();
        if (!baseName.startsWith('Q') && !excludedClasses.contains (baseName))
            lst.append (baseClass.name ());
    }
    if (!aClass.useDPointer())
    {
        Q_FOREACH (const MemberVariable& member, aClass.memberVariables ()) {
            const QString type = member.type ();
            if (allClasses.contains (type)) {
                lst.append (type);
            }
        }
    }

    return lst;
}

static bool allKnown (const QStringList& deps, const QStringList& classNames)
{
    Q_FOREACH (const QString& dep, deps) {
        if (!classNames.contains (dep))
            return false;
    }
    return true;
}


/**
 * This method sorts a list of classes in a way that the base class
 * of a class, as well as the classes it use by value in member vars,
 * always appear before the class itself.
 */
static Class::List sortByDependenciesHelper (const Class::List &classes, const QStringList& excludedClasses)
{
    Class::List allClasses( classes );
    QStringList allClassNames;
    Q_FOREACH( const Class& c, classes )
        allClassNames.append( c.name() );

    Class::List retval;

    QStringList classNames;

    // copy all classes without dependencies
    Class::List::Iterator it;
    for ( it = allClasses.begin(); it != allClasses.end(); ++it ) {
        if ( dependenciesForClass( *it, allClassNames, excludedClasses ).isEmpty() ) {
            retval.append( *it );
            classNames.append( (*it).name() );

            it = allClasses.erase( it );
            it--;
        }
    }

    while ( allClasses.count() > 0 ) {
        const int currentCount = allClasses.count();
        // copy all classes which have a class from retval/classNames (i.e. already written out)
        // as base class - or as member variable
        for ( it = allClasses.begin(); it != allClasses.end(); ++it ) {

            const QStringList deps = dependenciesForClass( *it, allClassNames, excludedClasses );
            if ( allKnown( deps, classNames ) ) {
                retval.append( *it );
                classNames.append( (*it).name() );

                it = allClasses.erase( it );
                it--;
            }
        }
        if (allClasses.count() == currentCount) {
            // We didn't resolve anything this time around, so let's not loop forever
            qDebug() << "ERROR: Couldn't find class dependencies (base classes, member vars) for classes" << allClasses.classNames();
            Q_FOREACH(const Class& c, allClasses) {
                qDebug() << c.name() << "depends on" << dependenciesForClass( c, allClassNames, excludedClasses );
            }

            return retval;
        }
    }

    return retval;
}

void ClassList::sortByDependencies( const QStringList& excludedClasses )
{
    *this = sortByDependenciesHelper( *this, excludedClasses );
}

ClassList::iterator ClassList::findClass(const QString &name)
{
    ClassList::iterator it = begin();
    for (; it != end(); ++it)
        if ((*it).name() == name)
            break;
    return it;
}

QStringList KODE::ClassList::classNames() const
{
    QStringList names;
    ClassList::const_iterator it = begin();
    for (; it != end(); ++it)
        names.append((*it).name());
    return names;
}

void KODE::Class::addDeclarationMacro(const QString &macro)
{
    d->_declMacros.append(macro);
}

QStringList KODE::Class::declarationMacros() const
{
    return d->_declMacros;
}

void KODE::Class::setNamespaceAndName( const QString& name )
{
    d->_name = name;
    d->_nameSpace.clear();
    while (d->_name.contains("::")) {
        const int pos = d->_name.indexOf("::");
        if (!d->_nameSpace.isEmpty())
            d->_nameSpace += QLatin1String("::");
        d->_nameSpace += d->_name.left(pos);
        d->_name = d->_name.mid(pos+2);
    }
}
