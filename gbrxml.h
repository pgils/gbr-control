#ifndef GBRXML_H
#define GBRXML_H

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <tinyxml2.h>


#ifndef NDEBUG
#define XML_COMPACT 0
#else
#define XML_COMPACT 1
#endif // NDEBUG

enum class gbrXMLMessageType
{
    UNDEFINED,
    GETCONFIGS,
    NODECONFIGS,
    SETCONFIGS,
    SENDSIGNAL,
    CONFIG,
    SIGNAL,
    GETNODECONFIG,
    NODECONFIG,
};

struct NodeConfig
{
    std::string			eui64;
    int					active;
    int					status;
    std::vector<int>	groups;
    int					role;
    int					signal;

    //TODO: should we comapre `groups` as well?
    bool operator==(const NodeConfig& c) const
    {
        return (
            c.eui64		== eui64 &&
            c.status	== status &&
            c.groups	== groups &&
            c.role		== role &&
            c.signal	== signal);
    }
};

struct Signal
{
    int					signal;
    std::vector<int>	groups;
};

class gbrXML
{
public:
    gbrXML(gbrXMLMessageType *type)				: gbrXML(FROM_MESSAGE_TYPE, type, nullptr,
                                                         nullptr, nullptr, nullptr) 		{}
    gbrXML(std::string *xml)					: gbrXML(FROM_XML_STRING, nullptr, xml,
                                                         nullptr, nullptr, nullptr) 		{}
    gbrXML(NodeConfig *conf)					: gbrXML(FROM_NODECONFIG, nullptr, nullptr,
                                                         conf, nullptr, nullptr)			{}
    gbrXML(std::vector<NodeConfig> *configs)	: gbrXML(FROM_NODEVECTOR, nullptr, nullptr,
                                                         nullptr, configs, nullptr)			{}
    gbrXML(Signal *signal)						: gbrXML(FROM_SIGNAL, nullptr, nullptr,
                                                         nullptr, nullptr, signal)			{}
    virtual ~gbrXML();

private:
    const std::map<gbrXMLMessageType, const char*> TypeMap =
    {
        { gbrXMLMessageType::UNDEFINED		, "undefined"		},
        { gbrXMLMessageType::GETCONFIGS		, "getconfigs"		},
        { gbrXMLMessageType::NODECONFIGS	, "nodeconfigs"		},
        { gbrXMLMessageType::SETCONFIGS		, "setconfigs"		},
        { gbrXMLMessageType::SENDSIGNAL		, "sendsignal"		},
//        { gbrXMLMessageType::CONFIG			, "config"			},
        { gbrXMLMessageType::SIGNAL			, "signal"			},
        { gbrXMLMessageType::GETNODECONFIG	, "getnodeconfig"	},
        { gbrXMLMessageType::NODECONFIG		, "nodeconfig"		}
    };
    enum XMLOperation {
      FROM_MESSAGE_TYPE,
      FROM_XML_STRING,
      FROM_NODECONFIG,
      FROM_NODEVECTOR,
      FROM_SIGNAL,
    };
    gbrXML(XMLOperation op, gbrXMLMessageType *type, std::string *xml,
           NodeConfig *conf, std::vector<NodeConfig> *configs, Signal *signal);
    gbrXML( const gbrXML& );
    gbrXML& operator=( const gbrXML& );

public:
    gbrXMLMessageType GetType() const;
    std::string * GetXML();
    NodeConfig * GetNodeConfig();
    std::vector<NodeConfig> * GetNodeConfigs();
    Signal * GetSignal();

private:
    gbrXMLMessageType 		mType;
    std::string 			mXml;
    NodeConfig				mConf;
    std::vector<NodeConfig>	mConfigs;
    Signal					mSignal;

    int ReadXML();
    template<typename T>
    tinyxml2::XMLError GetXMLElementValue(tinyxml2::XMLHandle *handle, const char* name,
                                          tinyxml2::XMLError(tinyxml2::XMLElement::*query)(T*) const, T *target);
    int GetXMLGroupElement(tinyxml2::XMLHandle *handle, std::vector<int> *target);
    int GetXMLNodeElement(tinyxml2::XMLHandle *handle, NodeConfig *config);
    int BuildXML(XMLOperation op);

    template<typename T>
    int AddXMLElement(tinyxml2::XMLPrinter *printer, const char* element, T value);
    int NodeConfigToXML(tinyxml2::XMLPrinter *printer, NodeConfig *conf);
};

#endif // GBRXML_H
