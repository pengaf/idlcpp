#include "VariableTypeListNode.h"
#include "VariableTypeNode.h"
#include "IdentifyNode.h"
#include "ErrorList.h"


VariableTypeListNode::VariableTypeListNode(VariableTypeListNode* variableTypeList, TokenNode* delimiter, VariableTypeNode* variableType)
{
	m_nodeType = snt_variable_type_list;
	m_variableTypeList = variableTypeList;
	m_delimiter = delimiter;
	m_variableType = variableType;
}

void VariableTypeListNode::collectVariableTypeNodes(std::vector<std::pair<TokenNode*, VariableTypeNode*>>& variableTypeNodes)
{
	VariableTypeListNode* list = this;
	while (0 != list)
	{
		variableTypeNodes.push_back(std::make_pair(list->m_delimiter, list->m_variableType));
		list = list->m_variableTypeList;
	}
	std::reverse(variableTypeNodes.begin(), variableTypeNodes.end());
}

void VariableTypeListNode::collectVariableTypeNodes(std::vector<VariableTypeNode*>& variableTypeNodes)
{
	VariableTypeListNode* list = this;
	while (0 != list)
	{
		variableTypeNodes.push_back(list->m_variableType);
		list = list->m_variableTypeList;
	}
	std::reverse(variableTypeNodes.begin(), variableTypeNodes.end());
}

VariableTypeNode* VariableTypeListNode::getFirstVariableType()
{
	VariableTypeNode* variableType = nullptr;
	VariableTypeListNode* list = this;
	while (0 != list)
	{
		variableType = list->m_variableType;
		list = list->m_variableTypeList;
	}
	return variableType;
}
