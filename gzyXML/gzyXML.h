//====================================================================
//  gzyXML.h
//  created 3.18.19
//  written by gzy
//
//  https://github.com/guo122/gzyBase
//====================================================================

#ifndef GZYXML_H_C1A9C397CF09B78418AD30F93912AA72
#define GZYXML_H_C1A9C397CF09B78418AD30F93912AA72

#include <string>

#define     GZY_NAMESPACE_BEGIN     namespace gzy {
#define     GZY_NAMESPACE_END       }

GZY_NAMESPACE_BEGIN

#define     RESULT_UNKNOWN          -1
#define     RESULT_OK               0
#define     RESULT_BAD_DOC          1
#define     RESULT_LOAD_FAILURE     2
#define     RESULT_FILE_FAILURE     3
#define     RESULT_TAG_NO_NAME      4
#define     RESULT_TAG_ILLEGAL      5

class XMLNode;
typedef std::shared_ptr<XMLNode> XMLNodePtr;

class XMLNode
{
public:
    XMLNode();
    ~XMLNode();

public:
    XMLNodePtr child(const std::string &name_);
    XMLNodePtr next();

public:
    bool SetName(const std::string &name_);
    std::string name();
    void SetValue(const std::string &value_);
    void AppendValue(const std::string &value_);
    std::string value();
    void AddAttribute(const std::string &name_, const std::string &value_);

public: // todo 待保护
    void AddChild(const XMLNodePtr &child_);
    void AddChild_map(const XMLNodePtr &child_);
    void SetParent(const XMLNodePtr &parent_);
    XMLNodePtr parent();

public:
    bool IsOver();
    bool Over();

private:
    struct Impl;
    Impl *_Impl;
};

class XMLDocument
{
public:
    XMLDocument();
    ~XMLDocument();

public:
    int load_file(const std::string &file_path_);
    int load_string(const std::string &xml_str_);
    bool to_string(std::string &str_);

public:
    XMLNodePtr fisrt_child();
    XMLNodePtr child(const std::string &name_);

private:
    struct Impl;
    Impl *_Impl;
};

GZY_NAMESPACE_END

#endif // GZYXML_H_C1A9C397CF09B78418AD30F93912AA72