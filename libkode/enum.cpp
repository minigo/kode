/*
    This file is part of KDE.

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

#include <QtCore/QStringList>

#include "enum.h"
#include "code.h"
#include "style.h"

using namespace KODE;

class Enum::Private
{
public:
    Private ()
        : _combinable (false)
    {
    }

    QString _name;
    QStringList _enums;
    bool _combinable;
};

Enum::Enum ()
    : d (new Private)
{
}

Enum::Enum (const Enum &other)
    : d (new Private)
{
    *d = *other.d;
}

Enum::Enum (const QString &name, const QStringList &enums, bool combinable)
    : d (new Private)
{
    d->_name = name;
    d->_enums = enums;
    d->_combinable = combinable;
}

Enum::~Enum ()
{
    delete d;
}

Enum& Enum::operator=( const Enum &other )
{
    if ( this == &other )
        return *this;

    *d = *other.d;
    return *this;
}

QString Enum::name () const {
    return d->_name;
}

/**
 * @brief Enum::declaration
 * @return Returns a QString contatining the enum declaration in the following format:
 * enum Foo { Foo_a, Foo_b, Foo_Invalid };
 *
 * If the enum name is suffixed with Enum (FooEnum) then the Enum suffix will be removed
 * from the enum item's names:
 * enum FooEnum { Foo_a, Foo_b, Foo_Invalid };
 *
 * The last item (Foo_Invalid) is automatically generated to each elements, and the
 * enum parsing methods will return this value if the XML contains a non-enum value.
 */
QString Enum::declaration () const
{
    QString retval( "enum " + d->_name + " {" );
    uint value = 0;
    QStringList::ConstIterator it;
    QString baseName = name();
    if ( d->_name.right(4) == "Enum" && d->_name.length() > 4 ) {
        baseName = d->_name.left(d->_name.length() - 4);
    }

    for ( it = d->_enums.constBegin(); it != d->_enums.constEnd(); ++it, ++value ) {
        if ( d->_combinable ) {
            if ( it == d->_enums.constBegin() )
                retval += QString( " %1_%2 = %3" ).arg( baseName ).arg( Style::sanitize( *it ) ).arg( 1 << value );
            else
                retval += QString( ", %1_%2 = %3" ).arg( baseName ).arg( Style::sanitize( *it ) ).arg( 1 << value );
        } else {
            if ( it == d->_enums.constBegin() )
                retval += ' ' + baseName + '_' + Style::sanitize( *it );
            else
                retval += ", " + baseName + '_' + Style::sanitize( *it );
        }
    }

    retval += ", " + baseName + "_Invalid";
    retval += " };";

    return retval;
}

KODE::Function Enum::parserMethod () const
{
    QString baseName = name ();
    if (d->_name.right (4) == "Enum" && d->_name.length() > 4)
        baseName = d->_name.left (d->_name.length() - 4);

    KODE::Function ret (KODE::Style::lowerFirst (this->name()) + "FromString", this->name());
    ret.setStatic (true);

    ret.addArgument ("const QString & v");
    ret.addArgument ("bool *ok", "0x00");

    KODE::Code code;
    code += "if (ok) *ok = true;";
    code.newLine ();
    bool first = true;
    foreach (QString enumItem, d->_enums) {
        if (first) {
            code += "if (v == \"" + enumItem + "\") {";
            first = false;
        } else {
            code += "} else if (v == \"" + enumItem + "\") {";
        }
        code.indent ();
        code += "return " + baseName + '_' + Style::sanitize(enumItem) + ";";
        code.unindent ();
    }
    code += "} else {";
    code.indent ();
    code += "if (ok) *ok = false;";
    code += "return " + baseName + "_Invalid;";
    code.unindent ();
    code += "}";
    code.newLine();
    code += "return " + baseName + "_Invalid;";

    ret.setBody(code);
    return ret;
}

KODE::Function Enum::writerMethod () const
{
    QString baseName = name ();
    if (d->_name.right(4) == "Enum" && d->_name.length() > 4)
        baseName = d->_name.left(d->_name.length() - 4);

    KODE::Function ret (KODE::Style::lowerFirst(this->name()) + "ToString", "QString");
    ret.setStatic (true);

    ret.addArgument (QString("const %1 & v").arg(this->name()));

    KODE::Code code;
    code += "switch (v) {";
    code.indent ();
    foreach (QString enumItem, d->_enums ) {
        code += QString("case %1: return \"%2\";").arg(baseName + '_' + Style::sanitize(enumItem)).arg(enumItem);
    }
    code += "case " + baseName + "_Invalid:";
    code += "default:";
    code.indent();
    code += QString("qCritical() << \"Unable to serialize a(n) %1 enum because it has invalid value\" << %2;")
            .arg (this->name())
            .arg ("v");
    code += "return QString ();";
    code.unindent ();
    code.unindent ();
    code += "}";
    ret.setBody (code);
    return ret;
}
