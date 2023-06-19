#pragma once

#include "SyntaxNodeImpl.h"
#include <vector>

struct TokenNode;
struct VariableTypeNode;

struct VariableTypeListNode : SyntaxNodeImpl
{
	VariableTypeListNode* m_variableTypeList;
	TokenNode* m_delimiter;
	VariableTypeNode* m_variableType;
public:
	VariableTypeListNode(VariableTypeListNode* variableTypeList, TokenNode* delimiter, VariableTypeNode* variableType);
	void collectVariableTypeNodes(std::vector<std::pair<TokenNode*, VariableTypeNode*>>& variableTypeNodes);
	void collectVariableTypeNodes(std::vector<VariableTypeNode*>& variableTypeNodes);
	VariableTypeNode* getFirstVariableType();
};

