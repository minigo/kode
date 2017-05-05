/*
    This file is part of kdepim.

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

#include <QtCore/QStringList>

#include "file.h"

using namespace KODE;

class File::Private
{
public:
    Private ()
        : _project ()
    {
    }

    QString _headerFilename;
    QString _implFilename;
    QString _nameSpace;
    QString _project;
    QStringList _copyrightStrings;
    License _license;
    QStringList _includes;
    Class::List _classes;
    Variable::List _fileVariables;
    Function::List _fileFunctions;
    Enum::List _fileEnums;
    QStringList _externCDeclarations;
    Code _fileCode;
};

File::File()
    : d( new Private )
{
}

File::File( const File &other )
    : d( new Private )
{
    *d = *other.d;
}

File::~File()
{
    delete d;
}

File& File::operator=( const File &other )
{
    if ( this == &other )
        return *this;

    *d = *other.d;

    return *this;
}

void File::setFilename( const QString& filename )
{
    d->_implFilename = filename + ".cpp";
    d->_headerFilename = filename + ".h";
}

void File::setImplementationFilename( const QString &filename )
{
    d->_implFilename = filename;
}

void File::setHeaderFilename( const QString &filename )
{
    d->_headerFilename = filename;
}

QString File::filenameHeader() const
{
    if ( !d->_headerFilename.isEmpty() )
        return d->_headerFilename;

    if ( !d->_classes.isEmpty() ) {
        QString className = d->_classes[ 0 ].name();
        return className.toLower() + ".h";
    }

    return QString();
}

QString File::filenameImplementation() const
{
    if ( !d->_implFilename.isEmpty() )
        return d->_implFilename;

    if ( !d->_classes.isEmpty() ) {
        QString className = d->_classes[ 0 ].name();
        return className.toLower() + ".cpp";
    }

    return QString();
}

void File::setNameSpace( const QString &nameSpace )
{
    d->_nameSpace = nameSpace;
}

QString File::nameSpace() const
{
    return d->_nameSpace;
}

void File::setProject( const QString &project )
{
    if ( project.isEmpty() )
        return;

    d->_project = project;
}

QString File::project() const
{
    return d->_project;
}

void File::addCopyright( int year, const QString &name, const QString &email )
{
    QString str = "Copyright (c) " + QString::number( year ) + ' ' + name + " <"
            + email + '>';

    d->_copyrightStrings.append( str );
}

QStringList File::copyrightStrings() const
{
    return d->_copyrightStrings;
}

void File::setLicense( const License &license )
{
    d->_license = license;
}

License File::license() const
{
    return d->_license;
}

void File::addInclude( const QString &_include )
{
    QString include = _include;
    if ( !include.endsWith( ".h" ) )
        include.append( ".h" );

    if ( !d->_includes.contains( include ) )
        d->_includes.append( include );
}

QStringList File::includes() const
{
    return d->_includes;
}

void File::insertClass (const Class &newClass)
{
    Q_ASSERT(!newClass.name ().isEmpty ());
    Class::List::Iterator it;
    for (it = d->_classes.begin (); it != d->_classes.end (); ++it) {
        if ((*it).qualifiedName() == newClass.qualifiedName ()) {
            it = d->_classes.erase (it);
            d->_classes.insert (it, newClass);
            return;
        }
    }

    d->_classes.append( newClass );
}

Class::List File::classes() const
{
    return d->_classes;
}

bool File::hasClass( const QString &name )
{
    Class::List::ConstIterator it;
    for ( it = d->_classes.constBegin(); it != d->_classes.constEnd(); ++it ) {
        if ( (*it).name() == name )
            break;
    }

    return it != d->_classes.constEnd();
}

Class File::findClass( const QString &name )
{
    Class::List::ConstIterator it;
    for ( it = d->_classes.constBegin(); it != d->_classes.constEnd(); ++it ) {
        if ( (*it).name() == name )
            return *it;
    }

    return Class();
}

void File::addFileVariable( const Variable &variable )
{
    d->_fileVariables.append( variable );
}

Variable::List File::fileVariables() const
{
    return d->_fileVariables;
}

void File::addFileFunction( const Function &function )
{
    d->_fileFunctions.append( function );
}

Function::List File::fileFunctions() const
{
    return d->_fileFunctions;
}

void File::addFileEnum( const Enum &enumValue )
{
    d->_fileEnums.append( enumValue );
}

Enum::List File::fileEnums() const
{
    return d->_fileEnums;
}

void File::addExternCDeclaration( const QString &externalCDeclaration )
{
    d->_externCDeclarations.append( externalCDeclaration );
}

QStringList File::externCDeclarations() const
{
    return d->_externCDeclarations;
}

void File::addFileCode( const Code &code )
{
    d->_fileCode = code;
}

Code File::fileCode() const
{
    return d->_fileCode;
}

void File::clearClasses()
{
    d->_classes.clear();
}

void File::clearFileFunctions()
{
    d->_fileFunctions.clear();
}

void File::clearFileVariables()
{
    d->_fileVariables.clear();
}

void File::clearCode()
{
    clearClasses();
    clearFileFunctions();
    clearFileVariables();
}
