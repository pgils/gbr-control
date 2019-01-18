#include "gbrxml.h"
#include <iostream>
#include <tinyxml2.h>
#include <cstring>
#include <stdexcept>

using namespace tinyxml2;

gbrXML::gbrXML(XMLOperation op, gbrXMLMessageType *type, std::string *xml,
       NodeConfig *conf, std::vector<NodeConfig> *configs, Signal *signal)
{
    this->mType		= (type		!= nullptr) ? *type : gbrXMLMessageType::UNDEFINED;
    if( xml 	!= nullptr ) { this->mXml		= *xml; 	}
    if( conf	!= nullptr ) { this->mConf		= *conf; 	}
    if( configs != nullptr ) { this->mConfigs	= *configs;	}
    if( signal  != nullptr ) { this->mSignal	= *signal;	}

    try {
        ( FROM_XML_STRING == op ) ? ReadXML() : BuildXML(op);
    } catch (std::runtime_error&) {
        throw;
    }
}

gbrXML::~gbrXML()
{
}

gbrXMLMessageType gbrXML::GetType() const
{
    return this->mType;
}

std::string * gbrXML::GetXML()
{
    return &this->mXml;
}

NodeConfig * gbrXML::GetNodeConfig()
{
    return &this->mConf;
}

std::vector<NodeConfig> * gbrXML::GetNodeConfigs()
{
    return &this->mConfigs;
}

Signal * gbrXML::GetSignal()
{
    return &this->mSignal;
}

template<typename T>
int gbrXML::AddXMLElement(XMLPrinter *printer, const char* element, T value)
{
    printer->OpenElement(element, XML_COMPACT);
    printer->PushText(value);
    printer->CloseElement(XML_COMPACT);

    return 0;
}

int gbrXML::NodeConfigToXML(tinyxml2::XMLPrinter *printer, NodeConfig *conf)
{
    printer->OpenElement("node", XML_COMPACT);
    AddXMLElement(printer, "eui64", conf->eui64);
    AddXMLElement(printer, "ipaddress", conf->ipaddress.c_str());
    AddXMLElement(printer, "status", conf->status);
    AddXMLElement(printer, "role", conf->role);
    if( 0 < conf->groups.size() )
    {
        printer->OpenElement("groups", XML_COMPACT);
        for( int group : conf->groups )
        {
            AddXMLElement(printer, "group", group);
        }
        printer->CloseElement(XML_COMPACT);
    }
    AddXMLElement(printer, "signal", conf->signal);
    printer->CloseElement(XML_COMPACT);

    return 0;
}

int gbrXML::BuildXML(XMLOperation op)
{
    XMLPrinter	printer;
    switch(op)
    {
    case FROM_MESSAGE_TYPE:
        AddXMLElement(&printer, "messagetype", TypeMap.at(this->mType));
        this->mXml = printer.CStr();
        break;
    case FROM_NODECONFIG:
        AddXMLElement(&printer, "messagetype", TypeMap.at(gbrXMLMessageType::NODECONFIG));
        NodeConfigToXML(&printer, &this->mConf);
        this->mXml = printer.CStr();
        break;
    case FROM_NODEVECTOR:
        AddXMLElement(&printer, "messagetype", TypeMap.at(gbrXMLMessageType::NODECONFIGS));
        for( NodeConfig config : this->mConfigs )
        {
            NodeConfigToXML(&printer, &config);
        }
        this->mXml = printer.CStr();
        break;
    case FROM_SIGNAL:
        AddXMLElement(&printer, "messagetype", TypeMap.at(gbrXMLMessageType::SIGNAL));
        AddXMLElement(&printer, "signal", this->mSignal.signal);
        if( 0 < this->mSignal.groups.size() )
        {
            printer.OpenElement("groups", XML_COMPACT);
            for( int group : this->mSignal.groups )
            {
                AddXMLElement(&printer, "group", group);
            }
            printer.CloseElement(XML_COMPACT);
        }
        this->mXml = printer.CStr();
        break;
    default:
        break;
    }

    return 0;
}

template<typename T>
XMLError gbrXML::GetXMLElementValue(XMLHandle *handle, const char* name,
                                    XMLError(XMLElement::*query)(T*) const, T *target)
{
    XMLElement *element	= handle->FirstChildElement(name).ToElement();
    if( element )
    {
        return (element->*query)( target );
    }
    std::ostringstream oss;
    oss << "Failed to get XML Element: " << name << std::endl;
    throw std::runtime_error(oss.str());

    return XMLError::XML_NO_ATTRIBUTE;
}

int gbrXML::GetXMLGroupElement(XMLHandle *handle, std::vector<int> *target)
{
    int 		group;

    // Get the first group number
    XMLElement *groupElement = handle->FirstChildElement("groups").FirstChildElement("group").ToElement();
    if( groupElement )
    {
        groupElement->QueryIntText( &group );
        target->push_back( group );

        // Get additional group numbers if present
        while( (groupElement = groupElement->NextSiblingElement()) )
        {
            groupElement->QueryIntText( &group );
            target->push_back( group );
        }
    }
    else {
        return 1;
    }

    return 0;
}

int gbrXML::GetXMLNodeElement(XMLHandle *handle, NodeConfig *config)
{
    try {
        GetXMLElementValue(handle, "eui64", 	&XMLElement::QueryInt64Text, 	&config->eui64);
        GetXMLElementValue(handle, "status", 	&XMLElement::QueryIntText, 		&config->status);
        GetXMLElementValue(handle, "role", 		&XMLElement::QueryIntText, 		&config->role);
        GetXMLElementValue(handle, "signal", 	&XMLElement::QueryIntText, 		&config->signal);
    } catch (std::runtime_error&) {
        throw;
    }

    // Get the ip-adress directly, as it is stored in string format.
    XMLElement *ipElement		= handle->FirstChildElement("ipaddress").ToElement();
    if( ipElement )
    {
        config->ipaddress	= ipElement->GetText();
    }

    GetXMLGroupElement(handle, &config->groups);

    return 0;

}

int gbrXML::ReadXML()
{
    XMLDocument 	doc;
    XMLHandle		docHandle( &doc );
    const char		*rawMessageType;
    NodeConfig conf;

    std::map<gbrXMLMessageType, const char*>::const_iterator it;

    if( XML_SUCCESS != doc.Parse(this->mXml.c_str()))
    {
        return 1;
    }

    XMLHandle		messageTypeHandle = docHandle.FirstChildElement("messagetype");
    if( !messageTypeHandle.ToElement() )
    {
        return 1;
    }
    rawMessageType = messageTypeHandle.ToElement()->GetText();

    // lookup MessageType
    for( it = TypeMap.begin(); it != TypeMap.end(); ++it )
    {
        if( 0 == strcmp(it->second, rawMessageType) )
        {
            this->mType = it->first;
        }
    }

    switch(this->mType)
    {
    case gbrXMLMessageType::SENDSIGNAL:
    case gbrXMLMessageType::SIGNAL:
        try {
        GetXMLElementValue(&docHandle, "signal", &XMLElement::QueryIntText, &this->mSignal.signal);
    } catch (std::runtime_error&) {
        throw;
    }
        GetXMLGroupElement(&docHandle, &this->mSignal.groups);
        break;
    case gbrXMLMessageType::NODECONFIG:
    {
        XMLHandle nodeHandle	= docHandle.FirstChildElement("node");
        if( !nodeHandle.ToElement() )
        {
            return 1;
        }
        try {
            GetXMLNodeElement(&nodeHandle, &this->mConf);
        } catch (std::runtime_error&) {
            throw;
        }

        break;
    }
    case gbrXMLMessageType::SETCONFIGS:
    {
        // Get the first node
        XMLHandle nodeHandle	= docHandle.FirstChildElement("node");
        if( !nodeHandle.ToElement() )
        {
            return 1;
        }
        try {
            GetXMLNodeElement(&nodeHandle, &conf);
        } catch (std::runtime_error&) {
            throw;
        }
        this->mConfigs.push_back(conf);

        // Get additional nodes if present
        while( (nodeHandle.NextSiblingElement().ToElement()) )
        {
            nodeHandle			= nodeHandle.NextSiblingElement();
            try {
                GetXMLNodeElement(&nodeHandle, &conf);
            } catch (std::runtime_error&) {
                throw;
            }
            this->mConfigs.push_back(conf);
        }
        break;
    }
    default:
        break;
    }

    return 0;
}
