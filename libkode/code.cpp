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

#include "code.h"

using namespace KODE;

static int s_defaultIndentation = 4;

class Code::Private
{
public:
    Private ()
        : _indent (0)
    {
    }

    QString _text;
    int _indent;
};

Code::Code ()
    : d (new Private)
{
}

Code::Code (const Code &other)
    : d (new Private)
{
    *d = *other.d;
}

Code::Code (int indent)
    : d (new Private)
{
    d->_indent = indent;
}

Code::~Code ()
{
    delete d;
}

Code& Code::operator=( const Code &other )
{
    if ( this == &other )
        return *this;

    *d = *other.d;
    return *this;
}

void Code::clear ()
{
    d->_indent = 0;
    d->_text.clear ();
}

bool Code::isEmpty () const {
    return d->_text.isEmpty();
}

void Code::setIndent (int indent) {
    d->_indent = indent;
}

void Code::indent () {
    d->_indent += s_defaultIndentation;
}

void Code::unindent () {
    d->_indent -= s_defaultIndentation;
    if ( d->_indent < 0 )
        d->_indent = 0;
}

QString Code::text () const {
    return d->_text;
}

void Code::addLine (const QString &line)
{
    d->_text += spaces( d->_indent );
    d->_text += line;
    d->_text += '\n';
}

void Code::addLine (const char c)
{
    d->_text += spaces( d->_indent );
    d->_text += c;
    d->_text += '\n';
}

void Code::newLine () {
    d->_text += '\n';
}

QString Code::spaces (int count) {
    QString str;
    for ( int i = 0; i < count; ++i )
        str += ' ';

    return str;
}

void Code::addBlock (const QString &block)
{
    QStringList lines = block.split( "\n" );
    if ( !lines.isEmpty() && lines.last().isEmpty() )
        lines.pop_back();

    QStringList::ConstIterator it;
    for ( it = lines.constBegin(); it != lines.constEnd(); ++it ) {
        if ( !(*it).isEmpty() )
            d->_text += spaces( d->_indent );

        d->_text += *it;
        d->_text += '\n';
    }
}

void Code::addBlock (const QString &block, int indent)
{
    int tmp = d->_indent;
    d->_indent = indent;
    addBlock( block );
    d->_indent = tmp;
}

void Code::addBlock (const Code &c) {
    addBlock (c.text ());
}

void Code::addWrappedText (const QString &txt)
{
    int maxWidth = 80 - d->_indent;
    int pos = 0;
    while ( pos < txt.length() ) {
        QString line = txt.mid( pos, maxWidth );
        addLine( line );
        pos += maxWidth;
    }
}

void Code::addFormattedText (const QString &text)
{
    int maxWidth = 80 - d->_indent;
    int lineLength = 0;

    QString line;
    const QStringList words = text.split( ' ', QString::SkipEmptyParts );

    QStringList::ConstIterator it;
    for ( it = words.constBegin(); it != words.constEnd(); ++it ) {
        if ( (*it).length() + lineLength >= maxWidth ) {
            line = line.trimmed();
            addLine( line );
            line.truncate( 0 );
            lineLength = 0;
        }

        int pos = (*it).indexOf( "\n" );
        if ( pos != -1 ) {
            line += (*it).left( pos );
            line = line.trimmed();
            addLine( line );

            line = (*it).mid( pos + 1 ) + ' ';
            lineLength = (*it).length() - pos;
        } else {
            line += *it + ' ';
            lineLength += (*it).length() + 1;
        }
    }

    line = line.trimmed();
    addLine( line );
}

Code &Code::operator+=( const QString &str )
{
    addLine( str );
    return *this;
}

Code &Code::operator+=( const QByteArray& str )
{
    addLine( QString::fromLocal8Bit( str.data(), str.size() ) );
    return *this;
}

Code &Code::operator+=( const char *str )
{
    addLine( QString::fromLocal8Bit( str ) );
    return *this;
}

Code &Code::operator+=( const char c )
{
    addLine( c );
    return *this;
}

Code &Code::operator+=( const Code &code )
{
    d->_text += code.d->_text;
    return *this;
}

void Code::setDefaultIndentation (int indent) {
    s_defaultIndentation = indent;
}

int Code::defaultIndentation () {
    return s_defaultIndentation;
}
