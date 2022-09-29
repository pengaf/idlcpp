#pragma once

#include "MemberNode.h"

struct TokenNode;
struct TypeNameNode;

struct FieldNode : MemberNode
{
	TokenNode* m_static;
	TypeNameNode* m_typeName;
	TokenNode* m_leftBracket;
	TokenNode* m_rightBracket;
	TokenNode* m_semicolon;
	TypeCompound m_typeCompound;
public:
	FieldNode(TypeNameNode* typeName, TypeCompound typeCompound, IdentifyNode* name);
	bool isArray();
	bool isStatic();
	virtual void checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void checkSemantic(TemplateArguments* templateArguments);
};
