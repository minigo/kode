/*
    This file is part of KDE Schema Parser

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2006 Michaël Larouche <michael.larouche@kdemail.net>
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

#include <QDir>
#include <QFile>
#include <QUrl>
#include <QXmlSimpleReader>
#include <QtCore/QLatin1String>
#include <QtDebug>

#include "parser.h"
#include <common/fileprovider.h>
#include <common/messagehandler.h>
#include <common/nsmanager.h>
#include <common/parsercontext.h>

static const QString XMLSchemaURI ("http://www.w3.org/2001/XMLSchema");
static const QString WSDLSchemaURI ("http://schemas.xmlsoap.org/wsdl/");

namespace XSD {

class Parser::Private {
public:
    QString _nameSpace;

    SimpleType::List _simpleTypes;
    ComplexType::List _complexTypes;
    Element::List _elements;
    Attribute::List _attributes;
    AttributeGroup::List _attributeGroups;
    Annotation::List _annotations;

    QStringList _importedSchemas;
    QStringList _includedSchemas;
    QStringList _namespaces;

    QString _path;
};

Parser::Parser (const QString &nameSpace)
    : d (new Private)
{
    d->_nameSpace = nameSpace;
}

Parser::Parser (const Parser &other)
    : d (new Private)
{
    *d = *other.d;
}

Parser::~Parser ()
{
    clear ();
    delete d;
}

Parser &Parser::operator = (const Parser &other)
{
    if (this == &other)
        return *this;

    *d = *other.d;
    return *this;
}

void Parser::clear ()
{
    d->_importedSchemas.clear();
    d->_namespaces.clear();
    d->_complexTypes.clear();
    d->_simpleTypes.clear();
    d->_elements.clear();
    d->_attributes.clear();
    d->_attributeGroups.clear();
}

bool Parser::parseFile (ParserContext *context, QFile &file)
{
    QFileInfo fi (file);
    d->_path = fi.path ();

    QXmlInputSource source (&file);
    return parse (context, &source);
}

bool Parser::parseString (ParserContext *context, const QString &data)
{
    QXmlInputSource source;
    source.setData (data);
    return parse (context, &source);
}

bool Parser::parse (ParserContext *context, QXmlInputSource *source)
{
    QXmlSimpleReader reader;
    reader.setFeature (
                QLatin1String("http://xml.org/sax/features/namespace-prefixes"), true);

    //QDomDocument document (QLatin1String("KWSDL"));

    QString errorMsg;
    int errorLine, errorCol;
    QDomDocument doc;
    if (!doc.setContent(source, &reader, &errorMsg, &errorLine, &errorCol)) {
        qDebug("%s at (%d,%d)", qPrintable(errorMsg), errorLine, errorCol);
        return false;
    }

    QDomElement element = doc.documentElement();
    const QName name = QName (element.tagName ());
    if (name.localName () != QLatin1String ("schema")) {
        qDebug () << "[Parser][parse] Document element is" << qPrintable (element.tagName());
        return false;
    }

    return parseSchemaTag (context, element);
}

bool Parser::parseSchemaTag (ParserContext *context, const QDomElement &root)
{
    QName name = QName (root.tagName ());
    if (name.localName () != QLatin1String ("schema"))
        return false;

    NSManager *parentManager = context->namespaceManager ();
    NSManager namespaceManager;

    // copy namespaces from wsdl
    if (parentManager)
        namespaceManager = *parentManager;

    context->setNamespaceManager (&namespaceManager);

    QDomNamedNodeMap attributes = root.attributes ();
    for (int i = 0; i < attributes.count(); ++i) {
        QDomAttr attribute = attributes.item (i).toAttr ();
        if (attribute.name ().startsWith (QLatin1String ("xmlns:"))) {
            QString prefix = attribute.name ().mid (6);
            context->namespaceManager ()->setPrefix (prefix, attribute.value ());
        }
    }

    if (root.hasAttribute (QLatin1String ("targetNamespace")))
        d->_nameSpace = root.attribute (QLatin1String ("targetNamespace"));

    // mTypesTable.setTargetNamespace( mNameSpace );

    QDomElement element = root.firstChildElement ();
    while (!element.isNull ()) {
        QName name = QName (element.tagName ());
        if (name.localName () == QLatin1String ("import")) {
            parseImport (context, element);
        } else if (name.localName() == QLatin1String ("element")) {
            addGlobalElement (parseElement (context, element, d->_nameSpace, element));
        } else if (name.localName() == QLatin1String ("complexType")) {
            ComplexType ct = parseComplexType (context, element);
            d->_complexTypes.append (ct);
        } else if (name.localName() == QLatin1String ("simpleType")) {
            SimpleType st = parseSimpleType(context, element);
            d->_simpleTypes.append (st);
        } else if (name.localName() == QLatin1String ("attribute")) {
            addGlobalAttribute(parseAttribute(context, element));
        } else if (name.localName() == QLatin1String ("attributeGroup")) {
            d->_attributeGroups.append(parseAttributeGroup(context, element));
        } else if (name.localName() == QLatin1String ("annotation")) {
            d->_annotations = parseAnnotation(context, element);
        } else if (name.localName() == QLatin1String ("include")) {
            parseInclude (context, element);
        }

        element = element.nextSiblingElement();
    }

    context->setNamespaceManager (parentManager);
    d->_namespaces = joinNamespaces(d->_namespaces, namespaceManager.uris());
    d->_namespaces = joinNamespaces(d->_namespaces, QStringList(d->_nameSpace));

    resolveForwardDeclarations ();
    return true;
}

void Parser::parseImport (ParserContext *context, const QDomElement &element)
{
    QString location = element.attribute ("schemaLocation");

    if (location.isEmpty ())
        location = element.attribute ("namespace");

    if (!location.isEmpty ()) {
        // don't import a schema twice
        if (d->_importedSchemas.contains (location))
            return;
        else
            d->_importedSchemas.append (location);

        importSchema (context, location);
    }
}

void Parser::parseInclude (ParserContext *context, const QDomElement &element)
{
    QString location = element.attribute ("schemaLocation");

    if (!location.isEmpty ()) {
        // don't include a schema twice
        if (d->_includedSchemas.contains (location))
            return;
        else
            d->_includedSchemas.append (location);

        includeSchema (context, location);
    } else {
        context->messageHandler()->warning(
                    QString("include tag found at (%1, %2) contains no schemaLocation tag.")
                    .arg(element.lineNumber(), element.columnNumber()));
    }
}

Annotation::List Parser::parseAnnotation (ParserContext *, const QDomElement &element)
{
    Annotation::List result;

    QDomElement e;
    for (e = element.firstChildElement(); !e.isNull();
         e = e.nextSiblingElement()) {
        QName name = QName (e.tagName ());
        if (name.localName() == "documentation") {
            result.append(Annotation(e));
        } else if (name.localName() == "appinfo") {
            result.append(Annotation(e));
        }
    }

    return result;
}

ComplexType Parser::parseComplexType (ParserContext *context, const QDomElement &element)
{
    ComplexType newType (d->_nameSpace);
    newType.setName (element.attribute ("name"));

    if (element.hasAttribute ("mixed"))
        newType.setContentModel (XSDType::MIXED);

    QDomElement childElement = element.firstChildElement ();

    AttributeGroup::List attributeGroups;

    while (!childElement.isNull()) {
        QName name = QName (childElement.tagName ());
        if (name.localName () == "all") {
            all (context, childElement, newType);
        } else if (name.localName() == "sequence") {
            parseCompositor (context, childElement, newType);
        } else if (name.localName() == "choice") {
            parseCompositor (context, childElement, newType);
        } else if (name.localName() == "attribute") {
            newType.addAttribute (parseAttribute (context, childElement));
        } else if (name.localName() == "attributeGroup") {
            AttributeGroup g = parseAttributeGroup (context, childElement);
            attributeGroups.append (g);
        } else if (name.localName () == "anyAttribute") {
            addAnyAttribute(context, childElement, newType);
        } else if (name.localName () == "complexContent") {
            parseComplexContent (context, childElement, newType);
        } else if (name.localName () == "simpleContent") {
            parseSimpleContent (context, childElement, newType);
        } else if (name.localName () == "annotation") {
            Annotation::List annotations = parseAnnotation (context, childElement);
            newType.setDocumentation (annotations.documentation ());
            newType.setAnnotations (annotations);
        } else
            qCritical () << "[Parser][parseComplexType] Undefined LocalName" << name.localName ();

        childElement = childElement.nextSiblingElement();
    }

    newType.setAttributeGroups (attributeGroups);
    return newType;
}

void Parser::all (ParserContext *context, const QDomElement &element,
                  ComplexType &ct)
{
    QDomElement childElement = element.firstChildElement ();

    while (!childElement.isNull()) {
        QName name = QName (childElement.tagName ());
        if (name.localName () == "element") {
            Element e = parseElement (context, childElement, ct.nameSpace(), childElement);
            ct.addElement (e);
        } else if (name.localName() == "annotation") {
            Annotation::List annotations = parseAnnotation (context, childElement);
            ct.setDocumentation (annotations.documentation ());
            ct.setAnnotations (annotations);
        }

        childElement = childElement.nextSiblingElement ();
    }
}

void Parser::parseCompositor (ParserContext *context, const QDomElement &element,
                              ComplexType &ct)
{
    QName name = QName (element.tagName ());
    bool isChoice = name.localName () == "choice";
    bool isSequence = name.localName () == "sequence";

    Compositor compositor;
    if (isChoice)
        compositor.setType (Compositor::Choice);
    else if (isSequence)
        compositor.setType (Compositor::Sequence);

    if (isChoice || isSequence)
    {
        Element::List newElements;
        QDomElement childElement = element.firstChildElement ();

        while (!childElement.isNull())
        {
            QName csName = QName (childElement.tagName ());
            if (csName.localName () == "element") {
                Element newElement;
                if (isChoice)
                    newElement = parseElement (context, childElement, ct.nameSpace (), element);
                else
                    newElement = parseElement (context, childElement, ct.nameSpace (), childElement);

                newElements.append (newElement);
                compositor.addChild (csName);
            } else if (csName.localName() == "any") {
                addAny (context, childElement, ct);
            } else if (isChoice) {
                parseCompositor (context, childElement, ct);
            } else if (isSequence) {
                parseCompositor (context, childElement, ct);
            }

            childElement = childElement.nextSiblingElement ();
        }

        foreach (Element e, newElements) {
            e.setCompositor (compositor);
            ct.addElement (e);
        }
    }
}

Element Parser::parseElement (ParserContext *context, const QDomElement &element,
                              const QString &nameSpace,
                              const QDomElement &occurrenceElement)
{
    Element newElement (nameSpace);
    newElement.setName (element.attribute ("name"));

    if (element.hasAttribute ("form")) {
        if (element.attribute ("form") == "qualified")
            newElement.setIsQualified (true);
        else if (element.attribute ("form") == "unqualified")
            newElement.setIsQualified (false);
        else
            newElement.setIsQualified (false);
    }

    if (element.hasAttribute ("ref")) {
        QName reference = QName (element.attribute ("ref"));
        reference.setNameSpace (context->namespaceManager ()->uri (reference.prefix ()));

        newElement.setReference (reference);
    }

    setOccurrenceAttributes (newElement, occurrenceElement);

    newElement.setDefaultValue (element.attribute ("default"));
    newElement.setFixedValue (element.attribute ("fixed"));

    //bool nill = false;
    //if (element.hasAttribute ("nillable"))
    //    nill = true;

    //QName anyType ("http://www.w3.org/2001/XMLSchema", "any");

    if (element.hasAttribute ("type")) {
        QName typeName = QName (element.attribute ("type"));
        typeName.setNameSpace (context->namespaceManager()->uri (typeName.prefix()));
        newElement.setType (typeName);
    } else {
        QDomElement childElement = element.firstChildElement ();

        while (!childElement.isNull ())
        {
            QName childName = QName (childElement.tagName ());
            if (childName.localName () == "complexType")
            {
                ComplexType ct = parseComplexType (context, childElement);

                ct.setName (newElement.name () + "Anonymous");
                d->_complexTypes.append (ct);

                newElement.setType (ct.qualifiedName ());
            }
            else if (childName.localName () == "simpleType")
            {
                SimpleType st = parseSimpleType (context, childElement);

                st.setName (newElement.name () + "Anonymous");
                d->_simpleTypes.append (st);

                newElement.setType (st.qualifiedName ());
            }
            else if (childName.localName() == "annotation")
            {
                Annotation::List annotations = parseAnnotation (context, childElement);
                newElement.setDocumentation (annotations.documentation ());
                newElement.setAnnotations (annotations);
            }

            childElement = childElement.nextSiblingElement ();
        }
    }

    return newElement;
}

void Parser::addAny (ParserContext *, const QDomElement &element,
                     ComplexType &complexType)
{
    Element newElement (complexType.nameSpace ());
    newElement.setName ("any");
    setOccurrenceAttributes (newElement, element);

    complexType.addElement (newElement);
}

void Parser::setOccurrenceAttributes (Element &newElement,
                                      const QDomElement &element)
{
    newElement.setMinOccurs (element.attribute ("minOccurs", "1").toInt ());

    QString value = element.attribute ("maxOccurs", "1");
    if (value == "unbounded")
        newElement.setMaxOccurs (UNBOUNDED);
    else
        newElement.setMaxOccurs (value.toInt());
}

void Parser::addAnyAttribute (ParserContext *, const QDomElement &element,
                              ComplexType &complexType)
{
    Attribute newAttribute;
    newAttribute.setName("anyAttribute");

    newAttribute.setNameSpace(element.attribute("namespace"));

    complexType.addAttribute(newAttribute);
}

Attribute Parser::parseAttribute (ParserContext *context,
                                  const QDomElement &element)
{
    Attribute newAttribute;

    newAttribute.setName (element.attribute ("name"));

    if (element.hasAttribute ("type")) {
        QName typeName = QName (element.attribute ("type"));
        typeName.setNameSpace (context->namespaceManager()->uri(typeName.prefix()));
        newAttribute.setType(typeName);
    }

    if (element.hasAttribute("form")) {
        if (element.attribute("form") == "qualified")
            newAttribute.setIsQualified(true);
        else if (element.attribute("form") == "unqualified")
            newAttribute.setIsQualified(false);
        else
            newAttribute.setIsQualified(false);
    }

    if (element.hasAttribute("ref")) {
        QName reference;
        reference = element.attribute("ref");
        reference.setNameSpace(
                    context->namespaceManager()->uri(reference.prefix()));

        newAttribute.setReference(reference);
    }

    newAttribute.setDefaultValue(element.attribute("value"));
    newAttribute.setFixedValue(element.attribute("fixed"));

    if (element.hasAttribute("use")) {
        if (element.attribute("use") == "optional")
            newAttribute.setIsUsed(false);
        else if (element.attribute("use") == "required")
            newAttribute.setIsUsed(true);
        else
            newAttribute.setIsUsed(false);
    }

    QDomElement childElement = element.firstChildElement();

    while (!childElement.isNull()) {
        QName childName = QName (childElement.tagName ());
        if (childName.localName() == "simpleType") {
            SimpleType st = parseSimpleType(context, childElement);
            st.setName(newAttribute.name());
            d->_simpleTypes.append(st);

            newAttribute.setType(st.qualifiedName());
        } else if (childName.localName() == "annotation") {
            Annotation::List annotations = parseAnnotation(context, childElement);
            newAttribute.setDocumentation(annotations.documentation());
            newAttribute.setAnnotations(annotations);
        }

        childElement = childElement.nextSiblingElement();
    }

    return newAttribute;
}

SimpleType Parser::parseSimpleType (ParserContext *context,
                                    const QDomElement &element)
{
    SimpleType st (d->_nameSpace);
    st.setName (element.attribute ("name"));

    QDomElement childElement = element.firstChildElement ();

    while (!childElement.isNull ()) {
        QName name = QName (childElement.tagName ());
        if (name.localName() == "restriction")
        {
            st.setSubType (SimpleType::TypeRestriction);

            QName typeName (childElement.attribute ("base"));
            typeName.setNameSpace(
                        context->namespaceManager()->uri(typeName.prefix()));
            st.setBaseTypeName (typeName);

            parseRestriction (context, childElement, st);
        }
        else if (name.localName() == "union")
        {
            st.setSubType (SimpleType::TypeUnion);
            qDebug() << "[Parser][parseSimpleType] SimpleType::union not supported";
        }
        else if (name.localName () == "list")
        {
            st.setSubType (SimpleType::TypeList);
            if (childElement.hasAttribute ("itemType"))
            {
                QName typeName (childElement.attribute ("itemType"));
                if (!typeName.prefix ().isEmpty ())
                    typeName.setNameSpace (
                                context->namespaceManager ()->uri (typeName.prefix()));
                else
                    typeName.setNameSpace (st.nameSpace ());
                st.setListTypeName (typeName);
            }
            else
            {
                // TODO: add support for anonymous types
                qCritical () << "--------------------------> JOPA" << st.name ();
            }
        }
        else if (name.localName () == "annotation")
        {
            Annotation::List annotations = parseAnnotation (context, childElement);
            st.setDocumentation (annotations.documentation ());
            st.setAnnotations (annotations);
        }

        childElement = childElement.nextSiblingElement();
    }

    return st;
}

void Parser::parseRestriction (ParserContext *, const QDomElement &element,
                               SimpleType &st)
{
    if (st.baseTypeName ().isEmpty ())
        qDebug () << "[Parser][parseRestriction] Parser::parseRestriction";

    QDomElement childElement = element.firstChildElement ();

    while (!childElement.isNull ()) {
        QName tagName = QName (childElement.tagName ());
        if (!st.isValidFacet (tagName.localName ())) {
            qDebug ("<restriction>: %s is not a valid facet for the simple type",
                    qPrintable (childElement.tagName()));
            childElement = childElement.nextSiblingElement ();
            continue;
        }

        st.setFacetValue (childElement.attribute ("value"));

        childElement = childElement.nextSiblingElement ();
    }
}

void Parser::parseComplexContent (ParserContext *context,
                                  const QDomElement &element,
                                  ComplexType &complexType)
{
    QName typeName;

    if (element.attribute("mixed") == "true") {
        qDebug("<complexContent>: No support for mixed=true");
        return;
    }

    complexType.setContentModel(XSDType::COMPLEX);

    QDomElement childElement = element.firstChildElement();

    while (!childElement.isNull()) {
        QName name = QName (childElement.tagName ());

        if (name.localName() == "restriction" || name.localName() == "extension") {
            typeName = childElement.attribute("base");
            typeName.setNameSpace(
                        context->namespaceManager()->uri(typeName.prefix()));

            complexType.setBaseTypeName(typeName);

            // if the base soapenc:Array, then read only the arrayType attribute and
            // nothing else
            if (typeName.localName() == "Array") {
                complexType.setIsArray(true);

                QDomElement arrayElement = childElement.firstChildElement();
                if (!arrayElement.isNull()) {
                    QString prefix = context->namespaceManager()->prefix(WSDLSchemaURI);
                    QString attributeName =
                            (prefix.isEmpty() ? "arrayType" : prefix + ":arrayType");

                    QString typeStr = arrayElement.attribute(attributeName);
                    if (typeStr.endsWith("[]"))
                        typeStr.truncate(typeStr.length() - 2);

                    QName arrayType(typeStr);
                    arrayType.setNameSpace(
                                context->namespaceManager()->uri(arrayType.prefix()));

                    Attribute attr(complexType.nameSpace());
                    attr.setName("items");
                    attr.setType(QName("arrayType"));
                    attr.setArrayType(arrayType);

                    complexType.addAttribute(attr);
                }
            } else {
                QDomElement ctElement = childElement.firstChildElement();
                while (!ctElement.isNull()) {
                    QName name = QName (ctElement.tagName ());

                    if (name.localName() == "all") {
                        all(context, ctElement, complexType);
                    } else if (name.localName() == "sequence") {
                        parseCompositor(context, ctElement, complexType);
                    } else if (name.localName() == "choice") {
                        parseCompositor(context, ctElement, complexType);
                    } else if (name.localName() == "attribute") {
                        complexType.addAttribute(parseAttribute(context, ctElement));
                    } else if (name.localName() == "anyAttribute") {
                        addAnyAttribute(context, ctElement, complexType);
                    }

                    ctElement = ctElement.nextSiblingElement ();
                }
            }
        }

        childElement = childElement.nextSiblingElement ();
    }
}

void Parser::parseSimpleContent (ParserContext *context,
                                 const QDomElement &element,
                                 ComplexType &complexType)
{
    complexType.setContentModel (XSDType::SIMPLE);

    QDomElement childElement = element.firstChildElement ();

    while (!childElement.isNull ())
    {
        QName name = QName (childElement.tagName ());
        if (name.localName() == "restriction") {
            SimpleType st(d->_nameSpace);

            if (childElement.hasAttribute("base")) {
                QName typeName(childElement.attribute("base"));
                typeName.setNameSpace(
                            context->namespaceManager()->uri(typeName.prefix()));
                st.setBaseTypeName(typeName);
            }

            parseRestriction(context, childElement, st);
        } else if (name.localName() == "extension") {
            // This extension does not use the full model that can come in
            // ComplexContent.
            // It uses the simple model. No particle allowed, only attributes

            if (childElement.hasAttribute ("base")) {
                QName typeName (childElement.attribute ("base"));
                typeName.setNameSpace (
                            context->namespaceManager()->uri(typeName.prefix()));
                complexType.setBaseTypeName (typeName);

                QDomElement ctElement = childElement.firstChildElement ();
                while (!ctElement.isNull()) {
                    QName name = QName (ctElement.tagName ());
                    if (name.localName () == "attribute")
                        complexType.addAttribute (parseAttribute(context, ctElement));

                    ctElement = ctElement.nextSiblingElement ();
                }
            }
        }

        childElement = childElement.nextSiblingElement();
    }
}

void Parser::addGlobalElement (const Element &newElement)
{
    // don't add elements twice
    bool found = false;
    for (int i = 0; i < d->_elements.count(); ++i) {
        if (d->_elements[i].qualifiedName () == newElement.qualifiedName ()) {
            found = true;
            break;
        }
    }

    if (!found)
        d->_elements.append (newElement);
}

void Parser::addGlobalAttribute (const Attribute &newAttribute)
{
    // don't add attributes twice
    bool found = false;
    for (int i = 0; i < d->_attributes.count(); ++i) {
        if (d->_attributes[i].qualifiedName() == newAttribute.qualifiedName()) {
            found = true;
            break;
        }
    }

    if (!found)
        d->_attributes.append(newAttribute);
}

AttributeGroup Parser::parseAttributeGroup (ParserContext *context,
                                            const QDomElement &element)
{
    Attribute::List attributes;

    AttributeGroup group;

    if (element.hasAttribute ("ref")) {
        QName reference;
        reference = element.attribute ("ref");
        reference.setNameSpace (
                    context->namespaceManager()->uri(reference.prefix()));
        group.setReference (reference);

        return group;
    }

    QDomNode n;
    for (n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        QName childName = QName(e.tagName());
        if (childName.localName() == "attribute") {
            Attribute a = parseAttribute(context, e);
            addGlobalAttribute(a);
            attributes.append(a);
        }
    }

    group.setName (element.attribute ("name"));
    group.setAttributes (attributes);

    return group;
}

QString Parser::targetNamespace () const {
    return d->_nameSpace;
}

void Parser::importSchema (ParserContext *context, const QString &location)
{
    FileProvider provider;
    QString fileName;
    QString schemaLocation;
    QString newlocation;

    if (location.startsWith ("http://") || location.startsWith ("https://")) {
        QUrl url (location);
        QDir dir (location);
        if ((url.scheme ().isEmpty () || url.scheme () == "file") && dir.isRelative())
            schemaLocation = context->documentBaseUrl () + '/' + location;
    } else {
        if (d->_path.endsWith ("/"))
            newlocation = d->_path + QString (location).replace ("\\", "/");
        else
            newlocation = d->_path + "/" + QString (location).replace ("\\", "/");

        QFileInfo fi (newlocation);
        if (!fi.exists ()) {
            qCritical () << "[Parser][includeSchema] Could not find file"
                         << fi.absoluteFilePath ();
            return;
        }

        schemaLocation = fi.absoluteFilePath ();
    }

    qDebug () << "[Parser][includeSchema] Importing schema at"
              << qPrintable (schemaLocation);

    if (provider.get (schemaLocation, fileName)) {
        QFile file (fileName);
        if (!file.open (QIODevice::ReadOnly)) {
            qCritical () << "[Parser][includeSchema] Unable to open file"
                         << qPrintable (file.fileName());
            return;
        }

        QXmlInputSource source (&file);
        QXmlSimpleReader reader;
        reader.setFeature ("http://xml.org/sax/features/namespace-prefixes", true);

        QDomDocument doc ("kwsdl");
        QString errorMsg;
        int errorLine, errorColumn;
        bool ok = doc.setContent (&source, &reader, &errorMsg, &errorLine, &errorColumn);
        if (!ok) {
            qDebug() << "[Parser][importSchema] Could not set content"
                     << "File" << fileName << "Line" << errorLine << "Column"
                     << errorColumn << "Error" << qPrintable (errorMsg);
            return;
        }

        NSManager *parentManager = context->namespaceManager();
        NSManager namespaceManager;
        context->setNamespaceManager (&namespaceManager);

        QDomElement node = doc.documentElement ();
        QName tagName = QName (node.tagName ());
        if (tagName.localName() == "schema")
            parseSchemaTag (context, node);
        else
            qDebug () << "[Parser][importSchema] No schema tag found in schema file"
                      << qPrintable (schemaLocation);

        d->_namespaces = joinNamespaces (d->_namespaces, namespaceManager.uris());
        context->setNamespaceManager (parentManager);

        file.close ();
        provider.cleanUp ();
    }
}

// TODO: Try to merge import and include schema
void Parser::includeSchema (ParserContext *context, const QString &location)
{
    FileProvider provider;
    QString fileName;
    QString schemaLocation;
    QString newlocation;

    if (location.startsWith ("http://") || location.startsWith ("https://")) {
        QUrl url (location);
        QDir dir (location);
        if ((url.scheme ().isEmpty () || url.scheme () == "file") && dir.isRelative())
            schemaLocation = context->documentBaseUrl () + '/' + location;
    } else {
        if (d->_path.endsWith ("/"))
            newlocation = d->_path + QString (location).replace ("\\", "/");
        else
            newlocation = d->_path + "/" + QString (location).replace ("\\", "/");

        QFileInfo fi (newlocation);
        if (!fi.exists ()) {
            qCritical () << "[Parser][includeSchema] Could not find file"
                         << fi.absoluteFilePath ();
            return;
        }

        schemaLocation = fi.absoluteFilePath ();
    }

    qDebug () << "[Parser][includeSchema] Including schema at"
              << qPrintable (schemaLocation);

    if (provider.get (schemaLocation, fileName)) {
        QFile file (fileName);
        if (!file.open (QIODevice::ReadOnly)) {
            qDebug() << "[Parser][includeSchema] Unable to open file"
                     << qPrintable (file.fileName ());
            return;
        }

        QXmlInputSource source (&file);
        QXmlSimpleReader reader;
        reader.setFeature ("http://xml.org/sax/features/namespace-prefixes", true);

        QDomDocument doc ("kwsdl");
        QString errorMsg;
        int errorLine, errorColumn;
        bool ok = doc.setContent (&source, &reader, &errorMsg, &errorLine, &errorColumn);
        if (!ok) {
            qDebug() << "[Parser][includeSchema] Could not set content"
                     << "File" << fileName << "Line" << errorLine << "Column"
                     << errorColumn << "Error" << qPrintable (errorMsg);
            return;
        }

        NSManager *parentManager = context->namespaceManager ();
        NSManager namespaceManager;
        context->setNamespaceManager (&namespaceManager);

        QDomElement node = doc.documentElement ();
        QName tagName = QName (node.tagName ());
        if (tagName.localName() == "schema") {
            // For include, targetNamespace must be the same as the current document.
            if (node.hasAttribute (QLatin1String("targetNamespace"))) {
                if (node.attribute (QLatin1String("targetNamespace")) != d->_nameSpace) {
                    context->messageHandler ()->error (
                                QLatin1String ("Included schema must be in the same namespace of "
                                               "the resulting schema."));
                    return;
                }
            }
            parseSchemaTag (context, node);
        } else
            qDebug() << "[Parser][includeSchema] No schema tag found in schema file"
                     << qPrintable (schemaLocation);

        d->_namespaces = joinNamespaces (d->_namespaces, namespaceManager.uris());
        context->setNamespaceManager (parentManager);

        file.close ();
        provider.cleanUp ();
    }
}

QString Parser::schemaUri () {
    return XMLSchemaURI;
}

void Parser::printDebugInfo () const
{
    qDebug () << "[Parser][printDebugInfo] Debug info";
    qDebug () << "Complex types";
    for (int i = 0; i < d->_complexTypes.size(); ++i)
        qDebug() << "Name:" << d->_complexTypes[i].name();
}

void Parser::printComplexTypeInfo (const QString &name) const
{
    int i = 0;
    for (; i < d->_complexTypes.size(); ++i)
        if (d->_complexTypes[i].name() == name)
            break;

    if (i == d->_complexTypes.size()) {
        qWarning () << "[Parser][printComplexTypeInfo] Could noty find complex type"
                    << name;
        return;
    }

    qDebug () << "[Parser][printComplexTypeInfo] Complex type info";
    qDebug () << "  Name:" << d->_complexTypes[i].name();
    qDebug () << "  NameSpace:" << d->_complexTypes[i].nameSpace();
    qDebug () << "  BaseTypeName:" << d->_complexTypes[i].baseTypeName().qname();
    qDebug () << "  Elements:";

    for (int j = 0; j < d->_complexTypes[i].elements().size(); ++j)
        qDebug () << "  name:" << d->_complexTypes[i].elements()[j].name()
                  << "type" << d->_complexTypes[i].elements()[j].type ().qname ();
}

void Parser::printElementInfo (const Element &value) const
{
    qDebug () << "[Parser][printElementInfo] Element";
    qDebug () << "[Parser][printElementInfo] name" << value.name ();
    qDebug () << "[Parser][printElementInfo] type" << value.type ().localName ();
    qDebug () << "[Parser][printElementInfo] qname" << value.type ().qname ();
}

QStringList Parser::joinNamespaces (const QStringList &list,
                                    const QStringList &namespaces)
{
    QStringList retval (list);

    for (int i = 0; i < namespaces.count(); ++i) {
        if (!retval.contains(namespaces[i]))
            retval.append(namespaces[i]);
    }

    return retval;
}

Element Parser::findElement (const QName &name)
{
    for (int i = 0; i < d->_elements.count(); ++i) {
        if (d->_elements[i].nameSpace() == name.nameSpace() &&
                d->_elements[i].name() == name.localName())
            return d->_elements[i];
    }

    return Element ();
}

Attribute Parser::findAttribute (const QName &name)
{
    for (int i = 0; i < d->_attributes.count(); ++i) {
        if (d->_attributes[i].nameSpace() == name.nameSpace() &&
                d->_attributes[i].name() == name.localName())
            return d->_attributes[i];
    }

    return Attribute ();
}

AttributeGroup Parser::findAttributeGroup (const QName &name)
{
    foreach (AttributeGroup g, d->_attributeGroups) {
        if (g.nameSpace() == name.nameSpace() && g.name() == name.localName()) {
            return g;
        }
    }

    return AttributeGroup ();
}

void Parser::resolveForwardDeclarations ()
{
    for (int i = 0; i < d->_complexTypes.count(); ++i)
    {
        Element::List elements = d->_complexTypes[i].elements();
        for (int j = 0; j < elements.count(); ++j) {
            Element element = elements[j];
            if (!element.isResolved ()) {
                Element refElement = findElement (element.reference ());
                Element resolvedElement = refElement;
                resolvedElement.setMinOccurs (element.minOccurs());
                resolvedElement.setMaxOccurs (element.maxOccurs());
                resolvedElement.setCompositor (element.compositor());
                elements[j] = resolvedElement;
            }
        }
        d->_complexTypes[i].setElements (elements);

        Attribute::List attributes = d->_complexTypes[i].attributes();

        for (int j = 0; j < attributes.count(); ++j) {
            if (!attributes[j].isResolved()) {
                Attribute refAttribute = findAttribute(attributes[j].reference());
                attributes[j] = refAttribute;
            }
        }

        AttributeGroup::List attributeGroups =
                d->_complexTypes[i].attributeGroups();
        foreach (AttributeGroup group, attributeGroups) {
            if (!group.isResolved()) {
                AttributeGroup refAttributeGroup =
                        findAttributeGroup(group.reference());
                Attribute::List groupAttributes = refAttributeGroup.attributes();
                foreach (Attribute ga, groupAttributes) { attributes.append(ga); }
            }
        }

        d->_complexTypes[i].setAttributes (attributes);
    }
}

Types Parser::types () const
{
    Types types;

    types.setSimpleTypes (d->_simpleTypes);
    types.setComplexTypes (d->_complexTypes);
    types.setElements (d->_elements);
    types.setAttributes (d->_attributes);
    types.setAttributeGroups (d->_attributeGroups);
    types.setNamespaces (d->_namespaces);

    return types;
}

Annotation::List Parser::annotations () const {
    return d->_annotations;
}
}

// kate: space-indent on; indent-width 2; encoding utf-8; replace-tabs on;
