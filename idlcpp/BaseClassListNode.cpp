#include "BaseClassListNode.h"
#include "BaseClassNode.h"


BaseClassListNode::BaseClassListNode(BaseClassListNode* baseClassList, TokenNode* delimiter, BaseClassNode* baseClass)
{
	m_nodeType = snt_base_class_list;
	m_baseClassList = baseClassList;
	m_delimiter = delimiter;
	m_baseClass = baseClass;
}

void BaseClassListNode::collectBaseClassNodes(std::vector<BaseClassNode*>& baseClassNodes)
{
	BaseClassListNode* list = this;
	while (0 != list)
	{
		baseClassNodes.push_back(list->m_baseClass);
		list = list->m_baseClassList;
	}
	std::reverse(baseClassNodes.begin(), baseClassNodes.end());
}

void BaseClassListNode::collectBaseClassNodesNotNoCode(std::vector<std::pair<TokenNode*, BaseClassNode*>>& baseClassNodes)
{
	BaseClassListNode* list = this;
	while (0 != list)
	{
		if (!list->m_baseClass->isNoCode())
		{
			baseClassNodes.push_back(std::make_pair(list->m_delimiter, list->m_baseClass));
		}
		list = list->m_baseClassList;
	}
	std::reverse(baseClassNodes.begin(), baseClassNodes.end());
}

void BaseClassListNode::collectBaseClassNodesNotNoMeta(std::vector<std::pair<TokenNode*, BaseClassNode*>>& baseClassNodes)
{
	BaseClassListNode* list = this;
	while (0 != list)
	{
		if (!list->m_baseClass->isNoMeta())
		{
			baseClassNodes.push_back(std::make_pair(list->m_delimiter, list->m_baseClass));
		}
		list = list->m_baseClassList;
	}
	std::reverse(baseClassNodes.begin(), baseClassNodes.end());
}
