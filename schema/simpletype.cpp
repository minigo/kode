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

#include "simpletype.h"
#include <QDebug>

namespace XSD {

class SimpleType::Private
{
public:
    Private ()
        : _restriction (false)
        , _facetId (NONE)
        , _anonymous (false)
        , _subType (TypeRestriction)
    {}

    QString _documentation;
    QName _baseTypeName;
    bool _restriction;
    int _facetId;
    bool _anonymous;
    QStringList _enums;
    SubType _subType;

    QName _listTypeName;

    typedef struct
    {
        int length;
        struct
        {
            int minlen, maxlen;
        } lenRange;
        WhiteSpaceType wsp;
        struct
        {
            int maxinc, mininc, maxex, minex;
        } valRange;
        int tot;
        int frac;
        QString pattern;
    } FacetValueType;

    FacetValueType _facetValue;
};

SimpleType::SimpleType ()
    : XSDType ()
    , d (new Private)
{
}

SimpleType::SimpleType (const QString &nameSpace)
    : XSDType (nameSpace)
    , d (new Private)
{
}

SimpleType::SimpleType (const SimpleType &other)
    : XSDType (other)
    , d (new Private)
{
    *d = *other.d;
}

SimpleType::~SimpleType ()
{
    delete d;
}

SimpleType &SimpleType::operator=( const SimpleType &other )
{
    if ( this == &other )
        return *this;

    *d = *other.d;

    return *this;
}

void SimpleType::setDocumentation( const QString &documentation )
{
    d->_documentation = documentation;
}

QString SimpleType::documentation() const
{
    return d->_documentation;
}

void SimpleType::setBaseTypeName (const QName &baseTypeName)
{
    d->_baseTypeName = baseTypeName;
    d->_restriction = true;
}

QName SimpleType::baseTypeName () const {
    return d->_baseTypeName;
}

void SimpleType::setSubType (SubType subType) {
    d->_subType = subType;
}

SimpleType::SubType SimpleType::subType () const {
    return d->_subType;
}

void SimpleType::setListTypeName( const QName &name )
{
    d->_listTypeName = name;
}

QName SimpleType::listTypeName() const
{
    return d->_listTypeName;
}

void SimpleType::setAnonymous( bool anonymous )
{
    d->_anonymous = anonymous;
}

bool SimpleType::isAnonymous() const
{
    return d->_anonymous;
}

bool SimpleType::isValidFacet (const QString &facet)
{
    if ( d->_baseTypeName.isEmpty() ) {
        qDebug( "isValidFacet:Unknown base type" );
        return false;
    }

    if ( facet == "length" )
        d->_facetId |= LENGTH;
    else if ( facet == "minLength" )
        d->_facetId |= MINLEN;
    else if ( facet == "maxLength" )
        d->_facetId |= MAXLEN;
    else if ( facet == "enumeration" )
        d->_facetId |= ENUM;
    else if ( facet == "whiteSpace" )
        d->_facetId |= WSP;
    else if ( facet == "pattern" )
        d->_facetId |= PATTERN;
    else if ( facet == "maxInclusive" )
        d->_facetId |= MAXINC;
    else if ( facet == "maxExclusive" )
        d->_facetId |= MAXEX;
    else if ( facet == "minInclusive" )
        d->_facetId |= MININC;
    else if ( facet == "minExclusive" )
        d->_facetId |= MINEX;
    else if ( facet == "totalDigits" )
        d->_facetId |= TOT;
    else if ( facet == "fractionDigits" )
        d->_facetId |= FRAC;
    else {
        d->_facetId = NONE;
        return false;
    }

    return true;
}

void SimpleType::setFacetValue (const QString &value)
{
    int number = -1;

    if (d->_facetId & ENUM) {
        d->_enums.append (value);
    } else if (d->_facetId & PATTERN) {
        d->_facetValue.pattern = value;
    } else if (d->_facetId & WSP) {
        if (value == "preserve")
            d->_facetValue.wsp = PRESERVE;
        else if (value == "collapse")
            d->_facetValue.wsp = COLLAPSE;
        else if (value == "replace")
            d->_facetValue.wsp = REPLACE;
        else {
            qDebug () << "[SimpleType][setFacetValue] Invalid facet value for whitespace";
            return;
        }
    } else {
        number = value.toInt ();
    }

    if (d->_facetId & MAXEX) {
        d->_facetValue.valRange.maxex = number;
    } else if (d->_facetId & MAXINC) {
        d->_facetValue.valRange.maxinc = number;
    } else if (d->_facetId & MININC) {
        d->_facetValue.valRange.mininc = number;
    } else if (d->_facetId & MINEX) {
        d->_facetValue.valRange.minex = number;
    } else if (d->_facetId & MAXEX) {
        d->_facetValue.valRange.maxex = number;
    } else if (d->_facetId & LENGTH) {
        d->_facetValue.length = number;
    } else if (d->_facetId & MINLEN) {
        d->_facetValue.lenRange.minlen = number;
    } else if (d->_facetId & MAXLEN) {
        d->_facetValue.lenRange.maxlen = number;
    } else if (d->_facetId & TOT) {
        d->_facetValue.tot = number;
    } else if (d->_facetId & FRAC) {
        d->_facetValue.frac = number;
    }
}

int SimpleType::facetType () const {
    return d->_facetId;
}

int SimpleType::facetLength () const {
    return d->_facetValue.length;
}

int SimpleType::facetMinimumLength () const {
    return d->_facetValue.lenRange.minlen;
}

int SimpleType::facetMaximumLength () const {
    return d->_facetValue.lenRange.maxlen;
}

QStringList SimpleType::facetEnums () const {
    return d->_enums;
}

SimpleType::WhiteSpaceType SimpleType::facetWhiteSpace () const {
    return d->_facetValue.wsp;
}

int SimpleType::facetMinimumInclusive () const {
    return d->_facetValue.valRange.mininc;
}

int SimpleType::facetMaximumInclusive () const {
    return d->_facetValue.valRange.maxinc;
}

int SimpleType::facetMinimumExclusive () const {
    return d->_facetValue.valRange.minex;
}

int SimpleType::facetMaximumExclusive () const {
    return d->_facetValue.valRange.maxex;
}

int SimpleType::facetTotalDigits () const {
    return d->_facetValue.tot;
}

int SimpleType::facetFractionDigits () const {
    return d->_facetValue.frac;
}

QString SimpleType::facetPattern () const {
    return d->_facetValue.pattern;
}

bool SimpleType::isValid ()
{
    if (d->_baseTypeName.qname ().isEmpty () && name ().isEmpty ())
        return false;
    return true;
}

}
