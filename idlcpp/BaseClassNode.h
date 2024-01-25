#pragma once
#include "SyntaxNodeImpl.h"
#include "TypeNameNode.h"
#include "TokenNode.h"


struct TokenNode;

struct BaseClassNode : SyntaxNodeImpl
{
	TypeNameNode* m_typeName;
	TokenNode* m_filterNode;
public:
	BaseClassNode(TypeNameNode* typeName, TokenNode* filter);
	bool isNoCode();
	bool isNoMeta();
};

