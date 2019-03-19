//====================================================================
//  gzyXML.cpp
//  created 3.18.19
//  written by gzy
//
//  https://github.com/guo122/gzyBase
//====================================================================

#include <map>
#include <vector>
#include <fstream>

#include "gzyXML.h"

GZY_NAMESPACE_BEGIN

enum meta_type
{
    end = 0,            // 初始状态
    just_begin,         // 刚刚读取到 < 标志

    // 相对顺序固定
    declaration_over,
    declaration,
    declaration_end,
    decl_attri_name,
    decl_attri_name_end,
    decl_attri_value_apos,
    decl_attri_value_quot,

    // 相对顺序固定
    tag,
    tag_end,
    tag_attri_name,
    tag_attri_name_end,
    tag_attri_value_apos,
    tag_attri_value_quot,

    element,
    element_end,

    end_tag,

    comment,
    comment_1,
    comment_2,
    comment_3,
    comment_4,
    comment_end
};

struct attribute_struct
{
    attribute_struct(const std::string &name_, const std::string &value_)
        : _name(name_)
        , _value(value_)
    {}

    std::string	_name;
    std::string	_value;
};

struct XMLNode::Impl
{
    Impl()
        : _name("")
        , _value("")
        , _next(nullptr)
        , _parent(nullptr)
        , _overFlag(false)
    {}

    std::string _name;
    std::string _value;
    std::vector<attribute_struct> _attriList;

    // 用于同名tag
    XMLNodePtr _next;

    XMLNodePtr _parent;
    std::vector<XMLNodePtr> _children;
    std::map<std::string, std::vector<XMLNodePtr>> _childrenMap;

    bool _overFlag;
};

XMLNode::XMLNode()
    : _Impl(new Impl)
{

}
XMLNode::~XMLNode()
{
    delete _Impl;
    _Impl = nullptr;
}

XMLNodePtr XMLNode::child(const std::string &name_)
{
    XMLNodePtr Result = nullptr;
    std::string lastStr = name_;
    std::string tmpStr = "";
    std::string curStr = "";
    int index = 0;
    // 去掉前面多余的正斜线
    if (lastStr.size() > 0 && lastStr[0] == '/')
    {
        lastStr.erase(0, 1);
    }
    // 增加尾部必须的正斜线
    if (lastStr.size() > 0 && lastStr[lastStr.size() - 1] != '/')
    {
        lastStr.push_back('/');
    }
    tmpStr = lastStr.substr(0, lastStr.find_first_of('/'));
    curStr = tmpStr.substr(0, tmpStr.find_first_of('['));
    lastStr.erase(0, lastStr.find_first_of('/') + 1);
    // 判断是否有下标
    tmpStr.erase(0, tmpStr.find_first_of('['));
    if (tmpStr.size() > 2)
    {
        tmpStr.erase(0, 1);
        tmpStr.erase(tmpStr.size() - 1, 1);
        index = std::atoi(tmpStr.c_str());
    }

    if (!_Impl->_childrenMap[curStr].empty())
    {
        if (index < 0 || index >= _Impl->_childrenMap[curStr].size())
        {
            index = 0;
        }
        if (lastStr.empty())
        {
            // 当前目标
            Result = _Impl->_childrenMap[curStr][index];

            if (_Impl->_childrenMap[curStr].size() > 1 && !_Impl->_childrenMap[curStr][0]->_Impl->_next)
            {
                // 存在多个同名项
                for (int i = 1; i < _Impl->_childrenMap[curStr].size(); ++i)
                {
                    // 建立 next快速访问
                    _Impl->_childrenMap[curStr][i - 1]->_Impl->_next = _Impl->_childrenMap[curStr][i];
                }
            }
        }
        else
        {
            // 递归
            Result = _Impl->_childrenMap[curStr][index]->child(lastStr);
        }
    }

    return Result;
}

XMLNodePtr XMLNode::next()
{
    return _Impl->_next;
}

bool XMLNode::SetName(const std::string &name_)
{
    bool Result = false;
    typeof (std::find(name_.begin(), name_.end(), '/')) it;
    if ( (it = std::find(name_.begin(), name_.end(), '/')) == name_.end() &&
         (it = std::find(name_.begin(), name_.end(), '[')) == name_.end() &&
         (it = std::find(name_.begin(), name_.end(), ']')) == name_.end())
    {
        _Impl->_name = name_;
        Result = true;
    }

    return Result;
}

std::string XMLNode::name()
{
    return _Impl->_name;
}

void XMLNode::SetValue(const std::string &value_)
{
    _Impl->_value = value_;
}

void XMLNode::AppendValue(const std::string &value_)
{
    _Impl->_value += value_;
}

std::string XMLNode::value()
{
    return _Impl->_value;
}

void XMLNode::AddAttribute(const std::string &name_, const std::string &value_)
{
    _Impl->_attriList.push_back(attribute_struct(name_, value_));
}

void XMLNode::AddChild(const XMLNodePtr &child_)
{
    _Impl->_children.push_back(child_);
}

void XMLNode::AddChild_map(const XMLNodePtr &child_)
{
    if (child_ && !child_->name().empty())
    {
        _Impl->_childrenMap[child_->name()].push_back(child_);
    }
}

void XMLNode::SetParent(const XMLNodePtr &parent_)
{
    _Impl->_parent = parent_;
}

XMLNodePtr XMLNode::parent()
{
    return _Impl->_parent;
}

bool XMLNode::IsOver()
{
    return _Impl->_overFlag;
}
bool XMLNode::Over()
{
    bool Result = !_Impl->_overFlag;
    _Impl->_overFlag = true;
    return Result;
}

struct XMLDocument::Impl
{
    Impl()
        : _root(nullptr)
    {}

    // pos2_ 为超尾 下标
    bool XMLSlice(const std::string &xml_, int &pos1_, const int &pos2_, std::string &str_)
    {
        bool Result = false;
        if (pos1_ >= 0 && pos1_ < xml_.size() &&
                pos2_ >= 0 && pos2_ < xml_.size() &&
                pos1_ <= pos2_)
        {
            str_ = xml_.substr(pos1_, pos2_ - pos1_);
            pos1_ = -1;
            Result = true;
        }
        return Result;
    }

    XMLNodePtr _root;
    XMLNodePtr _decl;
};

XMLDocument::XMLDocument()
    : _Impl(new Impl)
{
}

XMLDocument::~XMLDocument()
{
    delete _Impl;
    _Impl = nullptr;
}

int XMLDocument::load_file(const std::string &file_path_)
{
    int Result = RESULT_UNKNOWN;

    std::string buf = "";
    std::string tmpStr = "";

    std::ifstream fp(file_path_, std::ios::in);
    if (fp.is_open())
    {
        while (getline(fp, buf))
        {
            tmpStr += buf + "\n";
        }
        Result = load_string(tmpStr);
    }
    else
    {
        Result = RESULT_FILE_FAILURE;
    }

    return Result;
}

int XMLDocument::load_string(const std::string &xml_str_)
{
    int Result = RESULT_UNKNOWN;

    clear();

    meta_type curType = meta_type::end;
    int markPos = -1;
    std::string lastAttriName = "";
    std::string tmpStr = "";
    XMLNodePtr cur = nullptr;
    XMLNodePtr tmpNodePtr = nullptr;

    _Impl->_decl = std::make_shared<XMLNode>();

    char tmpC;

    for (size_t i = 0; i < xml_str_.size(); ++i)
    {
        tmpC = xml_str_[i];
        if (curType == meta_type::end)
        {
            if (xml_str_[i] == '<')
            {
                curType = meta_type::just_begin;
            }
            else
            {
                curType = meta_type::element;
                markPos = i;
            }
        }
        else if (curType == meta_type::element)
        {
            if (xml_str_[i] == '<' && cur)
            {
                curType = meta_type::just_begin;
                if (!_Impl->XMLSlice(xml_str_, markPos, i, tmpStr))
                {
                    Result = RESULT_UNKNOWN;
                    break;
                }
                cur->AppendValue(tmpStr);
            }
        }
        else if (curType == meta_type::just_begin)
        {
            if (xml_str_[i] == '?')
            {
                curType = meta_type::declaration;
                cur = _Impl->_decl;
                markPos = i + 1;
            }
            else if (xml_str_[i] == '!')
            {
                curType = meta_type::comment;
            }
            else if (xml_str_[i] == '/')
            {
                curType = meta_type::end_tag;
                markPos = i + 1;
            }
            else if ((xml_str_[i] >= 'a' && xml_str_[i] <= 'z') ||
                     (xml_str_[i] >= 'A' && xml_str_[i] <= 'Z') ||
                     xml_str_[i] == '_'
                     )
            {
                curType = meta_type::tag;
                markPos = i;

                tmpNodePtr = std::make_shared<XMLNode>();

                if (!_Impl->_root)
                {
                    cur = _Impl->_root = tmpNodePtr;
                }
                else
                {
                    if (cur)
                    {
                        cur->AddChild(tmpNodePtr);
                        tmpNodePtr->SetParent(cur);
                        cur = tmpNodePtr;
                    }
                    else
                    {
                        Result = RESULT_UNKNOWN;
                        break;
                    }
                }
            }
            else
            {
                Result = RESULT_BAD_DOC;
                break;
            }
        }
        else if (curType == meta_type::end_tag)
        {
            // 读取结束tag
            if (xml_str_[i] == '>' || xml_str_[i] == ' ')
            {
                if (!_Impl->XMLSlice(xml_str_, markPos, i, tmpStr))
                {
                    Result = RESULT_BAD_DOC;
                    break;
                }
                else
                {
                    // 匹配名字，是否配对
                    if (cur && cur->name() == tmpStr && cur->Over())
                    {
                        if (cur->parent())
                        {
                            // 将子节点添加到父节点map中
                            cur->parent()->AddChild_map(cur);
                        }

                        cur = cur->parent();
                    }
                    else
                    {
                        Result = RESULT_BAD_DOC;
                        break;
                    }
                }
            }
            if (xml_str_[i] == '>')
            {
                curType = meta_type::end;
                if (!cur)
                {
                    Result = RESULT_OK;
                }
            }
        }
        else if (curType == meta_type::comment)
        {
            if (xml_str_[i] == '-')
            {
                curType = meta_type::comment_1;
            }
            else
            {
                Result = RESULT_BAD_DOC;
                break;
            }
        }
        else if (curType == meta_type::comment_1)
        {
            if (xml_str_[i] == '-')
            {
                curType = meta_type::comment_2;
            }
            else
            {
                Result = RESULT_BAD_DOC;
                break;
            }
        }
        else if (curType == meta_type::comment_2)
        {
            if (xml_str_[i] == '-')
            {
                curType = meta_type::comment_3;
            }
        }
        else if (curType == meta_type::comment_3)
        {
            if (xml_str_[i] == '-')
            {
                curType = meta_type::comment_4;
            }
            else
            {
                Result = RESULT_BAD_DOC;
                break;
            }
        }
        else if (curType == meta_type::comment_4)
        {
            if (xml_str_[i] == '>')
            {
                curType = meta_type::end;
            }
            else
            {
                Result = RESULT_BAD_DOC;
                break;
            }
        }
        else if (curType == meta_type::declaration)
        {
            if (xml_str_[i] == ' ')
            {
                curType = meta_type::declaration_end;

                if (!_Impl->XMLSlice(xml_str_, markPos, i, tmpStr))
                {
                    Result = RESULT_UNKNOWN;
                    break;
                }
                if (tmpStr.empty())
                {
                    Result = RESULT_TAG_NO_NAME;
                    break;
                }
                if (!cur->SetName(tmpStr))
                {
                    Result = RESULT_TAG_ILLEGAL;
                    break;
                }
            }
        }
        else if (curType == meta_type::declaration_end)
        {
            if (xml_str_[i] == '?')
            {
                curType = meta_type::declaration_over;
            }
            else if ((xml_str_[i] >= 'a' && xml_str_[i] <= 'z') ||
                    (xml_str_[i] >= 'A' && xml_str_[i] <= 'Z'))
            {
                curType = meta_type::decl_attri_name;
                markPos = i;
            }
            else if (xml_str_[i] != ' ')
            {
                Result = RESULT_BAD_DOC;
                break;
            }
        }
        else if (curType == meta_type::declaration_over)
        {
            if (xml_str_[i] == '>')
            {
                curType = meta_type::end;
            }
            else
            {
                Result = RESULT_BAD_DOC;
                break;
            }
        }
        else if (curType == meta_type::tag)
        {
            if (xml_str_[i] == '>')
            {
                // 读取完毕无属性的tag
                curType = meta_type::end;
                if (!_Impl->XMLSlice(xml_str_, markPos, i, tmpStr))
                {
                    Result = RESULT_UNKNOWN;
                    break;
                }
                if ((i - 1) >= 0 && xml_str_[i - 1] == '/')
                {
                    // 此无属性tag提前结束
                    if (!cur || !cur->Over())
                    {
                        Result = RESULT_BAD_DOC;
                        break;
                    }
                    tmpStr.erase(tmpStr.size() - 1, 1);
                }
                if (tmpStr.empty())
                {
                    Result = RESULT_TAG_NO_NAME;
                    break;
                }
                if (!cur->SetName(tmpStr))
                {
                    Result = RESULT_TAG_ILLEGAL;
                    break;
                }
                if (cur->IsOver())
                {
                    if (cur->parent())
                    {
                        // 将子节点添加到父节点map中
                        cur->parent()->AddChild_map(cur);
                    }
                    cur = cur->parent();
                    if (!cur)
                    {
                        Result = RESULT_OK;
                    }
                }
            }
            else if (xml_str_[i] == ' ')
            {
                curType = meta_type::tag_end;
                if (!_Impl->XMLSlice(xml_str_, markPos, i, tmpStr))
                {
                    Result = RESULT_UNKNOWN;
                    break;
                }
                if (tmpStr.empty())
                {
                    Result = RESULT_TAG_NO_NAME;
                    break;
                }
                if (!cur->SetName(tmpStr))
                {
                    Result = RESULT_TAG_ILLEGAL;
                    break;
                }
            }
        }
        else if (curType == meta_type::tag_end)
        {
            // tag名字已读取，准备读属性或结束标志 >
            if (xml_str_[i] == '>')
            {
                // 结束tag
                curType = meta_type::end;
                if ((i - 1) >= 0 && xml_str_[i - 1] == '/')
                {
                    // 单标签，同时开始结束
                    if (cur && cur->Over())
                    {
                        if (cur->parent())
                        {
                            // 将子节点添加到父节点map中
                            cur->parent()->AddChild_map(cur);
                        }
                        cur = cur->parent();
                    }
                    else
                    {
                        Result = RESULT_BAD_DOC;
                        break;
                    }
                }
            }
            else if ((xml_str_[i] >= 'a' && xml_str_[i] <= 'z') ||
                    (xml_str_[i] >= 'A' && xml_str_[i] <= 'Z') ||
                     xml_str_[i] == '_')
            {
                curType = meta_type::tag_attri_name;
                markPos = i;
            }
            else if (xml_str_[i] != ' ')
            {
                Result = RESULT_BAD_DOC;
                break;
            }
        }
        else if (curType == meta_type::decl_attri_name ||
                 curType == meta_type::tag_attri_name)
        {
            if (xml_str_[i] == ' ' || xml_str_[i] == '=')
            {
                // 依赖meta_type中元素顺序 状态变成对应的 _end
                curType = static_cast<meta_type>(static_cast<int>(curType) + 1);

                // in
                if (!_Impl->XMLSlice(xml_str_, markPos, i, lastAttriName))
                {
                    Result = RESULT_UNKNOWN;
                    break;
                }
            }
        }
        else if (curType == meta_type::decl_attri_name_end ||
                 curType == meta_type::tag_attri_name_end)
        {
            if (xml_str_[i] == '\'')
            {
                // 依赖meta_type中元素顺序 状态变成对应的 _apos
                curType = static_cast<meta_type>(static_cast<int>(curType) + 1);
                markPos = i + 1;
            }
            else if (xml_str_[i] == '"')
            {
                // 依赖meta_type中元素顺序 状态变成对应的 _quot
                curType = static_cast<meta_type>(static_cast<int>(curType) + 2);
                markPos = i + 1;
            }
        }
        else if (curType == meta_type::decl_attri_value_apos ||
                 curType == meta_type::tag_attri_value_apos)
        {
            if (xml_str_[i] == '\'')
            {
                // 依赖meta_type中元素顺序 返回 decl_end 或 tag_end 状态
                curType = static_cast<meta_type>(static_cast<int>(curType) - 3);

                // in
                if (!_Impl->XMLSlice(xml_str_, markPos, i, tmpStr))
                {
                    Result = RESULT_UNKNOWN;
                    break;
                }
                cur->AddAttribute(lastAttriName, tmpStr);
            }
        }
        else if (curType == meta_type::decl_attri_value_quot ||
                 curType == meta_type::tag_attri_value_quot)
        {
            if (xml_str_[i] == '"')
            {
                // 依赖meta_type中元素顺序 返回 decl_end 或 tag_end 状态
                curType = static_cast<meta_type>(static_cast<int>(curType) - 4);

                // in
                if (!_Impl->XMLSlice(xml_str_, markPos, i, tmpStr))
                {
                    Result = RESULT_UNKNOWN;
                    break;
                }
                cur->AddAttribute(lastAttriName, tmpStr);
            }
        }
    }

    return Result;
}

bool XMLDocument::to_string(std::string &str_)
{
    bool Result = false;
    return Result;
}

void XMLDocument::clear()
{
    _Impl->_root = nullptr;
    _Impl->_decl = nullptr;
}

XMLNodePtr XMLDocument::fisrt_child()
{
    XMLNodePtr Result = _Impl->_root;
    return Result;
}

XMLNodePtr XMLDocument::child(const std::string &name_)
{
    XMLNodePtr Result = nullptr;
    std::string lastStr = name_;
    std::string curStr;
    // 去掉前面多余的正斜线
    if (lastStr.size() > 0 && lastStr[0] == '/')
    {
        lastStr.erase(0, 1);
    }
    // 增加尾部必须的正斜线
    if (lastStr.size() > 0 && lastStr[lastStr.size() - 1] != '/')
    {
        lastStr.push_back('/');
    }
    curStr = lastStr.substr(0, lastStr.find_first_of('/'));
    lastStr.erase(0, lastStr.find_first_of('/') + 1);

    if (_Impl->_root && curStr == _Impl->_root->name())
    {
        if (lastStr.empty())
        {
            Result = _Impl->_root;
        }
        else
        {
            Result = _Impl->_root->child(lastStr);
        }
    }

    return Result;
}

GZY_NAMESPACE_END
